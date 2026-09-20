/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2026 Jasmin Rutic (bytespiller@gmail.com)
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
#include "PlaylistLoading/PlaylistFileParser.h"
#include "../MyApp.h"
#include "../Config/AppSettings.h"
#include "../Config/UIStrings.h"
#include "../Helpers/HelpersWx.h"
#include "../UIElements/Playlist/Components/PlaylistModel.h"
#include "../../Util/Const.h"

#include <algorithm>
#include <chrono>
#include <unordered_set>

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

namespace
{
    constexpr int BATCH_APPLY_TIMER_INTERVAL_MS = 20;
    constexpr int BATCH_APPLY_MAX_PER_TICK = 1000; // Caps how many already-parsed songs get inserted into the model per timer tick, so the GUI thread stays responsive even during the (cheap, thanks to Playlist::AddMainSongs's single-notification batching, but non-zero) insertion work itself.
}

void FramePlayer::SendFilesToPlaylist(const wxArrayString& files, bool clearPrevious, bool autoPlayFirstImmediately)
{
    _addingFilesToPlaylist = true;
    AbortPlaylistLoad(); // Defensive: make sure any previous (in practice, always already-finished) background load is fully stopped before starting a new one.

    if (clearPrevious)
    {
        _app.StopPlayback();
        _app.UnloadActiveTune();

        SetStatusText(Strings::FramePlayer::STATUS_CLEARING_PLAYLIST, 2); // TODO
        _ui->treePlaylist->Clear();

        UpdateUiState();
        Update();
    }

    _loadEnabledShortSongSkip = _app.currentSettings->GetOption(Settings::AppSettings::ID::SkipShorter)->GetValueAsInt() > 0;
    _loadShouldAutoPlay = (autoPlayFirstImmediately) ? _app.currentSettings->GetOption(Settings::AppSettings::ID::AutoPlay)->GetValueAsBool() : false;
    _loadNextApplyIndex = 0;
    _loadPlayableTunesCount = 0;
    _loadLastProgressUiUpdate = std::chrono::steady_clock::time_point(); // Force the first tick's progress UI update through immediately.

    // Pre-pass: resolve MUS+STR companions up-front, order-independently, instead of the old sequential "is this the previous main song's companion?" check - this also lets us skip a companion .str file's own (would-be-redundant) background parse entirely, same as the original loop did for the sequential case.
    // Reminder: "files" is already a flat list of absolute paths (from Helpers::Wx::Files::GetValidFiles), so a plain wxString::Lower() is enough for the lookup sets below - do NOT wrap every file in a wxFileName here just to call GetFullPath(): at 60k+ files that parsing cost alone is enough to freeze the GUI thread for many seconds with zero progress feedback, defeating the entire point of this background-parsing feature. wxFileName is only needed (and only used below) for the much smaller subset of actual .mus files, to cleanly swap the extension when computing a candidate companion path.
    std::vector<SidFileParsePool::FileTask> tasks;
    tasks.reserve(files.GetCount());
    {
        std::unordered_set<wxString> discoveredPathsLower;
        discoveredPathsLower.reserve(files.GetCount());
        for (const wxString& f : files)
        {
            discoveredPathsLower.insert(f.Lower());
        }

        std::unordered_set<wxString> strPathsToSkipLower;
        std::vector<wxString> musCompanionForFile(files.GetCount());

        for (size_t i = 0; i < files.GetCount(); ++i)
        {
            const wxString& filepath = files[i];
            if (!filepath.Lower().EndsWith(".mus"))
            {
                continue;
            }

            // Check if the companion STR file exists as well, and load it in pair (MUS+STR)
            wxFileName extraStrFilePath(filepath);
            extraStrFilePath.SetExt("str");

            const bool exists =
                (Helpers::Wx::Files::IsWithinZipFile(extraStrFilePath.GetFullPath()) && Helpers::Wx::Files::FileExistsInZipArchive(extraStrFilePath.GetFullPath())) ||
                (extraStrFilePath.FileExists());

            if (exists)
            {
                musCompanionForFile[i] = extraStrFilePath.GetFullPath();

                const wxString strPathLower = extraStrFilePath.GetFullPath().Lower();
                if (discoveredPathsLower.count(strPathLower) != 0)
                {
                    strPathsToSkipLower.insert(strPathLower);
                }
            }
        }

        for (size_t i = 0; i < files.GetCount(); ++i)
        {
            const wxString& filepath = files[i];

            if (strPathsToSkipLower.count(filepath.Lower()) != 0)
            {
                continue; // Skip the duplicate STR file (MUS+STR already paired with its MUS file above).
            }

            SidFileParsePool::FileTask task;
            task.filepathUtf8 = std::string(filepath.ToUTF8());
            if (!musCompanionForFile[i].IsEmpty())
            {
                task.musCompanionStrFilePathUtf8 = std::string(musCompanionForFile[i].ToUTF8());
            }
            tasks.emplace_back(std::move(task));
        }
    }

    SetStatusText(wxString::Format(Strings::FramePlayer::STATUS_ADDING_FILES_WITH_COUNT, static_cast<int>(tasks.size())), 2);

    const unsigned int workerCount = static_cast<unsigned int>(std::max(1, _app.currentSettings->GetOption(Settings::AppSettings::ID::MaxParserThreads)->GetValueAsInt()));
    _parsePool.Start(std::move(tasks), workerCount, _sidDatabase, _stilInfo, _app.GetPlaybackInfo());

    _timerBatchApply->Start(BATCH_APPLY_TIMER_INTERVAL_MS);
    DoBatchApplyParsedSongs(); // Apply whatever's already available immediately (e.g. small playlists may finish parsing before the first timer tick even fires), rather than waiting a full tick.
}

