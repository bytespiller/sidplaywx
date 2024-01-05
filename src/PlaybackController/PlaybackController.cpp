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

#include "PlaybackController.h"
#include "../Util/HelpersGeneral.h"
#include <sidplayfp/SidTuneInfo.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>

// StateHolder inner class ------------------------------------

PlaybackController::StateHolder::StateHolder(PlaybackController& playbackController) :
    _playbackController(playbackController)
{
}

void PlaybackController::StateHolder::operator=(const PlaybackController::State state)
{
    Set(state);
}

bool PlaybackController::StateHolder::operator==(const PlaybackController::State state) const
{
    return _state == state;
}

bool PlaybackController::StateHolder::operator!=(const PlaybackController::State state) const
{
    return _state != state;
}

void PlaybackController::StateHolder::Set(PlaybackController::State state)
{
    _state = state;
    _playbackController.EmitSignal(SignalsPlaybackController::SIGNAL_PLAYBACK_STATE_CHANGED);
}

PlaybackController::State PlaybackController::StateHolder::Get() const
{
    return _state;
}

// ------------------------------------------------------------

namespace Static
{
    static std::string GetSidName(const SidTuneInfo& tuneInfo, int sidNum)
    {
        switch (tuneInfo.sidModel(sidNum))
        {
            case SidTuneInfo::SIDMODEL_6581:
                return "SID 6581";
            case SidTuneInfo::SIDMODEL_8580:
                return "SID 8580";
            case SidTuneInfo::SIDMODEL_ANY:
                return "SID 6581/8580";
            case SidTuneInfo::SIDMODEL_UNKNOWN:
                // fall-through
            default:
                return "Unknown";
        }
    }
}

// PlaybackController main class ------------------------------

PlaybackController::PlaybackController() :
    _portAudioOutput(std::make_unique<PortAudioOutput>()),
    _state(*this)
{
    const bool success = _portAudioOutput->PreInitPortAudioLibrary();
    if (!success)
    {
        _portAudioOutput = nullptr;
    }
}

PlaybackController::~PlaybackController()
{
    if (_seekOperation.seekThread.joinable())
    {
        _seekOperation.seekThread.join();
    }
}

bool PlaybackController::TryInit(const SyncedPlaybackConfig& config)
{
    if (_state != State::Undefined)
    {
        throw std::runtime_error("TryInit called more than once on the same PlaybackController instance."); // Instead you should destroy the failed instance, and TryInit a new one.
    }

    _sidDecoder = std::make_unique<SidDecoder>();

    const bool sidInitSuccess = TryResetSidDecoder(config);
    const bool audioInitSuccess = TryResetAudioOutput(config.audioConfig, false);

    const bool success = sidInitSuccess && audioInitSuccess;
    if (!success)
    {
        return false; // Failed to initialize PlaybackController. Check cerr/logs for more info.
    }

    _state = State::Stopped;

    return true;
}

