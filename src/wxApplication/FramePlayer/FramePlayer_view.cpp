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
#include "../Config/UIStrings.h"
#include "../Helpers/HelpersWx.h"
#include "../FrameChildren/FramePlaybackMods/FramePlaybackMods.h"
#include "../../Util/Const.h"

void FramePlayer::UpdateUiState()
{
    const bool playlistHasItems = !_ui->treePlaylist->IsEmpty();
    const PlaybackController& playback = _app.GetPlaybackInfo();
    const bool hasSomethingPlayable = playlistHasItems && playback.IsValidSongLoaded();

    _ui->EnablePlaybackControls(hasSomethingPlayable); // Pre enable/disable all.
    if (hasSomethingPlayable)
    {
        // Subsong buttons
        _ui->btnNextSubsong->Enable(_ui->treePlaylist->GetNextSubsong() != nullptr);
        _ui->btnPrevSubsong->Enable(_ui->treePlaylist->GetPrevSubsong() != nullptr);
        _ui->labelSubsong->Enable(playback.GetTotalSubsongs() > 1);

        // Transport buttons
        const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetActiveSong();
        _ui->btnNextTrack->Enable(node != nullptr && _ui->treePlaylist->GetNextSong(*node) != nullptr);
        _ui->btnPrevTrack->Enable(node != nullptr && _ui->treePlaylist->GetPrevSong(*node) != nullptr);

        // Other things to refresh regardless of the playback state
        SetStatusText(wxString::Format("%s / %s", playback.GetCurrentTuneSpeedDescription(), playback.GetCurrentTuneSidDescription()), 1); // TODO
    }

    const PlaybackController::State playbackState = playback.GetState();
    switch (playbackState)
    {
        case PlaybackController::State::Undefined:
        {
            // fall-through to Stopped
        }
        case PlaybackController::State::Stopped:
        {
            UpdatePeriodicDisplays(0);
            _ui->btnPlayPause->SetPlay();
            _ui->btnPlayPause->Enable(hasSomethingPlayable);
            _ui->btnStop->Enable(false);
            _ui->labelTime->Enable(false);
            _ui->compositeSeekbar->ResetPlaybackPosition(0); // Will also auto disable/enable itself.
            _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_NO_PROGRESS);

            _ui->waveformVisualization->Clear();

            break;
        }
        case PlaybackController::State::Playing:
        {
            _ui->btnPlayPause->SetPause();
            _ui->compositeSeekbar->ResetPlaybackPosition(GetEffectiveSongDuration(*_ui->treePlaylist->GetActiveSong()));
            _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_NORMAL);

            break;
        }
        case PlaybackController::State::Paused:
        {
            _ui->btnPlayPause->SetPlay();
            _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_PAUSED);
            break;
        }
        case PlaybackController::State::Seeking:
        {
            const PlaybackController::State resumeState = playback.GetResumeState();
            (resumeState == PlaybackController::State::Paused) ? _ui->btnPlayPause->SetPlay() : _ui->btnPlayPause->SetPause();
            _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_INDETERMINATE);
            break;
        }
        default:
        {
            throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
        }
    };

    DisplayCurrentSongInfo(!hasSomethingPlayable);

    if (playbackState != PlaybackController::State::Seeking && playbackState != PlaybackController::State::Stopped)  // If seeking, don't update immediately, because the GetTime() call would erroneously return 0 (causing seekbar to flicker once).
    {
        UpdatePeriodicDisplays(_app.GetPlaybackInfo().GetTime());
    }

    SetRefreshTimerThrottled(!hasSomethingPlayable); // This also (re)starts it.
}