void FramePlayer::DoBatchApplyParsedSongs()
{
    const size_t totalFiles = _parsePool.GetTotalCount();

    // First pass: gather this tick's chunk of already-parsed, in-order results as plain data, without touching the model yet.
    std::vector<std::unique_ptr<ParsedSongResult>> pendingResults;
    std::vector<UIElements::Playlist::MainSongData> pendingMainSongs;
    pendingResults.reserve(BATCH_APPLY_MAX_PER_TICK);
    pendingMainSongs.reserve(BATCH_APPLY_MAX_PER_TICK);

    int consideredThisTick = 0;
    while (consideredThisTick < BATCH_APPLY_MAX_PER_TICK && _loadNextApplyIndex < totalFiles)
    {
        std::unique_ptr<ParsedSongResult> result = _parsePool.TryTakeResult(_loadNextApplyIndex);
        if (result == nullptr)
        {
            break; // The next file (in original order) isn't parsed yet - wait for a later tick rather than applying out of order.
        }

        ++_loadNextApplyIndex;
        ++consideredThisTick;

        if (result->tuneIsValid)
        {
            UIElements::Playlist::MainSongData songData;
            songData.title = wxString::FromUTF8(result->title);
            songData.filepath = wxString::FromUTF8(result->filepath);
            songData.defaultSubsong = result->defaultSubsong;
            songData.duration = result->duration;
            songData.hvscPath = wxString::FromUTF8(result->hvscPath);
            songData.md5 = result->md5;
            songData.author = wxString::FromUTF8(result->author);
            songData.copyright = wxString::FromUTF8(result->copyright);
            songData.romRequirement = result->romRequirement;
            songData.playable = result->playable;
            songData.musCompanionStrFilePath = wxString::FromUTF8(result->musCompanionStrFilePath);

            pendingMainSongs.emplace_back(std::move(songData));
            pendingResults.emplace_back(std::move(result));
        }
    }

    // Bulk-insert this tick's main songs via a single model-change notification (see Playlist::AddMainSongs for why this matters at scale: repeated single AddMainSong() calls are each O(current top-level song count)).
    const std::vector<PlaylistTreeModelNode*> newNodes = _ui->treePlaylist->AddMainSongs(pendingMainSongs);

    // Second pass: gather subsongs for every song in this tick that has any, as plain data, without attaching them yet.
    std::vector<UIElements::Playlist::SubsongBatchEntry> subsongBatch;
    for (size_t i = 0; i < newNodes.size(); ++i)
    {
        const ParsedSongResult& result = *pendingResults[i];
        if (result.subsongDurations.empty())
        {
            continue;
        }

        UIElements::Playlist::SubsongBatchEntry entry;
        entry.parent = newNodes[i];
        entry.durations = result.subsongDurations;
        entry.titles.reserve(result.subsongTitles.size());
        for (const std::string& subsongTitleUtf8 : result.subsongTitles)
        {
            entry.titles.emplace_back(wxString::FromUTF8(subsongTitleUtf8));
        }

        subsongBatch.emplace_back(std::move(entry));
    }

    // Bulk-attach this tick's subsongs via a single Before/AfterReset cycle for the whole batch (see Playlist::AddSubsongsBatch for why this matters at scale: on wxGTK, each individual AddSubsongs() call's Before/AfterReset bracket is a full model reset costing O(current total playlist size), not just O(that song's own subsongs)).
    _ui->treePlaylist->AddSubsongsBatch(subsongBatch);

    // Third pass: per-song post-processing (tags, auto-play) now that every song in this tick has its subsongs (if any) already attached. None of this touches the top-level list structure - SetItemTag() only ever affects a single item's value - so it stays cheap regardless of overall playlist size.
    for (size_t i = 0; i < newNodes.size(); ++i)
    {
        PlaylistTreeModelNode& mainSongNodeNew = *newNodes[i];
        const ParsedSongResult& result = *pendingResults[i];

        if (result.playable)
        {
            ++_loadPlayableTunesCount;
        }

        if (!result.subsongDurations.empty() && !result.musCompanionStrFilePath.empty()) // MUS+STR tune
        {
            _ui->treePlaylist->SetItemTag(mainSongNodeNew.GetSubsong(2), PlaylistTreeModelNode::ItemTag::MUS_StandaloneMus, {});
            _ui->treePlaylist->SetItemTag(mainSongNodeNew.GetSubsong(3), PlaylistTreeModelNode::ItemTag::MUS_StandaloneStr, {});
        }

        // One tune (with any subsongs) added -----------------

        if (result.playable && _loadEnabledShortSongSkip) // Tag short songs
        {
            UpdateIgnoredSong(mainSongNodeNew);
        }
        else // Apply Normal tag and ROM requirement icons/styling
        {
            _ui->treePlaylist->SetItemTag(mainSongNodeNew, PlaylistTreeModelNode::ItemTag::Normal, true);
        }

        // Auto-play
        if (_loadShouldAutoPlay)
        {
            const PlaylistTreeModelNode* subsongItemData = _ui->treePlaylist->GetEffectiveInitialSubsong(mainSongNodeNew);
            if (subsongItemData != nullptr) // Can be nullptr if the main song is not playable (e.g., missing ROM).
            {
                _loadShouldAutoPlay = subsongItemData->GetTag() != PlaylistTreeModelNode::ItemTag::Normal || !TryPlayPlaylistItem(mainSongNodeNew);
            }
        }

        if (_loadPlayableTunesCount == 2)
        {
            UpdateUiState(); // Simply to enable the "next song" button immediately while still adding lots of files.
        }
    }

    if (_exitingApplication || !_addingFilesToPlaylist) // In case the user clicked Close (or cleared the playlist) while adding lots of files.
    {
        AbortPlaylistLoad();
        return;
    }

    const bool finished = (_loadNextApplyIndex >= totalFiles);

    // Progress UI (status text percentage + UpdatePlaylistPositionLabel()) is throttled to a fixed wall-clock cadence rather than updated on every tick.
    // Reminder: UpdatePlaylistPositionLabel() (when a song is already playing, e.g. via auto-play) sums durations across every song currently in the playlist - an O(current song count) scan. The original synchronous loop already throttled its equivalent status/label updates (roughly once per 100 files) for exactly this reason; doing it unconditionally per tick here made it run thousands of times against an ever-growing tens-of-thousands-strong list, which (compounded with the app's own independent, similarly-frequent playback-refresh timer once auto-play kicks in) is expensive enough on its own to look like another freeze, even with the O(N^2) top-level-insertion bug (see Playlist::AddMainSongs) already fixed.
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (finished || now - _loadLastProgressUiUpdate >= std::chrono::milliseconds(250))
    {
        _loadLastProgressUiUpdate = now;

        if (totalFiles > 0)
        {
            const wxString textAddingFilesWithCount = wxString::Format(Strings::FramePlayer::STATUS_ADDING_FILES_WITH_COUNT, static_cast<int>(totalFiles));
            const int currentPercentage = static_cast<int>((static_cast<float>(_loadNextApplyIndex) / static_cast<float>(totalFiles)) * 100.0f);
            SetStatusText(wxString::Format("%s (%i%%)", textAddingFilesWithCount, currentPercentage), 2);
        }

        UpdatePlaylistPositionLabel();
    }

    if (finished)
    {
        _timerBatchApply->Stop();
        OnPlaylistLoadFinished();
    }
}

void FramePlayer::OnPlaylistLoadFinished()
{
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

void FramePlayer::AbortPlaylistLoad()
{
    if (_timerBatchApply != nullptr)
    {
        _timerBatchApply->Stop();
    }

    _parsePool.Abort();
}

void FramePlayer::OnTimerBatchApply(wxTimerEvent& WXUNUSED(evt))
{
    DoBatchApplyParsedSongs();
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
        if (subsongNode->GetTag() == PlaylistTreeModelNode::ItemTag::MUS_StandaloneMus || subsongNode->GetTag() == PlaylistTreeModelNode::ItemTag::MUS_StandaloneStr)
        {
            continue;
        }

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