PlaybackController::SwitchAudioDeviceResult PlaybackController::TrySwitchPlaybackConfiguration(const SyncedPlaybackConfig& newConfig)
{
    const bool needResetSidDecoder = (newConfig.sidConfig.frequency != _sidDecoder->GetSidConfig().frequency) ||
                                     (newConfig.sidConfig.playback != _sidDecoder->GetSidConfig().playback) ||
                                     (newConfig.sidConfig.defaultC64Model != _sidDecoder->GetSidConfig().defaultC64Model) ||
                                     (newConfig.sidConfig.defaultSidModel != _sidDecoder->GetSidConfig().defaultSidModel) ||
                                     (newConfig.sidConfig.forceC64Model != _sidDecoder->GetSidConfig().forceC64Model) ||
                                     (newConfig.sidConfig.forceSidModel != _sidDecoder->GetSidConfig().forceSidModel) ||
                                     (newConfig.sidConfig.digiBoost != _sidDecoder->GetSidConfig().digiBoost) ||
                                     (newConfig.filterConfig.filterEnabled != _sidDecoder->GetFilterConfig().filterEnabled) ||
                                     (!Helpers::General::AreFloatsEqual(newConfig.filterConfig.filter6581Curve, _sidDecoder->GetFilterConfig().filter6581Curve)) ||
                                     (!Helpers::General::AreFloatsEqual(newConfig.filterConfig.filter8580Curve, _sidDecoder->GetFilterConfig().filter8580Curve));

    const bool needResetAudioOutput = (newConfig.audioConfig.lowLatency != _portAudioOutput->GetAudioConfig().lowLatency) ||
                                      (newConfig.audioConfig.channelCount != _portAudioOutput->GetAudioConfig().channelCount) ||
                                      (newConfig.audioConfig.preferredOutputDevice != _portAudioOutput->GetAudioConfig().preferredOutputDevice) ||
                                      (newConfig.audioConfig.sampleRate != _portAudioOutput->GetAudioConfig().sampleRate);

    SwitchAudioDeviceResult result = SwitchAudioDeviceResult::OnTheFly;
    bool success = true;

    const bool enablePreRender = _preRender != nullptr;

    if (needResetSidDecoder)
    {
        Stop();
        _preRender = nullptr; // SID params changed, any pre-rendered content is no longer valid.

        success = TryResetSidDecoder(newConfig);
        result = (success) ? SwitchAudioDeviceResult::Stopped : SwitchAudioDeviceResult::Failure;
    }

    if (success && needResetAudioOutput)
    {
        Stop();
        success = TryResetAudioOutput(newConfig.audioConfig, enablePreRender);
        result = (success) ? result : SwitchAudioDeviceResult::Failure;

        TrySetPlaybackSpeed(_playbackSpeedFactor);
    }

    EmitSignal(SignalsPlaybackController::SIGNAL_AUDIO_DEVICE_CHANGED, static_cast<int>(success));

    return result;
}

RomUtil::RomStatus PlaybackController::TrySetRoms(const std::wstring& pathKernal, const std::wstring& pathBasic, const std::wstring& pathChargen)
{
    const RomUtil::RomStatus& preCheckStatus = RomUtil::PreCheckRoms(pathKernal, pathBasic, pathChargen);
    if (!preCheckStatus.AreAllValidated())
    {
        return preCheckStatus;
    }

    _loadedRoms = _sidDecoder->TrySetRoms(pathKernal, pathBasic, pathChargen);
    return _loadedRoms;
}

bool PlaybackController::TryPlayFromBuffer(const std::wstring& filepathForUid, std::unique_ptr<BufferHolder>& loadedBufferToAdopt, unsigned int subsong, int preRenderDurationMs)
{
    PrepareTryPlay();
    _activeTuneHolder = std::make_unique<TuneHolder>(filepathForUid, loadedBufferToAdopt);
    const bool success = _sidDecoder->TryLoadSong(_activeTuneHolder->bufferHolder->buffer, _activeTuneHolder->bufferHolder->size, subsong);
    return FinalizeTryPlay(success, preRenderDurationMs);
}

bool PlaybackController::TryReplayCurrentSong(int preRenderDurationMs)
{
    return TryPlaySubsong(GetCurrentSubsong(), preRenderDurationMs);
}

bool PlaybackController::TryPlaySubsong(unsigned int subsong, int preRenderDurationMs)
{
    if (_state != State::Undefined && _activeTuneHolder != nullptr)
    {
        return TryReplayCurrentSongFromBuffer(subsong, preRenderDurationMs);
    }

    return false;
}

void PlaybackController::Pause()
{
    if (_state == State::Playing)
    {
        _portAudioOutput->StopStream(false); // Reminder: never put true, you'll have random problems.
        _state = State::Paused;
    }
    else if (_state == State::Seeking)
    {
        _seekOperation.resumeToState = State::Paused;
    }
    else
    {
        Warn("Pause called while not playing or seeking, ignored.");
    }
}

void PlaybackController::Resume()
{
    if (_state == State::Paused)
    {
        _portAudioOutput->TryStartStream();
        _state = State::Playing;
    }
    else if (_state == State::Seeking)
    {
        _seekOperation.resumeToState = State::Playing;
    }
    else
    {
        Warn("Resume called while not paused or seeking, ignored.");
    }
}

void PlaybackController::Stop()
{
    if (_state == State::Undefined)
    {
        DebugInfo("Stop called before init, ignored.");
        return;
    }

    if (_state == State::Seeking)
    {
        AbortSeek(false);
    }

    if (_state == State::Stopped)
    {
        DebugInfo("Stop called while already stopped, ignored.");
    }
    else
    {
        if (_state == State::Playing)
        {
            _portAudioOutput->StopStream(false); // Reminder: never put true, you'll have random problems.
        }

        if (_state != State::Stopped)
        {
            if (_preRender != nullptr)
            {
                _preRender->Stop();
            }

            _sidDecoder->Stop();
        }

        _state = State::Stopped;
    }
}

