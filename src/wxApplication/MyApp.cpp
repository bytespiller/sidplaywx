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

#include "MyApp.h"
#include "Config/AppSettings.h"
#include "Config/UIStrings.h"
#include "Helpers/HelpersWx.h"
#include "SingleInstanceManager/IpcSetup.h"
#include "../Util/BufferHolder.h"
#include "../PlaybackController/Util/RomUtil.h"
#include <wx/stdpaths.h>

namespace
{
    wxMilliClock_t lastFileListReceptionTime = 0;

    void WarnRomLoadFailed(const wxString& romPath, const char* errMessage)
    {
        const wxString additionalInfo = wxFileExists(romPath) ? "" : wxString::Format("\n%s", Strings::Error::MSG_ERR_ROM_FILE_NOT_FOUND);
        wxMessageBox(errMessage + additionalInfo, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR);
    }

    struct ExtractedC64ModelSetting
    {
        ExtractedC64ModelSetting() = delete;
        explicit ExtractedC64ModelSetting(Settings::AppSettings& settings)
        {
            using DefaultC64Model = Settings::AppSettings::DefaultC64Model;

            const int intValue = settings.GetOption(Settings::AppSettings::ID::DefaultC64Model)->GetValueAsInt();
            const DefaultC64Model c64ModelOption = static_cast<DefaultC64Model>(intValue);

            if (c64ModelOption == Settings::AppSettings::DefaultC64Model::Prefer_PAL || c64ModelOption == DefaultC64Model::Force_PAL)
            {
                defaultC64Model = SidConfig::c64_model_t::PAL;
            }
            else if (c64ModelOption == DefaultC64Model::Prefer_NTSC || c64ModelOption == DefaultC64Model::Force_NTSC)
            {
                defaultC64Model = SidConfig::c64_model_t::NTSC;
            }
            else if (c64ModelOption == DefaultC64Model::Prefer_Old_NTSC || c64ModelOption == DefaultC64Model::Force_Old_NTSC)
            {
                defaultC64Model = SidConfig::c64_model_t::OLD_NTSC;
            }
            else if (c64ModelOption == DefaultC64Model::Prefer_Drean || c64ModelOption == DefaultC64Model::Force_Drean)
            {
                defaultC64Model = SidConfig::c64_model_t::DREAN;
            }
            else if (c64ModelOption == DefaultC64Model::Prefer_PAL_M || c64ModelOption == DefaultC64Model::Force_PAL_M)
            {
                defaultC64Model = SidConfig::c64_model_t::PAL_M;
            }

            forceC64Model = intValue >= static_cast<int>(DefaultC64Model::Force_PAL);
        };

        SidConfig::c64_model_t defaultC64Model = SidConfig::c64_model_t::PAL;
        bool forceC64Model = false;
    };

    struct ExtractedSidModelSetting
    {
        ExtractedSidModelSetting() = delete;
        explicit ExtractedSidModelSetting(Settings::AppSettings& settings)
        {
            using DefaultSidModel = Settings::AppSettings::DefaultSidModel;
            const DefaultSidModel sidModelOption = static_cast<DefaultSidModel>(settings.GetOption(Settings::AppSettings::ID::DefaultSidModel)->GetValueAsInt());
            if ((sidModelOption == DefaultSidModel::Prefer_MOS8580) || (sidModelOption == DefaultSidModel::Force_MOS8580))
            {
                defaultSidModel = SidConfig::sid_model_t::MOS8580;
            }

            forceSidModel = (sidModelOption == DefaultSidModel::Force_MOS6581) || (sidModelOption == DefaultSidModel::Force_MOS8580);
        };

        SidConfig::sid_model_t defaultSidModel = SidConfig::sid_model_t::MOS6581;
        bool forceSidModel = false;
    };

    PortAudioOutput::AudioConfig LoadAudioConfig(Settings::AppSettings& settings)
    {
        PortAudioOutput::AudioConfig audioConfig;

        PaDeviceIndex absoluteDeviceIndex = Helpers::Wx::Audio::TryGetAudioDeviceIndex(settings.GetOption(Settings::AppSettings::ID::AudioOutputDevice)->GetValueAsString());
        if (absoluteDeviceIndex == paNoDevice)
        {
            absoluteDeviceIndex = Pa_GetDefaultOutputDevice();
            assert(absoluteDeviceIndex != paNoDevice);
        }

        audioConfig.preferredOutputDevice = absoluteDeviceIndex;
        audioConfig.channelCount = (settings.GetOption(Settings::AppSettings::ID::ForceMono)->GetValueAsBool()) ? 1 : 2;
        audioConfig.sampleRate = Pa_GetDeviceInfo(audioConfig.preferredOutputDevice)->defaultSampleRate;
        audioConfig.lowLatency = settings.GetOption(Settings::AppSettings::ID::LowLatency)->GetValueAsBool();

        return audioConfig;
    }

