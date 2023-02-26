/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2023 Jasmin Rutic (bytespiller@gmail.com)
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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "Config/AppSettings.h"
#include "SingleInstanceManager/SingleInstanceManager.h"
#include "FramePlayer/FramePlayer.h"
#include "../PlaybackController/PlaybackController.h"
#include "../Util/SimpleTimer.h"
#include "../Util/SimpleSignal/SimpleSignalProvider.h"
#include "../Util/SimpleSignal/SimpleSignalListener.h"

#include <memory>

enum class SignalsMyApp
{
    SIGNAL_SEEKING_CEASED
};

class MyApp : public wxApp, public SimpleSignalProvider<SignalsMyApp>, private SimpleSignalListener<SignalsPlaybackController>
{
public:
    MyApp() = default;

public:
    bool OnInit() override;

private:
    void HandoffToCanonicalInstance();

public:
    void Play(const wxString& filename, unsigned int subsong, int preRenderDurationMs); // TODO: PassKey or something to allow calling this by the TryPlayPlaylistItem method only?
    void ReplayLoadedTune(int preRenderDurationMs);
    void PausePlayback();
    void ResumePlayback();
    void StopPlayback();
    void PlaySubsong(int subsong, int preRenderDurationMs);

    void SetVolume(float volume);
    bool ImmediatelyUpdateVolumeBoost8580();
    void SeekTo(uint_least32_t timeMs);

    void SetPlaybackSpeed(double factor);
    void ToggleVoice(unsigned int sidNum, unsigned int voice, bool enable);
    const PlaybackController& GetPlaybackInfo() const;
    bool ReapplyPlaybackSettings();
    void UnloadActiveTune();

    SimpleSignalProvider<SignalsPlaybackController>& GetPlaybackSignalProvider();

    // Informs user about a critical problem. Returns true if settings were optionally reset. It's up to caller to decide what to do next.
    bool ResetToDefaultsRecovery(const wxString& message);

protected:
    void Execute(Subscriber& subscriber, int param) override;

private:
    void OnSeekingCeased();

    void RunOnMainThread(std::function<void()> fn);

    void FinalizePlaybackStarted();
    void PopSilencer();

public:
    std::unique_ptr<Settings::AppSettings> currentSettings;

private:
    FramePlayer* _framePlayer = nullptr;
    std::unique_ptr<PlaybackController> _playback;
    std::unique_ptr<SingleInstanceManager> _instanceManager;
    std::unique_ptr<SimpleTimer> _popSilencer;
};