void PlaybackController::SeekTo(uint_least32_t targetTimeMs)
{
    if (_state == State::Stopped || _state == State::Undefined)
    {
        throw std::runtime_error("SeekTo: not possible from current state!");
    }
    else if (_state == State::Seeking)
    {
        PlaybackController::State restoreState = _seekOperation.resumeToState;
        AbortSeek(false); // This now becomes a synchronous call and the OnSeekStatusReceived() will be called before the next line.
        _state = restoreState; // Replace OnSeekStatusReceived()'s "Paused" change -- due to AbortSeek(false) -- with previous state.
    }
    else if (_state == State::Playing)
    {
        _portAudioOutput->StopStream(false); // Reminder: never put true, you'll have random problems.
    }

    // Clean up
    if (_seekOperation.seekThread.joinable()) // Clean up old expired thread.
    {
        _seekOperation.seekThread.join();
    }
    _seekOperation.abortFlag = false;
    _seekOperation.resumeToState = _state.Get();
    _seekOperation.safeCtimeMs = 0;

    // Start seeking in a new thread
    _state = State::Seeking;
    _seekOperation.seekThread = std::thread([this, targetTimeMs]
    {
        if (_preRender != nullptr) // Instant seeking mode
        {
            return _preRender->SeekTo(targetTimeMs, [this](uint_least32_t cTimeMs, bool done) -> bool
            {
                return OnSeekStatusReceived(cTimeMs, done);
            });
        }
        else // Regular mode
        {
            _sidDecoder->SeekTo(targetTimeMs, [this](uint_least32_t cTimeMs, bool done) -> bool
            {
                return OnSeekStatusReceived(cTimeMs, done);
            });
        }
    });
}

void PlaybackController::AbortSeek(bool resumePlaybackState)
{
    if (_state == State::Seeking)
    {
        if (!resumePlaybackState)
        {
            _seekOperation.resumeToState = State::Paused;
        }

        _seekOperation.abortFlag = true;
        _seekOperation.seekThread.join();
    }
    else
    {
        Warn("AbortSeek called while not seeking, ignored.");
    }
}

PlaybackController::State PlaybackController::GetState() const
{
    return _state.Get();
}

PlaybackController::State PlaybackController::GetResumeState() const
{
    return _seekOperation.resumeToState;
}

uint_least32_t PlaybackController::GetTime() const
{
    if (_state == State::Seeking)
    {
        return _seekOperation.safeCtimeMs;
    }
    else
    {
        if (_preRender != nullptr)
        {
            return _preRender->GetCurrentSongTimeMs();
        }

        return _sidDecoder->GetTime();
    }
}

bool PlaybackController::TrySetPlaybackSpeed(double factor)
{
    const double deviceSampleRate = _portAudioOutput->GetAudioConfig().sampleRate;
    const double desiredStreamSampleRate = deviceSampleRate * factor;

    const bool supported = _portAudioOutput->IsOutputSampleRateSupported(desiredStreamSampleRate);
    _playbackSpeedFactor = (supported) ? factor : 1.0;

    const double applyStreamSampleRate = (supported) ? desiredStreamSampleRate : deviceSampleRate;

    _portAudioOutput->ResetStream(applyStreamSampleRate);
    if (_state == State::Playing)
    {
        _portAudioOutput->TryStartStream();
    }

    EmitSignal(SignalsPlaybackController::SIGNAL_PLAYBACK_SPEED_CHANGED);

    return supported;
}

double PlaybackController::GetPlaybackSpeedFactor() const
{
    return _playbackSpeedFactor;
}

int PlaybackController::GetCurrentSubsong() const
{
    return _sidDecoder->GetCurrentSubsong();
}

int PlaybackController::GetDefaultSubsong() // Reminder: not const on purpose (to prevent misusing it in PlaybackInfo instead of getting a proper info via Playlist)!
{
    return _sidDecoder->GetDefaultSubsong();
}