    SidConfig LoadSidConfig(SidConfig baseSidConfig, Settings::AppSettings& settings)
    {
        const ExtractedSidModelSetting sidModelSetting(settings);
        baseSidConfig.defaultSidModel = sidModelSetting.defaultSidModel;
        baseSidConfig.forceSidModel = sidModelSetting.forceSidModel;

        const ExtractedC64ModelSetting c64ModelSetting(settings);
        baseSidConfig.defaultC64Model = c64ModelSetting.defaultC64Model;
        baseSidConfig.forceC64Model = c64ModelSetting.forceC64Model;

        baseSidConfig.digiBoost = settings.GetOption(Settings::AppSettings::ID::DigiBoost)->GetValueAsBool();

        return baseSidConfig;
    }

    PlaybackController::FilterConfig LoadFilterConfig(Settings::AppSettings& settings)
    {
        return {
            settings.GetOption(Settings::AppSettings::ID::FilterCurve6581)->GetValueAsDouble(),
            settings.GetOption(Settings::AppSettings::ID::FilterCurve8580)->GetValueAsDouble()
        };
    }
}

bool MyApp::OnInit()
{
    _instanceManager = std::make_unique<SingleInstanceManager>();
    const bool weAreFirstInstance = _instanceManager->TryLock();

    wxSetWorkingDirectory(wxPathOnly(wxStandardPaths::Get().GetExecutablePath())); // Prevent the "Open With" changing our CWD.

    currentSettings = std::make_unique<Settings::AppSettings>();
    currentSettings->TryLoad(currentSettings->GetDefaultSettings());

    if (!weAreFirstInstance && currentSettings->GetOption(Settings::AppSettings::ID::SingleInstance)->GetValueAsBool())
    {
        HandoffToCanonicalInstance();
        _earlyExit = true;
        return true;
    }
    else // Normal init
    {
        wxFileSystem::AddHandler(new wxZipFSHandler);
        _playback = std::make_unique<PlaybackController>(); // Must be pre-init here in order for Pa_* methods to be usable immediately.

        const bool initSuccess = _playback->TryInit(PlaybackController::SyncedPlaybackConfig(LoadAudioConfig(*currentSettings),
                                                                                             LoadSidConfig(SidConfig(), *currentSettings),
                                                                                             LoadFilterConfig(*currentSettings)));
        if (initSuccess)
        {
            // Load ROMs
            const wxString& romPathKernal = Helpers::Wx::Files::AsAbsolutePathIfPossible(currentSettings->GetOption(Settings::AppSettings::ID::RomKernalPath)->GetValueAsString().ToStdWstring());
            const wxString& romPathBasic = Helpers::Wx::Files::AsAbsolutePathIfPossible(currentSettings->GetOption(Settings::AppSettings::ID::RomBasicPath)->GetValueAsString().ToStdWstring());
            const wxString& romPathChargen = Helpers::Wx::Files::AsAbsolutePathIfPossible(currentSettings->GetOption(Settings::AppSettings::ID::RomChargenPath)->GetValueAsString().ToStdWstring());
            const RomUtil::RomStatus& romStatus = _playback->TrySetRoms(romPathKernal.ToStdWstring(), romPathBasic.ToStdWstring(), romPathChargen.ToStdWstring());

            if (!romPathKernal.empty() && !romStatus.IsValidated(RomUtil::RomType::Kernal))
            {
                currentSettings->GetOption(Settings::AppSettings::ID::RomKernalPath)->UpdateValue(""); // TODO: remove me after manual clearing is implemented in Prefs!
                WarnRomLoadFailed(romPathKernal, Strings::Error::MSG_ERR_ROM_KERNAL);
            }

            if (!romPathBasic.empty() && !romStatus.IsValidated(RomUtil::RomType::Basic))
            {
                currentSettings->GetOption(Settings::AppSettings::ID::RomBasicPath)->UpdateValue(""); // TODO: remove me after manual clearing is implemented in Prefs!
                WarnRomLoadFailed(romPathBasic, Strings::Error::MSG_ERR_ROM_BASIC);
            }

            if (!romPathChargen.empty() && !romStatus.IsValidated(RomUtil::RomType::Chargen))
            {
                currentSettings->GetOption(Settings::AppSettings::ID::RomChargenPath)->UpdateValue(""); // TODO: remove me after manual clearing is implemented in Prefs!
                WarnRomLoadFailed(romPathChargen, Strings::Error::MSG_ERR_ROM_CHARGEN);
            }

            // Finalize
            SubscribeMe(*_playback, SignalsPlaybackController::SIGNAL_SEEKING_CEASED__WORKER_THREAD_CONTEXT, std::bind(&OnSeekingCeased, this));

            lastFileListReceptionTime = wxGetLocalTimeMillis(); // Must be before FramePlayer init.
            _framePlayer = new FramePlayer(Strings::FramePlayer::WINDOW_TITLE, wxDefaultPosition, wxDefaultSize, *this);
            _framePlayer->SetClientSize(_framePlayer->GetClientSize().GetWidth() + 30, _framePlayer->GetClientSize().GetHeight());
            _framePlayer->Show();

            _instanceManager->RegisterFileListIncomingNotifyCallback([this]()
            {
                _framePlayer->IndicateExternalFilesIncoming();
            });

            _instanceManager->RegisterFileListReceiver([this](wxMilliClock_t fileListReceptionTime, const wxArrayString& rawFiles)
            {
                const bool clearPrevious = fileListReceptionTime - lastFileListReceptionTime > SingleInstanceManager::FILE_LIST_RECEPTION_CONTINUITY_PERIOD;
                const bool autoPlay = currentSettings->GetOption(Settings::AppSettings::ID::AutoPlay) && (clearPrevious || _playback->GetState() == PlaybackController::State::Stopped);

                lastFileListReceptionTime = wxGetLocalTimeMillis();
                _framePlayer->DiscoverFilesAndSendToPlaylist(rawFiles, clearPrevious, autoPlay);
            });
        }
        else
        {
            wxMessageBox(Strings::Error::ERR_INIT_PLAYBACK, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR);
            ResetToDefaultsRecovery(Strings::Error::MSG_ERR_RESET_DEFAULTS_EXIT);
            return false;
        }

        return initSuccess;
    }
}

