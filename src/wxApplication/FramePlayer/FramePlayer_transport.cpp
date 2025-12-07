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
#include "../MyApp.h"
#include "../Config/AppSettings.h"
#include "../FrameChildren/FrameTuneInfo/FrameTuneInfo.h"

bool FramePlayer::TryPlayPlaylistItem(const PlaylistTreeModelNode& activatedNode)
{
    if (!activatedNode.IsPlayable())
    {
        return false;
    }

    int subsong = activatedNode.defaultSubsong;

    const PlaylistTreeModelNode* nodeToPlay = &activatedNode;

    // If the selected node is a mainsong, determine the initial subsong to play.
    if (activatedNode.type == PlaylistTreeModelNode::ItemType::Song && activatedNode.GetSubsongCount() > 0)
    {
        if (const PlaylistTreeModelNode* const subNode = _ui->treePlaylist->GetEffectiveInitialSubsong(activatedNode))
        {
            subsong = subNode->defaultSubsong;
            nodeToPlay = subNode;
        }
        else // All subsongs tagged for navigation auto-skip. Just expand the node but don't start any playback.
        {
            _ui->treePlaylist->ExpandSongNode(activatedNode);
            return false;
        }
    }

#ifdef WIN32
    wxSafeYield(); // Allow buttons to refresh to unclicked state before the blocking SID loading. Note: Linux (Wayland?) doesn't like this (may immediately mark the app as not responding).
#endif

    // Trigger playback
    wxString targetFilepath = nodeToPlay->filepath;
    const int preRenderDurationMs = (_app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool()) ? GetEffectiveSongDuration(*nodeToPlay) : 0;
    const bool sameTune = _app.GetPlaybackInfo().GetCurrentTuneFilePath() == targetFilepath.ToStdWstring();
    if (sameTune && nodeToPlay->musCompanionStrFilePath.IsEmpty())
    {
        _app.PlaySubsong(subsong, preRenderDurationMs); // Switch an already-loaded tune to subsong.
    }
    else
    {
        wxFileName musCompanionStrFilePath(nodeToPlay->musCompanionStrFilePath); // MUS+STR (if STR is available)

        switch (nodeToPlay->GetTag())
        {
            case PlaylistTreeModelNode::ItemTag::MUS_StandaloneMus: // Load MUS without STR
                musCompanionStrFilePath.Clear();
                break;
            case PlaylistTreeModelNode::ItemTag::MUS_StandaloneStr: // Load STR without MUS
                musCompanionStrFilePath.SetExt("str");
                targetFilepath = musCompanionStrFilePath.GetFullPath();
                musCompanionStrFilePath.Clear();
                break;
        }

        _app.Play(targetFilepath, subsong, preRenderDurationMs, musCompanionStrFilePath.GetFullPath());
    }

    // Highlight the item in the playlist if the playback started successfully (file exists etc.)
    const bool fileLoadedSuccessfully = _app.GetPlaybackInfo().GetCurrentTuneFilePath() == targetFilepath.ToStdWstring();
    const bool highlightable = fileLoadedSuccessfully && _ui->treePlaylist->TrySetActiveSong(*nodeToPlay, _app.currentSettings->GetOption(Settings::AppSettings::ID::AutoExpandSubsongs)->GetValueAsBool());
    UpdateUiState();

    if (highlightable && _app.currentSettings->GetOption(Settings::AppSettings::ID::SelectionFollowsPlayback)->GetValueAsBool())
    {
        _ui->treePlaylist->Select(*nodeToPlay);
        _ui->treePlaylist->EnsureVisible(*nodeToPlay); // Ensure a subsong itself is scrolled into view (not just its parent).
    }

    if (_frameTuneInfo != nullptr && _frameTuneInfo->ShouldFollowPlayback() && _app.GetPlaybackInfo().IsValidSongLoaded())
    {
        ShowTuneInfo();
    }

    return true;
}

bool FramePlayer::TryPlayNextValidSong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetNextSong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}

bool FramePlayer::TryPlayPrevValidSong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetPrevSong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}

bool FramePlayer::TryPlayNextValidSubsong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetNextSubsong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}

bool FramePlayer::TryPlayPrevValidSubsong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetPrevSubsong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}