int PlaybackController::GetTotalSubsongs() const
{
    return _sidDecoder->GetTotalSubsongs();
}

std::string PlaybackController::GetCurrentTuneInfoString(SongInfoCategory category) const
{
    return _sidDecoder->GetCurrentTuneInfoString(category);
}

std::wstring PlaybackController::GetCurrentTuneFilePath() const
{
    return (_activeTuneHolder != nullptr) ? _activeTuneHolder->filepath : L"";
}

int PlaybackController::GetCurrentTuneSidChipsRequired() const
{
    return _sidDecoder->GetCurrentTuneSidChipsRequired();
}

std::string PlaybackController::GetCurrentTuneSpeedDescription() const
{
    return _sidDecoder->GetEngineInfo().speedString();
}

SidConfig::sid_model_t PlaybackController::GetCurrentlyEffectiveSidModel() const
{
    const SidConfig& cSidConfig = _sidDecoder->GetSidConfig();
    SidConfig::sid_model_t effectiveSidChipModel = cSidConfig.defaultSidModel;

    if (IsValidSongLoaded() && !cSidConfig.forceSidModel)
    {
        const SidTuneInfo& tuneInfo = _sidDecoder->GetCurrentSongInfo();
        switch (tuneInfo.sidModel(0))
        {
            case SidTuneInfo::model_t::SIDMODEL_6581:
                effectiveSidChipModel = SidConfig::sid_model_t::MOS6581;
                break;
            case SidTuneInfo::model_t::SIDMODEL_8580:
                effectiveSidChipModel = SidConfig::sid_model_t::MOS8580;
                break;
            case SidTuneInfo::model_t::SIDMODEL_ANY:
                // fall-through
            case SidTuneInfo::model_t::SIDMODEL_UNKNOWN:
                // leave existing value (effectiveSidChipModel)
                break;
            default:
                throw std::runtime_error("Unhandled switch case!");
        }
    }

    return effectiveSidChipModel;
}

std::string PlaybackController::GetCurrentTuneSidDescription() const
{
    const SidTuneInfo& tuneInfo = _sidDecoder->GetCurrentSongInfo();
    std::string retString;

    { // Indicate SID model used by the tune (single or multiple)
        const int sidsNeeded = tuneInfo.sidChips();

        std::set<std::string> uniqueSidNames;
        for (int i = 0; i < sidsNeeded; ++i)
        {
            uniqueSidNames.emplace(Static::GetSidName(tuneInfo, i));
        }

        const bool multipleOfTheSame = uniqueSidNames.size() == 1 && sidsNeeded > 1;
        const std::string prefix = (multipleOfTheSame) ? std::to_string(sidsNeeded) + "x " : "";

        if (multipleOfTheSame)
        {
            retString = prefix + *uniqueSidNames.begin();
        }
        else
        {
            bool needsComma = false;
            for (const std::string& sidName : uniqueSidNames)
            {
                if (needsComma)
                {
                    retString += ", ";
                }

                needsComma = true;
                retString += sidName;
            }
        }
    }

    {
        // Determine effective SID model
        const SidConfig& cSidConfig = _sidDecoder->GetSidConfig();
        const SidTuneInfo::model_t tuneSidModel = tuneInfo.sidModel(0);
        SidConfig::sid_model_t effectiveSidChipModel = GetCurrentlyEffectiveSidModel();

        // Indicate effective SID model (if differs from tune specification)
        const bool anySidModel = tuneSidModel == SidTuneInfo::model_t::SIDMODEL_ANY || tuneSidModel == SidTuneInfo::model_t::SIDMODEL_UNKNOWN;
        if ((anySidModel || tuneSidModel == SidTuneInfo::model_t::SIDMODEL_6581) && effectiveSidChipModel == SidConfig::sid_model_t::MOS8580)
        {
            retString += " [8580]";
        }
        else if ((anySidModel || tuneSidModel == SidTuneInfo::model_t::SIDMODEL_8580) && effectiveSidChipModel == SidConfig::sid_model_t::MOS6581)
        {
            retString += " [6581]";
        }

        // Indicate DigiBoost
        const bool digiBoostEffective = cSidConfig.digiBoost && effectiveSidChipModel == SidConfig::sid_model_t::MOS8580;
        if (digiBoostEffective)
        {
            retString += " [DigiBoost]";
        }
    }

    // Indicate disabled filter
    if (!_sidDecoder->GetFilterConfig().filterEnabled)
    {
        retString += " [Filter Off]";
    }

    return retString;
}