int MyApp::OnRun()
{
    if (_earlyExit)
    {
        return EXIT_SUCCESS;
    }

    return wxApp::OnRun();
}

void MyApp::HandoffToCanonicalInstance()
{
    wxLogNull shutup; // Popup-errors suppressed until we exit this method.

    if (argc > 1)
    {
        wxString argStr;
        for (const wxString& str : argv.GetArguments())
        {
            argStr.Append(str).Append(IpcSetup::INTERNAL_FILES_SEPARATOR);
        }

        argStr.append(wxGetLocalTimeMillis().ToString());

        const bool success = _instanceManager->SendFilesToCanonicalInstance(argStr);
        if (!success)
        {
            wxMessageBox("This instance failed to handoff files!", wxString::Format("sidplaywx - %lu", wxGetProcessId()));
        }
    }
    else
    {
        const bool success = _instanceManager->BringCanonicalInstanceToForeground();
        if (!success)
        {
            //wxMessageBox("Failed to send to foreground!", wxString::Format("sidplaywx - %lu", wxGetProcessId()));
        }
    }
}

void MyApp::Play(const wxString& filename, unsigned int subsong, int preRenderDurationMs)
{
    assert(_playback != nullptr);

    StopPlayback();

    PlaybackController::PlaybackAttemptStatus status = PlaybackController::PlaybackAttemptStatus::Success;

    {
        std::unique_ptr<BufferHolder> bufferHolder;

        if (Helpers::Wx::Files::IsWithinZipFile(filename))
        {
            bufferHolder = Helpers::Wx::Files::GetFileContentFromZip(filename);
        }
        else
        {
            bufferHolder = Helpers::Wx::Files::GetFileContentFromDisk(filename);
        }

        status = (bufferHolder == nullptr) ? PlaybackController::PlaybackAttemptStatus::InputError : _playback->TryPlayFromBuffer(filename.ToStdWstring(), bufferHolder, subsong, preRenderDurationMs);
    }

    switch (status)
    {
        case PlaybackController::PlaybackAttemptStatus::Success:
            FinalizePlaybackStarted();
            break;

        case PlaybackController::PlaybackAttemptStatus::OutputError:
            wxMessageBox(Strings::Error::MSG_ERR_AUDIO_OUTPUT, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR);
            break;

        case PlaybackController::PlaybackAttemptStatus::InputError:
            [[fallthrough]];
        default:
            wxMessageBox(wxString::Format("%s\n%s", Strings::Error::MSG_ERR_TUNE_FILE, filename), Strings::FramePlayer::WINDOW_TITLE, wxICON_WARNING);
            break;
    }
}

