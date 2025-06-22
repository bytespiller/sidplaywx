/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2025 Jasmin Rutic (bytespiller@gmail.com)
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
#include "../UIElements/Playlist/Components/PlaylistModel.h"
#include "../../Util/Const.h"

namespace
{
    /// @brief └
    constexpr int BOX_CHAR_L = 0x2514;

    /// @brief ├
    constexpr int BOX_CHAR_VERT_RIGHT = 0x251C;
}

std::vector<wxString> FramePlayer::GetCurrentPlaylistFilePaths(bool includeBlacklistedSongs)
{
    const PlaylistTreeModelNodePtrArray& songs = _ui->treePlaylist->GetSongs();

    std::vector<wxString> fileList;
    fileList.reserve(songs.size());

    for (const PlaylistTreeModelNodePtr& node : songs)
    {
        if (!includeBlacklistedSongs && node->GetTag() == PlaylistTreeModelNode::ItemTag::Blacklisted)
        {
            continue;
        }

        fileList.emplace_back(node->filepath);
    }

    return fileList;
}

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

        SetStatusText(Strings::FramePlayer::STATUS_CLEARING_PLAYLIST, 2); // TODO
        _ui->treePlaylist->Clear();

        UpdateUiState();
        Update();
    }

    const bool enabledShortSongSkip = _app.currentSettings->GetOption(Settings::AppSettings::ID::SkipShorter)->GetValueAsInt() > 0;
    bool shouldAutoPlay = (autoPlayFirstImmediately) ? _app.currentSettings->GetOption(Settings::AppSettings::ID::AutoPlay)->GetValueAsBool() : false;

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
            // Tune title
            wxString songTitle(_silentSidInfoDecoder.GetCurrentTuneInfoString(SidDecoder::SongInfoCategory::Title));
            {
                if (songTitle.IsEmpty()) [[unlikely]] // Fallback/MUS file (rare situation)
                {
                    songTitle = wxFileNameFromPath(filepath);
                }

                const int sidsNeeded = _silentSidInfoDecoder.GetCurrentTuneSidChipsRequired();
                const wxString& songTitleAddendum = (sidsNeeded > 1) ? wxString::Format(" [%iSID]", sidsNeeded) : wxGetEmptyString();
                songTitle.Append(songTitleAddendum);
            }

            // Subsongs count
            const int defaultSubsong = _silentSidInfoDecoder.GetDefaultSubsong();
            const int totalSubsongs = _silentSidInfoDecoder.GetTotalSubsongs();

            // Tune ROM requirement
            const SidDecoder::RomRequirement romRequirement = _silentSidInfoDecoder.GetCurrentSongRomRequirement();
            const bool playable = playback.IsRomLoaded(romRequirement);

            // Add main song node to playlist tree
            PlaylistTreeModelNode* mainSongNodeNew = nullptr;

            const Songlengths::HvscInfo& hvscInfoMain = TryGetHvscInfo(_silentSidInfoDecoder.CalcCurrentTuneMd5());

            {
                const wxString author = Helpers::Wx::StringFromWin1252(_silentSidInfoDecoder.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Author)); // Don't use reference.
                const wxString copyright = Helpers::Wx::StringFromWin1252(_silentSidInfoDecoder.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Released)); // Don't use reference.

                // Determine ROM requirement
                PlaylistTreeModelNode::RomRequirement nodeRom = PlaylistTreeModelNode::RomRequirement::None;
                switch (romRequirement)
                {
                    case SidDecoder::RomRequirement::None:
                        nodeRom = PlaylistTreeModelNode::RomRequirement::None;
                        break;
                    case SidDecoder::RomRequirement::BasicRom:
                        nodeRom = PlaylistTreeModelNode::RomRequirement::BasicRom;
                        break;
                    case SidDecoder::RomRequirement::R64:
                        nodeRom = PlaylistTreeModelNode::RomRequirement::R64;
                        break;
                    default:
                        wxMessageBox(Strings::Internal::UNHANDLED_SWITCH_CASE); // throwing doesn't work properly with release mode wxWidgets
                        throw(Strings::Internal::UNHANDLED_SWITCH_CASE);
                }

                mainSongNodeNew = &_ui->treePlaylist->AddMainSong(Helpers::Wx::StringFromWin1252(songTitle.ToStdString()), filepath, defaultSubsong, hvscInfoMain.duration, hvscInfoMain.hvscPath, hvscInfoMain.md5, author, copyright, nodeRom, playable);
            }

            if (playable)
            {
                ++playableTunesCount;
            }

            // Add any subsongs to playlist tree
            if (totalSubsongs > 1)
            {
                std::vector<uint_least32_t> subsongDurations;
                subsongDurations.reserve(totalSubsongs);

                // Determine durations
                for (int i = 1; i <= totalSubsongs; ++i)
                {
                    subsongDurations.emplace_back(TryGetHvscInfo(hvscInfoMain.md5, i).duration);
                }

                // Determine subsong titles (from STIL where possible)
                std::vector<wxString> subsongTitles;
                subsongTitles.reserve(totalSubsongs);

                {
                    Stil::Info info = _stilInfo.Get(mainSongNodeNew->hvscPath.ToStdString()); // Blank if unavailable.

                    for (int i = 1; i <= totalSubsongs; ++i)
                    {
                        const int boxChar = (i < totalSubsongs) ? BOX_CHAR_VERT_RIGHT : BOX_CHAR_L;
                        const std::string& title = info.GetFieldAsString(info.names, i);

                        if (!title.empty()) // STIL title
                        {
                            subsongTitles.emplace_back(wxString::Format("%c %s %i: %s", boxChar, Strings::PlaylistTree::SUBSONG, i, Helpers::Wx::StringFromWin1252(title)));
                        }
                        else // Generic subsong title
                        {
                            subsongTitles.emplace_back(wxString::Format("%c %s %i", boxChar, Strings::PlaylistTree::SUBSONG, i));
                        }
                    }
                }

                // Add subsongs
                _ui->treePlaylist->AddSubsongs(subsongDurations, subsongTitles, *mainSongNodeNew);
            }

            // One tune (with any subsongs) added -----------------

            if (playable && enabledShortSongSkip) // Tag short songs
            {
                UpdateIgnoredSong(*mainSongNodeNew);
            }
            else // Apply Normal tag and ROM requirement icons/styling
            {
                _ui->treePlaylist->SetItemTag(*mainSongNodeNew, PlaylistTreeModelNode::ItemTag::Normal, true);
            }

            // Auto-play
            if (shouldAutoPlay)
            {
                const PlaylistTreeModelNode* subsongItemData = _ui->treePlaylist->GetEffectiveInitialSubsong(*mainSongNodeNew);
                if (subsongItemData != nullptr) // Can be nullptr if the main song is not playable (e.g., missing ROM).
                {
                    shouldAutoPlay = subsongItemData->GetTag() != PlaylistTreeModelNode::ItemTag::Normal || !TryPlayPlaylistItem(*mainSongNodeNew);
                }
            }
        }

        if (playableTunesCount == 2)
        {
            UpdateUiState(); // Simply to enable the "next song" button immediately while still adding lots of files.
        }

        wxYield(); // Also must be before the HasFocus() call because not even that updates otherwise!
        if (_exitingApplication || !_addingFilesToPlaylist) // In case the user clicked Close (or cleared the playlist) while adding lots of files. This should be checked immediately after any wxYield.
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
        SendFilesToPlaylist(moreFiles, false, false); // Process another batch of enqueued files.
    }
    else // All enqueued files processed.
    {
        _addingFilesToPlaylist = false;
        PadColumnsWidth();
    }
}