void FramePlayer::UpdatePlaybackStatusBar()
{
    wxFont statusFont(GetStatusBar()->GetFont());
    statusFont.SetWeight(wxFONTWEIGHT_NORMAL);

    const PlaybackController& playback = _app.GetPlaybackInfo();
    switch (playback.GetState())
    {
        case PlaybackController::State::Paused:
            SetStatusText(Strings::FramePlayer::STATUS_PAUSED, 0); // TODO: magic status indices
            break;

        case PlaybackController::State::Playing:
        {
            const bool preRendering = _app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool();
            wxString status((preRendering) ? Strings::FramePlayer::STATUS_PLAYING_PRERENDER : Strings::FramePlayer::STATUS_PLAYING);
            if (playback.GetPlaybackSpeedFactor() != 1.0 || !playback.AreAllRelevantVoicesEnabled())
            {
                status.append(Strings::FramePlayer::STATUS_MODIFIED_SUFFIX);
                statusFont.SetWeight(wxFONTWEIGHT_BOLD);
            }
            SetStatusText(status, 0);

            const std::string& typeDesc = playback.GetCurrentTuneTypeDescription();
            const wxString& romInfoSuffix = (typeDesc.empty()) ? "" : wxString::Format(" (%s)", typeDesc);

            SetStatusText(wxString::Format("%i%s%s", playback.GetCurrentTuneSize(), Strings::FramePlayer::STATUS_BYTES_SUFFIX, romInfoSuffix), 2);

            break;
        }

        case PlaybackController::State::Seeking:
            SetStatusText(Strings::FramePlayer::STATUS_SEEKING, 0);
            break;

        case PlaybackController::State::Stopped:
            SetStatusText(Strings::FramePlayer::STATUS_STOPPED, 0);
            SetStatusText("", 1);
            SetStatusText("", 2);
            break;

        case PlaybackController::State::Undefined:
            SetStatusText(Strings::FramePlayer::STATUS_ERROR, 0);
            SetStatusText("", 1);
            SetStatusText("", 2);
            break;

        default:
            std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
            break;
    }

    GetStatusBar()->SetFont(statusFont);
}

void FramePlayer::UpdatePeriodicDisplays(const uint_least32_t playbackTimeMs)
{
    uint_least32_t displayTimeMs = playbackTimeMs;

    // Seekbar
    _ui->compositeSeekbar->UpdatePlaybackPosition(static_cast<long>(playbackTimeMs), _app.GetPlaybackInfo().GetPreRenderProgressFactor());

    // Time position label
    wxFont font(_ui->labelTime->GetFont());
    if (_ui->compositeSeekbar->IsSeekPreviewing())
    {
        displayTimeMs = _ui->compositeSeekbar->GetNormalizedFillTarget() * _ui->compositeSeekbar->GetDurationValue();
        if (font.GetWeight() != wxFONTWEIGHT_BOLD)
        {
            font.MakeBold();
            _ui->labelTime->SetFont(font);
        }
    }
    else
    {
        if (font.GetWeight() != wxFONTWEIGHT_NORMAL)
        {
            font.SetWeight(wxFONTWEIGHT_NORMAL);
            _ui->labelTime->SetFont(font);
        }
    }

    _ui->labelTime->SetLabelText(Helpers::Wx::GetTimeFormattedString(displayTimeMs));

    // Visualization
    _app.GetVisualizationWaveform(_ui->waveformVisualization->GetBuffer());
    _ui->waveformVisualization->Refresh();
}

void FramePlayer::DisplayCurrentSongInfo(bool justClear)
{
    if (justClear)
    {
        _ui->labelTitle->SetLabelText("");
        _ui->labelAuthor->SetLabelText("");
        _ui->labelCopyright->SetLabelText("");
        _ui->labelSubsong->SetLabelText("0 / 0");
    }
    else
    {
        const PlaybackController& playback = _app.GetPlaybackInfo();
        _ui->labelTitle->SetLabelText(playback.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Title));
        _ui->labelAuthor->SetLabelText(playback.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Author));
        _ui->labelCopyright->SetLabelText(playback.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Released));

        _ui->labelSubsong->SetLabelText(wxString::Format("%i / %i", playback.GetCurrentSubsong(), playback.GetTotalSubsongs()));
    }

    UpdatePlaybackStatusBar();
}

void FramePlayer::SetRefreshTimerThrottled(bool throttle)
{
    const int targetInterval = (throttle) ? TIMER_REFRESH_INTERVAL_IDLE : TIMER_REFRESH_INTERVAL_HIGH;
    SetRefreshTimerInterval(targetInterval);
}
