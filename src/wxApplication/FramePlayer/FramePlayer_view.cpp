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
 #include "../Config/UIStrings.h"
 #include "../Helpers/HelpersWx.h"
 #include "../FrameChildren/FramePlaybackMods/FramePlaybackMods.h"
 #include "../../Util/Const.h"
 #include <chrono>
 #include <numeric>

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
 #ifdef WIN32
             _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_NO_PROGRESS);
 #endif

             _ui->waveformVisualization->Clear();

             break;
         }
         case PlaybackController::State::Playing:
         {
             _ui->btnPlayPause->SetPause();
             _ui->compositeSeekbar->ResetPlaybackPosition(GetEffectiveSongDuration(*_ui->treePlaylist->GetActiveSong()));
 #ifdef WIN32
             _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_NORMAL);
 #endif

             break;
         }
         case PlaybackController::State::Paused:
         {
             _ui->btnPlayPause->SetPlay();
 #ifdef WIN32
             _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_PAUSED);
 #endif
             break;
         }
         case PlaybackController::State::Seeking:
         {
             const PlaybackController::State resumeState = playback.GetResumeState();
             (resumeState == PlaybackController::State::Paused) ? _ui->btnPlayPause->SetPlay() : _ui->btnPlayPause->SetPause();
 #ifdef WIN32
             _ui->compositeSeekbar->SetTaskbarProgressState(wxTASKBAR_BUTTON_INDETERMINATE);
 #endif
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
             // Leftmost
             const bool preRendering = _app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool();
             wxString status((preRendering) ? Strings::FramePlayer::STATUS_PLAYING_PRERENDER : Strings::FramePlayer::STATUS_PLAYING);
             if (playback.GetPlaybackSpeedFactor() != 1.0 || !playback.AreRelevantSidsFullyEnabled())
             {
                 status.append(Strings::FramePlayer::STATUS_MODIFIED_SUFFIX);
                 statusFont.SetWeight(wxFONTWEIGHT_BOLD);
             }

             SetStatusText(status, 0);

             // Rightmost
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

 static class SeekMetrics
 {
     static constexpr std::chrono::milliseconds MIN_UPDATE_INTERVAL = static_cast<std::chrono::milliseconds>(200); // Ensures the display isn't updated too often.

 public:
     SeekMetrics() = default;

     // This constructor also snapshots the initial seek progress for convenience.
     SeekMetrics(const std::chrono::steady_clock::time_point timeNow, const uint_least32_t playbackTimeMs)
     {
         TrySnapshotSeekProgress(timeNow, playbackTimeMs);
     }

     SeekMetrics(SeekMetrics&) = delete;

 public:
     // If this instance was marked as expired, you'll need to initialize a new one.
     inline bool IsExpired() const
     {
         return _expired;
     }

     // Marks this instance as expired.
     void Expire()
     {
         _expired = true;
     }

     // Snapshots a seek progress for use in calculating the average seek speed over the update interval and returns true. Returns false if this class instance has expired and the snapshot can't happen.
     bool TrySnapshotSeekProgress(const std::chrono::steady_clock::time_point timeNow, const uint_least32_t playbackTimeMs)
     {
         if (playbackTimeMs < _previousPlaybackTimeMs)
         {
             _expired = true; // Seeking backwards was initiated while seeking forward was still underway.
         }

         if (_expired)
         {
             return false;
         }

         const uint_least32_t realtimeDelta = static_cast<uint_least32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - _previousTimestamp).count());
         if (realtimeDelta > 0)
         {
             const uint_least32_t playbackTimePositionDelta = playbackTimeMs - _previousPlaybackTimeMs;

             const uint_least32_t currentSpeed = playbackTimePositionDelta / std::max(static_cast<uint_least32_t>(1), realtimeDelta);
             _speedSnapshots.emplace_back(currentSpeed);

             _ready = true;
         }

         _previousTimestamp = timeNow;
         _previousPlaybackTimeMs = playbackTimeMs;

         return true;
     }

     // Calls your callback (with a rolling average result in between) if the time since the last call reached the minimum update interval.
     bool UpdateIfNeeded(const std::chrono::steady_clock::time_point timeNow, std::function<void(uint_least32_t)> callback)
     {
         if (_expired)
         {
             throw std::runtime_error("This SeekMetrics instance has been marked as expired!");
         }

         if (_ready && std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - _lastUpdateTimestamp) >= SeekMetrics::MIN_UPDATE_INTERVAL)
         {
             _lastUpdateTimestamp = timeNow;

             const uint_least32_t currentDisplayResult = std::accumulate(_speedSnapshots.begin(), _speedSnapshots.end(), 0) / _speedSnapshots.size();
             _speedSnapshots.clear();

             callback(currentDisplayResult);
             return true;
         }

         return false;
     }

 private:
     bool _ready = false;
     bool _expired = false;

     std::vector<uint_least32_t> _speedSnapshots;
     std::chrono::steady_clock::time_point _lastUpdateTimestamp;

     uint_least32_t _previousPlaybackTimeMs = 0;
     std::chrono::steady_clock::time_point _previousTimestamp = std::chrono::steady_clock::now();
 } seekMetrics;

 void FramePlayer::UpdatePeriodicDisplays(const uint_least32_t playbackTimeMs)
 {
     uint_least32_t displayTimeMs = playbackTimeMs;
     const PlaybackController& playback = _app.GetPlaybackInfo();

     // Seekbar
     _ui->compositeSeekbar->UpdatePlaybackPosition(static_cast<long>(playbackTimeMs), playback.GetPreRenderProgressFactor());

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

     // Seeking speed indicator
     if (playback.GetState() == PlaybackController::State::Seeking)
     {
         const std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();

         if (!seekMetrics.TrySnapshotSeekProgress(timeNow, playbackTimeMs))
         {
             seekMetrics = SeekMetrics(timeNow, playbackTimeMs);
         }

         seekMetrics.UpdateIfNeeded(timeNow, [this, playbackTimeMs, &playback](uint_least32_t seekingSpeed)
         {
             const uint_least32_t target = playback.GetLastSeekTargetMs();
             const uint_least32_t remainingSeconds = (target - playbackTimeMs) / (std::max(static_cast<uint_least32_t>(1), seekingSpeed) * 1000);

             wxString statusText(Strings::FramePlayer::STATUS_SEEKING);

             // Reminder: don't declare as a reference (due to way append() gets optimized in the release mode, we'd get gibberish)
             const wxString etaText = "ETA " + std::to_string(remainingSeconds + 1).append("s");
             const wxString speedText = wxString(std::to_string(seekingSpeed)).append('x');

             SetStatusText(statusText.append(' ').append(etaText).append(" @ ").append(speedText), 0);
         });
     }
     else if (!seekMetrics.IsExpired())
     {
         seekMetrics.Expire();
     }
 }

 void FramePlayer::DisplayCurrentSongInfo(bool justClear)
 {
     if (justClear)
     {
         _ui->labelTitle->SetLabelText("");
         _ui->labelAuthor->SetLabelText("");
         _ui->labelCopyright->SetLabelText("");
         _ui->labelSubsong->SetLabelText("0 / 0");

         _ui->labelStilNameTitle->SetText("");
         _ui->labelStilArtistAuthor->SetText("");
         _ui->labelStilComment->SetText("");
     }
     else
     {
         const PlaybackController& playback = _app.GetPlaybackInfo();
         const int subsong = playback.GetCurrentSubsong();
         const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetActiveSong();

         // Set song info labels
         if (node)
         {
             _ui->labelTitle->SetLabelText((playback.GetTotalSubsongs() <= 1)
                 ? Helpers::Wx::StringFromWin1252(node->title.ToStdString()).Trim(false) // Tune without subsongs.
                 : wxString::Format("%s: %s %i", playback.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Title), Strings::PlaylistTree::SUBSONG, subsong)); // Subsong

             _ui->labelAuthor->SetLabelText(Helpers::Wx::StringFromWin1252(playback.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Author)));
             _ui->labelCopyright->SetLabelText(Helpers::Wx::StringFromWin1252(playback.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Released)));
             _ui->labelSubsong->SetLabelText(wxString::Format("%i / %i", subsong, playback.GetTotalSubsongs()));
         }

         // Set STIL labels
         if (node)
         {
             const std::string& NONE("/");
             const std::string& SEPARATOR("  |  ");

             const Stil::Info& stil = _stilInfo.Get(node->hvscPath.ToStdString());

             // "Name - Title"
             {
                 std::string namesTitles = stil.GetFieldAsString(stil.names, subsong, SEPARATOR, true);
                 const std::string& titles = stil.GetFieldAsString(stil.titles, subsong, SEPARATOR, true);
                 if (!namesTitles.empty() && !titles.empty())
                 {
                     namesTitles.append(" - ");
                 }

                 const wxString& final = Helpers::Wx::StringFromWin1252(namesTitles.append(titles));
                 _ui->labelStilNameTitle->SetText((final.empty()) ? NONE : final);
             }

             // "Artist (Author)"
             {
                 std::string artistsAuthors = stil.GetFieldAsString(stil.artists, subsong, SEPARATOR, true);
                 const std::string& authors = stil.GetFieldAsString(stil.authors, subsong, SEPARATOR, true);

                 const bool bothPresent = !artistsAuthors.empty() && !authors.empty();
                 if (bothPresent)
                 {
                     artistsAuthors.append(" (");
                 }

                 artistsAuthors.append(authors);

                 if (bothPresent)
                 {
                     artistsAuthors.append(")");
                 }

                 _ui->labelStilArtistAuthor->SetText((artistsAuthors.empty()) ? NONE : Helpers::Wx::StringFromWin1252(artistsAuthors));
             }

             // Comment(s)
             {
                 std::string comments = stil.GetFieldAsString(stil.comments, subsong, SEPARATOR, true);

                 if (comments.empty())
                 {
                     // Try to get MUS comments instead
                     const std::string& musComments = playback.GetCurrentTuneMusComments();
                     if (!musComments.empty())
                     {
                         comments = musComments;
                     }
                 }

                 _ui->labelStilComment->SetText((comments.empty()) ? NONE : Helpers::Wx::StringFromWin1252(comments));
             }
         }
     }

     UpdatePlaybackStatusBar();
 }

 void FramePlayer::SetRefreshTimerThrottled(bool throttle)
 {
     const int targetInterval = (throttle) ? TIMER_REFRESH_INTERVAL_IDLE : TIMER_REFRESH_INTERVAL_HIGH;
     SetRefreshTimerInterval(targetInterval);
 }