std::string PlaybackController::GetCurrentTuneTypeDescription() const
{
    switch (_sidDecoder->GetCurrentSongInfo().compatibility())
    {
        case SidTuneInfo::COMPATIBILITY_BASIC:
            return "BASIC";
        case SidTuneInfo::COMPATIBILITY_C64:
            return "";
        case SidTuneInfo::COMPATIBILITY_PSID:
            return "PSID";
        case SidTuneInfo::COMPATIBILITY_R64:
            return "R64";
    }

    return "";
}

bool PlaybackController::IsValidSongLoaded() const
{
    return _activeTuneHolder != nullptr;
}

SidDecoder::RomRequirement PlaybackController::GetCurrentSongRomRequirement() const
{
    return _sidDecoder->GetCurrentSongRomRequirement();
}

bool PlaybackController::IsRomLoaded(SidDecoder::RomRequirement requirement) const
{
    switch (requirement)
    {
        case SidDecoder::RomRequirement::R64:
            return _loadedRoms.IsValidated(RomUtil::RomType::Kernal);
        case SidDecoder::RomRequirement::BasicRom:
            return _loadedRoms.IsValidated(RomUtil::RomType::Basic);
        case SidDecoder::RomRequirement::None:
            return true;
        default:
            throw std::runtime_error("Unhandled switch case!");
    }
}

SidConfig PlaybackController::GetSidConfig() const
{
    return _sidDecoder->GetSidConfig();
}

PortAudioOutput::AudioConfig PlaybackController::GetAudioConfig() const
{
    return _portAudioOutput->GetAudioConfig();
}

int PlaybackController::GetCurrentTuneSize(bool bulkSize) const
{
    const SidTuneInfo& tuneInfo = _sidDecoder->GetCurrentSongInfo();
    return (bulkSize) ? tuneInfo.dataFileLen() : tuneInfo.c64dataLen();
}

float PlaybackController::GetVolume() const
{
    return _portAudioOutput->GetVolume();
}

void PlaybackController::SetVolume(float volume)
{
    _portAudioOutput->SetVolume(volume);
}

bool PlaybackController::ToggleVoice(unsigned int sidNum, unsigned int voice, bool enable)
{
    if (IsValidSongLoaded())
    {
        if (sidNum + 1 <= GetCurrentTuneSidChipsRequired())
        {
            _sidDecoder->ToggleVoice(sidNum, voice, enable);
            EmitSignal(SignalsPlaybackController::SIGNAL_VOICE_TOGGLED);
            return true;
        }
    }

    return false;
}

bool PlaybackController::IsVoiceEnabled(unsigned int sidNum, unsigned int voice) const
{
    return _sidDecoder->GetSidVoicesEnabledStatus().at(sidNum).at(voice);
}

bool PlaybackController::AreAllRelevantVoicesEnabled() const
{
    const SidDecoder::SidVoicesEnabledStatus& allSidVoices = _sidDecoder->GetSidVoicesEnabledStatus();
    const int sidsNeeded = _sidDecoder->GetCurrentTuneSidChipsRequired();
    return std::all_of(allSidVoices.begin(), allSidVoices.begin() + sidsNeeded, [](const std::vector<bool>& cSidVoices)
    {
        return std::all_of(cSidVoices.begin(), cSidVoices.end(), [](const bool enabled){return enabled;});
    });
}

void PlaybackController::UnloadActiveTune()
{
    Stop();
    _sidDecoder->UnloadActiveTune();
    _activeTuneHolder = nullptr;
}

bool PlaybackController::TryResetSidDecoder(const SyncedPlaybackConfig& newConfig)
{
    const int subsong = (IsValidSongLoaded()) ? GetCurrentSubsong() : 1;

    if (_state != State::Undefined)
    {
        Stop();
    }

    _preRender = nullptr; // Some SID params changed, any pre-rendered content is no longer valid.

    const bool success = _sidDecoder->TryInitEmulation(newConfig.sidConfig, newConfig.filterConfig);
    if (success)
    {
        if (_state != State::Undefined && _activeTuneHolder != nullptr)
        {
            PrepareTryPlay(); // Stop everything (seeking, playback, whathaveyou).
            _sidDecoder->TrySetSubsong(subsong); // Sync _activeTuneHolder with the (now reset) SID engine.
        }
    }
    else
    {
        _activeTuneHolder = nullptr;
    }

    return success;
}