void FramePlayer::PadColumnsWidth()
{
    // Pad the Title, Author and Copyright column widths a little because the bold text takes up some extra width so the text could become cutoff when hard-selected.
    _ui->treePlaylist->AutoFitTextColumn(PlaylistTreeModel::ColumnId::Title);
    _ui->treePlaylist->AutoFitTextColumn(PlaylistTreeModel::ColumnId::Author);
    _ui->treePlaylist->AutoFitTextColumn(PlaylistTreeModel::ColumnId::Copyright);
}

void FramePlayer::UpdateIgnoredSongs(PassKey<FramePrefs>)
{
    UpdateIgnoredSongs();
}

void FramePlayer::UpdateIgnoredSongs()
{
    for (const PlaylistTreeModelNodePtr& songNode : _ui->treePlaylist->GetSongs())
    {
        UpdateIgnoredSong(*songNode.get());
    }

    UpdateUiState();
}

void FramePlayer::UpdateIgnoredSong(PlaylistTreeModelNode& mainSongNode)
{
    const uint_least32_t skipDurationThreshold = static_cast<uint_least32_t>(_app.currentSettings->GetOption(Settings::AppSettings::ID::SkipShorter)->GetValueAsInt() * Const::MILLISECONDS_IN_SECOND);
    const uint_least32_t fallbackDuration = static_cast<uint_least32_t>(_app.currentSettings->GetOption(Settings::AppSettings::ID::SongFallbackDuration)->GetValueAsInt() * Const::MILLISECONDS_IN_SECOND);

    const PlaylistTreeModelNodePtrArray& subNodes = mainSongNode.GetChildren();

    // Subsongs
    for (const PlaylistTreeModelNodePtr& subsongNode : subNodes)
    {
        const uint_least32_t relevantSubsongDuration = (subsongNode->duration == 0) ? fallbackDuration : subsongNode->duration;
        const bool durationIsShort = skipDurationThreshold > 0 && (relevantSubsongDuration < skipDurationThreshold);

        const PlaylistTreeModelNode::ItemTag tag = (durationIsShort) ? PlaylistTreeModelNode::ItemTag::ShortDuration : PlaylistTreeModelNode::ItemTag::Normal;
        _ui->treePlaylist->SetItemTag(*subsongNode.get(), tag);
    }

    // Main song
    {
        bool mainSongDurationIsShort = false;

        if (mainSongNode.GetSubsongCount() == 0)
        {
            const uint_least32_t relevantSingleSongDuration = (mainSongNode.duration == 0) ? fallbackDuration : mainSongNode.duration;
            mainSongDurationIsShort = skipDurationThreshold > 0 && (relevantSingleSongDuration < skipDurationThreshold);
        }
        else
        {
            const bool allSubsongsAreShort = std::all_of(subNodes.cbegin(), subNodes.cend(), [](const PlaylistTreeModelNodePtr& subNode)
            {
                return subNode->GetTag() == PlaylistTreeModelNode::ItemTag::ShortDuration;
            });

            mainSongDurationIsShort = allSubsongsAreShort;
        }

        const PlaylistTreeModelNode::ItemTag tag = (mainSongDurationIsShort) ? PlaylistTreeModelNode::ItemTag::ShortDuration : PlaylistTreeModelNode::ItemTag::Normal;
        _ui->treePlaylist->SetItemTag(mainSongNode, tag);
    }
}

long FramePlayer::GetEffectiveSongDuration(const PlaylistTreeModelNode& node) const
{
    long effectiveDuration = static_cast<long>(node.duration);
    if (effectiveDuration == 0)
    {
        effectiveDuration = _app.currentSettings->GetOption(Settings::AppSettings::ID::SongFallbackDuration)->GetValueAsInt() * Const::MILLISECONDS_IN_SECOND;
    }

    return effectiveDuration;
}

Songlengths::HvscInfo FramePlayer::TryGetHvscInfo(const char* md5, int subsong) const
{
    if (!_sidDatabase.IsLoaded())
    {
        //throw std::runtime_error("Database wasn't loaded!");
        return Songlengths::HvscInfo(); // Dummy
    }

    return _sidDatabase.GetHvscInfo(md5, subsong);
}