void MyApp::ReplayLoadedTune(int preRenderDurationMs, bool reusePreRender)
{
    StopPlayback();
    const bool success = _playback->TryReplayCurrentSong(preRenderDurationMs, reusePreRender);
    if (success)
    {
        FinalizePlaybackStarted();
    }
}

void MyApp::PausePlayback()
{
    _playback->Pause();
}

void MyApp::ResumePlayback()
{
    _playback->Resume();
}

void MyApp::StopPlayback()
{
    if (_playback != nullptr)
    {
        _playback->Stop();
    }
}

void MyApp::PlaySubsong(int subsong, int preRenderDurationMs)
{
    PopSilencer();
    _playback->TryPlaySubsong(subsong, preRenderDurationMs);
}

void MyApp::SetVolume(float volume)
{
    _playback->SetVolume(volume);
}

void MyApp::SeekTo(uint_least32_t timeMs)
{
    _playback->SeekTo(timeMs);
}

void MyApp::SetPlaybackSpeed(double factor)
{
    _playback->TrySetPlaybackSpeed(factor);
}

void MyApp::ToggleVoice(unsigned int sidNum, unsigned int voice, bool enable)
{
    _playback->ToggleVoice(sidNum, voice, enable);
}

void MyApp::ToggleFilter(unsigned int sidNum, bool enable)
{
    _playback->ToggleFilter(sidNum, enable);
}

const PlaybackController& MyApp::GetPlaybackInfo() const
{
    return *_playback;
}

bool MyApp::ReapplyPlaybackSettings()
{
    const PlaybackController::SwitchAudioDeviceResult result =_playback->TrySwitchPlaybackConfiguration(PlaybackController::SyncedPlaybackConfig(LoadAudioConfig(*currentSettings),
                                                                                                                                                 LoadSidConfig(_playback->GetSidConfig(), *currentSettings),
                                                                                                                                                 LoadFilterConfig(*currentSettings)));
    return result != PlaybackController::SwitchAudioDeviceResult::Failure;
}

void MyApp::UnloadActiveTune()
{
    _playback->UnloadActiveTune();
}

SimpleSignalProvider<SignalsPlaybackController>& MyApp::GetPlaybackSignalProvider()
{
    return *_playback;
}

size_t MyApp::SetVisualizationWaveformWindow(size_t milliseconds)
{
    return _playback->SetVisualizationWaveformWindow(milliseconds);
}

size_t MyApp::GetVisualizationWaveform(short* out) const
{
    return _playback->GetVisualizationWaveform(out);
}

bool MyApp::ResetToDefaultsRecovery(const wxString& message)
{
    const int response = wxMessageBox(message, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR | wxYES_NO);
    if (response == wxYES)
    {
        currentSettings->ResetTo(currentSettings->GetDefaultSettings());
        return true;
    }

    return false;
}

void MyApp::Execute(Subscriber& subscriber, int param)
{
    RunOnMainThread([&subscriber, param]()
    {
        subscriber.callback(param);
    });
}

void MyApp::OnSeekingCeased()
{
    RunOnMainThread([this]()
    {
        if (_playback->GetState() == PlaybackController::State::Playing)
        {
            _playback->Resume();
        }

        EmitSignal(SignalsMyApp::SIGNAL_SEEKING_CEASED); // Broadcast from the main-thread context to subscribers.
    });
}

void MyApp::RunOnMainThread(std::function<void()> fn)
{
    if (wxIsMainThread())
    {
        fn();
    }
    else
    {
        CallAfter(fn);
    }
}

void MyApp::FinalizePlaybackStarted()
{
    PopSilencer();
}

void MyApp::PopSilencer()
{
    const int muteDuration = currentSettings->GetOption(Settings::AppSettings::ID::PopSilencer)->GetValueAsInt();
    const float restoreVolume = _playback->GetVolume();
    if (muteDuration > 0 && restoreVolume > 0.0f)
    {
        _playback->SetVolume(0.0f);
        _popSilencer = std::make_unique<SimpleTimer>(SimpleTimer::Type::OneShot, muteDuration, [this, restoreVolume]()
        {
            _playback->SetVolume(restoreVolume);
        });
    }
}
