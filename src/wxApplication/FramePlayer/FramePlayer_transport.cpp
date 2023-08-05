/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2024 Jasmin Rutic (bytespiller@gmail.com)
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

bool FramePlayer::TryPlayPlaylistItem(const PlaylistTreeModelNode& node)
{
    if (!node.IsPlayable())
    {
        return false;
    }

    int subsong = node.defaultSubsong;

    // If the selected node is a mainsong, determine the initial subsong to play.
    if (node.type == PlaylistTreeModelNode::ItemType::Song && node.GetSubsongCount() > 0)
    {
        if (const PlaylistTreeModelNode* const subNode = _ui->treePlaylistNew->GetEffectiveInitialSubsong(node))
        {
            subsong = subNode->defaultSubsong;
        }
        else // All subsongs tagged for navigation auto-skip. Just expand the node but don't start any playback.
        {
            _ui->treePlaylistNew->ExpandSongNode(node);
            return false;
        }
    }

    // Trigger playback
    const int preRenderDurationMs = (_app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool()) ? GetEffectiveSongDuration(node) : 0;
    const bool sameTune = _app.GetPlaybackInfo().GetCurrentTuneFilePath() == node.filepath.ToStdWstring();
    if (sameTune)
    {
        _app.PlaySubsong(subsong, preRenderDurationMs); // Switch an already-loaded tune to subsong.
    }
    else
    {
        _app.Play(node.filepath, subsong, preRenderDurationMs);
    }

    // Highlight the item in the playlist if the playback started successfully (file exists etc.)
    const PlaylistTreeModelNode& actualNode = (node.type == PlaylistTreeModelNode::ItemType::Song && node.GetSubsongCount() > 0) ? *_ui->treePlaylistNew->GetSubsong(node.filepath, subsong) : node;
    const bool fileLoadedSuccessfully = _app.GetPlaybackInfo().GetCurrentTuneFilePath() == actualNode.filepath.ToStdWstring();
    const bool highlightable = fileLoadedSuccessfully && _ui->treePlaylistNew->TrySetActiveSong(actualNode, _app.currentSettings->GetOption(Settings::AppSettings::ID::AutoExpandSubsongs)->GetValueAsBool());
    UpdateUiState();

    if (highlightable && _app.currentSettings->GetOption(Settings::AppSettings::ID::SelectionFollowsPlayback)->GetValueAsBool())
    {
        _ui->treePlaylistNew->Select(actualNode);
        _ui->treePlaylistNew->EnsureVisible(actualNode); // Ensure a subsong itself is scrolled into view (not just its parent).
    }

    return true;
}

bool FramePlayer::TryPlayNextValidSong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylistNew->GetNextSong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}

bool FramePlayer::TryPlayPrevValidSong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylistNew->GetPrevSong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}

bool FramePlayer::TryPlayNextValidSubsong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylistNew->GetNextSubsong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}

bool FramePlayer::TryPlayPrevValidSubsong()
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylistNew->GetPrevSubsong();
    if (node != nullptr)
    {
        return TryPlayPlaylistItem(*node);
    }

    return false;
}
