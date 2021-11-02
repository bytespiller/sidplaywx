/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021 Jasmin Rutic (bytespiller@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/gpl-3.0.html
 */

#include "FramePlayer.h"
#include "ElementsPlayer.h"
#include "../MyApp.h"
#include "../Config/AppSettings.h"
#include "../Config/UIStrings.h"
#include "../Helpers/HelpersWx.h"
#include "../UIElements/Playlist/Playlist.h"
#include "../../Util/Const.h"

using PlaylistItemStyle = UIElements::Playlist::Playlist::ItemStyle;
using SongItemType = UIElements::Playlist::SongTreeItemData::SongItemType;
using ItemStatus = UIElements::Playlist::SongTreeItemData::ItemStatus;

static const int iconNoIcon = static_cast<int>(FrameElements::PlaylistIconId::NoIcon);
static const int iconDefaultSong = static_cast<int>(FrameElements::PlaylistIconId::DefaultSubsongIndicator);
static const int iconChipIcon = static_cast<int>(FrameElements::PlaylistIconId::ChipIcon);
static const int iconSkipShortIcon = static_cast<int>(FrameElements::PlaylistIconId::SkipShort);

void FramePlayer::DiscoverFilesAndSendToPlaylist(const wxArrayString& rawPaths, bool clearPrevious, bool autoPlayFirstImmediately)
{
    if (_exitingApplication) // In case the user clicked Close while adding lots of files.
    {
        return;
    }

    wxBeginBusyCursor();
    SetStatusText(Strings::FramePlayer::STATUS_DISCOVERING_FILES, 2); // TODO

    const wxArrayString& validFileList = Helpers::Wx::Files::GetValidFiles(rawPaths);
    if (!validFileList.IsEmpty())
    {
        if (_addingFilesToPlaylist)
        {
            _enqueuedFiles.reserve(_enqueuedFiles.GetCount() + validFileList.GetCount());
            for (auto& item : validFileList)
            {
                _enqueuedFiles.Add(item);
            }
        }
        else
        {
            SendFilesToPlaylist(validFileList, clearPrevious, autoPlayFirstImmediately);
        }
    }

    UpdateUiState();
    wxEndBusyCursor();
    SetCursor(*wxSTANDARD_CURSOR); // Workaround the wx issue where the busy cursor doesn't properly update until the mouse is moved.
}

void FramePlayer::SendFilesToPlaylist(const wxArrayString& files, bool clearPrevious, bool autoPlayFirstImmediately)
{
    //wxWindowDisabler disabler;
    //wxBusyCursor busyCursor;

    _addingFilesToPlaylist = true;

    if (clearPrevious)
    {
        _app.StopPlayback();
        _app.UnloadActiveTune();

        SetStatusText(Strings::FramePlayer::STATUS_CLEARING_PLAYLIST, 2); // TODO (for large amount of items this can take a few sec)
        _ui->treePlaylist->ClearPlaylist();

        UpdateUiState();
        Update();
    }

    bool shouldAutoPlay = (autoPlayFirstImmediately) ? _app.currentSettings->GetOption(Settings::AppSettings::ID::AutoPlay)->GetValueAsBool() : false;
    const uint_least32_t skipDurationThreshold = static_cast<uint_least32_t>(_app.currentSettings->GetOption(Settings::AppSettings::ID::SkipShorter)->GetValueAsInt() * Const::MILLISECONDS_IN_SECOND);
    const uint_least32_t fallbackDuration = static_cast<uint_least32_t>(_app.currentSettings->GetOption(Settings::AppSettings::ID::SongFallbackDuration)->GetValueAsInt() * Const::MILLISECONDS_IN_SECOND);

    int processedFilesCount = 0;
    int lastPercentage = -1;
    bool tuneIsValid = false;

    int playableTunesCount = 0; // Not important if it rolls over.
    const PlaybackController& playback = _app.GetPlaybackInfo();

    for (const wxString& filepath : files)
    {
        ++processedFilesCount;

        const int totalFiles = files.GetCount() + _enqueuedFiles.GetCount();
        const float totalFilesFloat = static_cast<float>(totalFiles);

        // Progress percentage display ------------------------
        const int currentPercentage = static_cast<int>((processedFilesCount / totalFilesFloat) * 100.0f);
        if (currentPercentage != lastPercentage) // SetStatusText calls are expensive.
        {
            const wxString textAddingFilesWithCount = wxString::Format(Strings::FramePlayer::STATUS_ADDING_FILES_WITH_COUNT, totalFiles);
            SetStatusText(wxString::Format("%s (%i%%)", textAddingFilesWithCount, currentPercentage), 2);
        }
        lastPercentage = currentPercentage;

        // Inspect tune in a separate info-only decoder -------
        if (Helpers::Wx::Files::IsWithinZipFile(filepath))
        {
            const std::unique_ptr<BufferHolder>& infoTuneBufferHolder = Helpers::Wx::Files::GetFileContentFromZip(filepath);
            tuneIsValid = infoTuneBufferHolder != nullptr && _silentSidInfoDecoder.TryLoadSong(infoTuneBufferHolder->buffer, infoTuneBufferHolder->size);
        }
        else
        {
            //tuneIsValid =_silentSidInfoDecoder.TryLoadSong(filepath); // Would be much faster but no unicode paths support then.
            const std::unique_ptr<BufferHolder>& infoTuneBufferHolder = Helpers::Wx::Files::GetFileContentFromDisk(filepath);
            tuneIsValid = infoTuneBufferHolder != nullptr && _silentSidInfoDecoder.TryLoadSong(infoTuneBufferHolder->buffer, infoTuneBufferHolder->size, 0);
        }

        if (tuneIsValid)
        {
            bool hasNonAutoSkippedSong = false;

            // Tune title
            const int sidsNeeded = _silentSidInfoDecoder.GetCurrentTuneSidChipsRequired();
            const wxString songTitleAddendum = (sidsNeeded > 1) ? wxString::Format(" [%iSID]", sidsNeeded) : "";
            const wxString songTitle = _silentSidInfoDecoder.GetCurrentTuneInfoString(SidDecoder::SongInfoCategory::Title) + songTitleAddendum;

            // Subsongs count
            const int defaultSubsong = _silentSidInfoDecoder.GetDefaultSubsong();
            const int totalSubsongs = _silentSidInfoDecoder.GetTotalSubsongs();

            // Tune ROM requirement
            const SidDecoder::SongRequirement romRequirement = _silentSidInfoDecoder.GetCurrentSongRomRequirement();
            const bool playable = playback.IsRomLoaded(romRequirement);
            const ItemStatus playlistItemStatus = (playable) ? ItemStatus::Normal : ItemStatus::Unplayable;
            const bool needsBasicRom = romRequirement == SidDecoder::SongRequirement::BasicRom;

            // Item style
            PlaylistItemStyle style = PlaylistItemStyle::Normal;
            if (!playable)
            {
                style = (needsBasicRom) ? PlaylistItemStyle::MissingBasic : PlaylistItemStyle::MissingKernal;
            }

            wxTreeItemId mainSongNode;

            {
                // Tune duration
                const uint_least32_t realDuration = _silentSidInfoDecoder.TryGetActiveSongDuration();

                // Add main song to playlist tree
                const int mainSongIcon = (romRequirement == SidDecoder::SongRequirement::None) ? iconNoIcon : iconChipIcon;
                mainSongNode = _ui->treePlaylist->AppendMainSong(new SongTreeItemData(songTitle, filepath, defaultSubsong, realDuration, SongItemType::Song, playlistItemStatus, mainSongIcon), totalSubsongs, style);
                if (totalSubsongs == 1)
                {
                    const uint_least32_t relevantDuration = (realDuration == 0) ? fallbackDuration : realDuration;
                    const bool autoSkipped = skipDurationThreshold > 0 && (relevantDuration < skipDurationThreshold);
                    hasNonAutoSkippedSong = !autoSkipped;
                }
            }

            if (playable)
            {
                ++playableTunesCount;
            }

            // Add any subsongs to playlist tree
            if (totalSubsongs > 1)
            {
                for (int i = 1; i <= totalSubsongs; ++i)
                {
                    if (_silentSidInfoDecoder.TrySetSubsong(i))
                    {
                        const int subsongIcon = (i == defaultSubsong) ? iconDefaultSong : iconNoIcon;
                        const uint_least32_t realDuration = _silentSidInfoDecoder.TryGetActiveSongDuration();
                        const wxTreeItemId& subsongNode = _ui->treePlaylist->AppendSubSong(mainSongNode, new SongTreeItemData(songTitle, filepath, i, realDuration, SongItemType::Subsong, playlistItemStatus, subsongIcon), style);

                        const uint_least32_t relevantDuration = (realDuration == 0) ? fallbackDuration : realDuration;
                        if (skipDurationThreshold > 0 && (relevantDuration < skipDurationThreshold))
                        {
                            _ui->treePlaylist->IgnoreSong(subsongNode, iconSkipShortIcon);
                        }
                        else
                        {
                            hasNonAutoSkippedSong = true;
                        }
                    }
                }
            }

            // One tune (with any subsongs) added -----------------
            if (!hasNonAutoSkippedSong)
            {
                _ui->treePlaylist->IgnoreSong(mainSongNode, iconSkipShortIcon);
            }

            if (shouldAutoPlay)
            {
                const SongTreeItemData* subsongItemData = nullptr;

                {
                    const SongTreeItemData& mainSongItemData = _ui->treePlaylist->GetSongTreeItemData(mainSongNode);
                    subsongItemData = _ui->treePlaylist->GetEffectiveDefaultOrFirstPlayableSubsong(mainSongItemData);
                }

                shouldAutoPlay = subsongItemData->GetStatus() != SongTreeItemData::ItemStatus::Normal || !TryPlayPlaylistItem(*subsongItemData);
            }
        }

        if (playableTunesCount == 2)
        {
            UpdateUiState(); // Simply to enable the "next song" button immediately while still adding lots of files.
        }

        wxYield(); // Also must be before the HasFocus() call because not even that updates otherwise!
        if (_exitingApplication) // In case the user clicked Close while adding lots of files. This should be checked immediately after any wxYield.
        {
            return;
        }
    }

    // All tunes added ----------------------------------------
    if (!_enqueuedFiles.IsEmpty())
    {
        wxArrayString moreFiles(_enqueuedFiles);
        _enqueuedFiles.Clear();
        _enqueuedFiles.Shrink();
        SendFilesToPlaylist(moreFiles, false, false);
    }
    else
    {
        _addingFilesToPlaylist = false;
    }

    //_ui->treePlaylist->SortChildren(_ui->treePlaylist->GetRootItem()); // TODO: turn this into a menu action instead
}

void FramePlayer::UpdateIgnoredSongs(PassKey<FramePrefs>)
{
    UpdateIgnoredSongs();
}

void FramePlayer::UpdateIgnoredSongs()
{
    const uint_least32_t skipDurationThreshold = static_cast<uint_least32_t>(_app.currentSettings->GetOption(Settings::AppSettings::ID::SkipShorter)->GetValueAsInt() * Const::MILLISECONDS_IN_SECOND);
    const uint_least32_t fallbackDuration = static_cast<uint_least32_t>(_app.currentSettings->GetOption(Settings::AppSettings::ID::SongFallbackDuration)->GetValueAsInt() * Const::MILLISECONDS_IN_SECOND);

    _ui->treePlaylist->ForEachSibling([skipDurationThreshold, fallbackDuration, this](SongTreeItemData& mainSongData)
    {
        assert(mainSongData.GetType() == SongTreeItemData::SongItemType::Song);

        // Main song
        {
            const uint_least32_t realMainSongDuration = mainSongData.GetDuration();
            const uint_least32_t relevantMainSongDuration = (realMainSongDuration == 0) ? fallbackDuration : realMainSongDuration;
            if (skipDurationThreshold > 0 && (relevantMainSongDuration < skipDurationThreshold))
            {
                _ui->treePlaylist->IgnoreSong(mainSongData, iconSkipShortIcon);
            }
            else
            {
                _ui->treePlaylist->RestoreIgnoredSong(mainSongData);
            }
        }

        // Subsongs
        {
            _ui->treePlaylist->ForEachSibling([skipDurationThreshold, fallbackDuration, this](SongTreeItemData& subsongData)
            {
                const uint_least32_t realSubsongDuration = subsongData.GetDuration();
                const uint_least32_t relevantSubsongDuration = (realSubsongDuration == 0) ? fallbackDuration : realSubsongDuration;
                if (skipDurationThreshold > 0 && (relevantSubsongDuration < skipDurationThreshold))
                {
                    _ui->treePlaylist->IgnoreSong(subsongData, iconSkipShortIcon);
                }
                else
                {
                    _ui->treePlaylist->RestoreIgnoredSong(subsongData);
                }
            }, mainSongData.GetId());
        }

    }, _ui->treePlaylist->GetRootItem());

    UpdateUiState();
}