bool PlaybackController::TryResetAudioOutput(const PortAudioOutput::AudioConfig& audioConfig, bool enablePreRender)
{
    if (_sidDecoder == nullptr)
    {
        throw std::runtime_error("SID decoder must be initialized first!");
    }

    if (_sidDecoder->GetSidConfig().frequency != static_cast<uint_least32_t>(audioConfig.sampleRate))
    {
        throw std::runtime_error("Sample rates not in sync between SID decoder and Audio Output!");
    }

    _preRender = (enablePreRender) ? std::make_unique<PreRender>() : nullptr; // Enable the pre-render output if desired, otherwise destroy the old instance.

    IBufferWriter* decoder = (_preRender == nullptr) ? _sidDecoder.get() : static_cast<IBufferWriter*>(_preRender.get()); // Use either the pre-render or the realtime audio output.
    return _portAudioOutput != nullptr && _portAudioOutput->TryInit(audioConfig, decoder);
}

void PlaybackController::PrepareTryPlay()
{
    if (_state == State::Seeking)
    {
        AbortSeek(false);
    }

    if (_state == State::Playing)
    {
        _portAudioOutput->StopStream(false); // Reminder: never put true, you'll have random problems.
    }

    if (_state != State::Stopped)
    {
        if (_preRender != nullptr)
        {
            _preRender->Stop();
        }

        _sidDecoder->Stop();
    }
}

bool PlaybackController::FinalizeTryPlay(bool isSuccessful, int preRenderDurationMs)
{
    if (isSuccessful)
    {
        if (preRenderDurationMs > 0)
        {
            if (_preRender == nullptr)
            {
                TryResetAudioOutput(GetAudioConfig(), true);
            }

            const SidConfig& sidConfig = _sidDecoder->GetSidConfig();
            _preRender->DoPreRender(*_sidDecoder.get(), sidConfig.frequency, sidConfig.playback, preRenderDurationMs);
        }
        else
        {
            if (_preRender != nullptr)
            {
                TryResetAudioOutput(GetAudioConfig(), false); // Destroy _preRender
            }
        }

        isSuccessful = _portAudioOutput->TryStartStream();
        if (!isSuccessful)
        {
            if (_preRender != nullptr)
            {
                _preRender->Stop();
            }

            _sidDecoder->Stop();
            _activeTuneHolder = nullptr;
        }
    }

    _state = (isSuccessful) ? State::Playing : State::Stopped;
    return isSuccessful;
}

bool PlaybackController::TryReplayCurrentSongFromBuffer(unsigned int subsong, int preRenderDurationMs)
{
    PrepareTryPlay();
    const bool success = _sidDecoder->TrySetSubsong(subsong);
    return FinalizeTryPlay(success, preRenderDurationMs);
}

bool PlaybackController::OnSeekStatusReceived(uint_least32_t cTimeMs, bool done)
{
    _seekOperation.safeCtimeMs = cTimeMs;

    if (_seekOperation.abortFlag || done)
    {
        _state = _seekOperation.resumeToState;
        if (_state == State::Undefined)
        {
            throw std::runtime_error("OnSeekStatusReceived return state is Undefined, something got messed up quite a bit!"); // https://xkcd.com/2200/
        }

        _seekOperation.abortFlag = false;
        _seekOperation.resumeToState = State::Undefined;
        _seekOperation.safeCtimeMs = 0;

        // Reminder: can't resume playback (start PA stream) *here* because we're not in a main thread and we could get PA error -9999 for WASAPI audio devices (unless the app is UAC elevated).

        EmitSignal(SignalsPlaybackController::SIGNAL_SEEKING_CEASED__WORKER_THREAD_CONTEXT);

        return SeekProcessStatus::Abort;
    }

    return SeekProcessStatus::Resume;
}

void PlaybackController::Warn(const char* message)
{
    std::cerr << "[Warning] PlaybackController: " << message << std::endl;
}

void PlaybackController::DebugInfo(const char* message)
{
    std::cout << "[Info] PlaybackController: " << message << std::endl;
}
