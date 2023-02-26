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
#include "../MyApp.h"
#include "../Config/AppSettings.h"

bool FramePlayer::TryPlayPlaylistItem(const SongTreeItemData& itemData)
{
    if (_ui->treePlaylist->IsPlayableItem(itemData))
    {
        const Settings::Option* const optionDefaultSubsong = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatModeDefaultSubsong);
        const int subsong = (itemData.IsSubsong() || optionDefaultSubsong->GetValueAsBool()) ? itemData.GetDefaultSubsong() : _ui->treePlaylist->GetEffectiveDefaultOrFirstPlayableSubsong(itemData)->GetDefaultSubsong();
        const bool sameTune = _app.GetPlaybackInfo().GetCurrentTuneFilePath() == itemData.GetFilePath().ToStdWstring();

        const int preRenderDurationMs = (_app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool()) ? GetEffectiveSongDuration(&itemData) : 0;
        if (sameTune)
        {
            _app.PlaySubsong(subsong, preRenderDurationMs); // Switch an already-loaded tune to subsong.
        }
        else
        {
            _app.Play(itemData.GetFilePath(), subsong, preRenderDurationMs);
        }

        const bool fileLoadedSuccessfully = _app.GetPlaybackInfo().GetCurrentTuneFilePath() == itemData.GetFilePath();
        const bool highlightable = fileLoadedSuccessfully && _ui->treePlaylist->TryHighlightPlaylistItem(itemData.GetId(), subsong, _app.currentSettings->GetOption(Settings::AppSettings::ID::AutoExpandSubsongs)->GetValueAsBool());
        UpdateUiState();

        if (highlightable && _app.currentSettings->GetOption(Settings::AppSettings::ID::SelectionFollowsPlayback)->GetValueAsBool())
        {
            _ui->treePlaylist->SelectItem(itemData.GetId());

            const wxTreeItemId& cItemId = _ui->treePlaylist->TryGetCurrentSongTreeItemData()->GetId();
            if (cItemId.IsOk()) // Ensure a subsong itself is scrolled into view (not just its parent).
            {
                _ui->treePlaylist->EnsureVisible(cItemId);
            }
        }

        return true;
    }

    return false;
}

bool FramePlayer::TryPlayNextValidSong()
{
    const SongTreeItemData* const nextValidSong = _ui->treePlaylist->GetNextSong();
    if (nextValidSong != nullptr)
    {
        return TryPlayPlaylistItem(*nextValidSong);
    }

    return false;
}

bool FramePlayer::TryPlayPrevValidSong()
{
    const SongTreeItemData* const prevValidSong = _ui->treePlaylist->GetPrevSong();
    if (prevValidSong != nullptr)
    {
        return TryPlayPlaylistItem(*prevValidSong);
    }

    return false;
}

bool FramePlayer::TryPlayNextValidSubsong()
{
    const SongTreeItemData* const nextValidSubsong = _ui->treePlaylist->GetNextSubsong();
    if (nextValidSubsong != nullptr)
    {
        return TryPlayPlaylistItem(*nextValidSubsong);
    }

    return false;
}

bool FramePlayer::TryPlayPrevValidSubsong()
{
    const SongTreeItemData* const prevValidSubSong = _ui->treePlaylist->GetPrevSubsong();
    if (prevValidSubSong != nullptr)
    {
        return TryPlayPlaylistItem(*prevValidSubSong);
    }

    return false;
}
