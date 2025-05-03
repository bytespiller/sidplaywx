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

#pragma once

#include "PreRender.h"
#include "PlaybackWrappers/Output/PortAudioOutput.h"
#include "PlaybackWrappers/Input/SidDecoder.h"
#include "Util/RomUtil.h"
#include "../Util/BufferHolder.h"
#include "../Util/SimpleSignal/SimpleSignalProvider.h"

#include <atomic>
#include <memory>
#include <thread>

enum class SignalsPlaybackController
{
    SIGNAL_SEEKING_CEASED__WORKER_THREAD_CONTEXT,
    SIGNAL_PLAYBACK_SPEED_CHANGED,
    SIGNAL_VOICE_TOGGLED,
    SIGNAL_AUDIO_DEVICE_CHANGED,
    SIGNAL_PLAYBACK_STATE_CHANGED,
};

class PlaybackController : public SimpleSignalProvider<SignalsPlaybackController>
{
public:
    enum class State
    {
        Undefined,
        Stopped,
        Playing,
        Paused,
        Seeking
    };

    enum class SwitchAudioDeviceResult
    {
        OnTheFly,
        Stopped,
        Failure
    };

    using FilterConfig = SidDecoder::FilterConfig;
    using SongInfoCategory = SidDecoder::SongInfoCategory;

    struct SyncedPlaybackConfig
    {
        SyncedPlaybackConfig() = delete;

        SyncedPlaybackConfig(const PortAudioOutput::AudioConfig& aAudioConfig, const SidConfig& aSidConfig, const FilterConfig& aFilterConfig) :
            audioConfig(aAudioConfig),
            sidConfig(aSidConfig),
            filterConfig(aFilterConfig)
        {
            sidConfig.frequency = static_cast<uint_least32_t>(audioConfig.sampleRate);
            sidConfig.playback = (audioConfig.channelCount == 1) ? SidConfig::playback_t::MONO : SidConfig::playback_t::STEREO;
        }

        PortAudioOutput::AudioConfig audioConfig;
        SidConfig sidConfig;
        FilterConfig filterConfig;
    };

private:
    class StateHolder
    {
    public:
        StateHolder() = delete;
        explicit StateHolder(PlaybackController& playbackController);

        void operator=(const PlaybackController::State state);
        bool operator==(const PlaybackController::State state) const;
        bool operator!=(const PlaybackController::State state) const;

    public:
        PlaybackController::State Get() const;
        void Set(PlaybackController::State state);

    private:
        PlaybackController& _playbackController;
        PlaybackController::State _state{};
    };

    struct SeekOperation
    {
        std::atomic_bool abortFlag {false};
        std::atomic_uint_least32_t safeCtimeMs {0};
        std::atomic_uint_least32_t safeTargetTimeMs {0}; // Informative
        PlaybackController::State resumeToState{};
        std::thread seekThread;
    };

    struct TuneHolder
    {
        TuneHolder() = delete;
        TuneHolder(const std::filesystem::path& filepathForUid, std::unique_ptr<BufferHolder>& loadedBufferToAdopt) :
            filepath(filepathForUid),
            bufferHolder(std::move(loadedBufferToAdopt))
        {
        }

        const std::filesystem::path filepath;
        const std::unique_ptr<const BufferHolder> bufferHolder;
    };

public:
    PlaybackController();
    PlaybackController(PlaybackController&) = delete;

    ~PlaybackController();

public:
    bool TryInit(const SyncedPlaybackConfig& config);
    SwitchAudioDeviceResult TrySwitchPlaybackConfiguration(const SyncedPlaybackConfig& newConfig);

    // Paths should be absolute.
    RomUtil::RomStatus TrySetRoms(const std::filesystem::path& pathKernal, const std::filesystem::path& pathBasic, const std::filesystem::path& pathChargen);

public:
    bool TryPlayFromBuffer(const std::filesystem::path& filepathForUid, std::unique_ptr<BufferHolder>& loadedBufferToAdopt, unsigned int subsong, int preRenderDurationMs);
    bool TryReplayCurrentSong(int preRenderDurationMs, bool reusePreRender = false);
    bool TryPlaySubsong(unsigned int subsong, int preRenderDurationMs, bool reusePreRender = false);

    void Pause();
    void Resume();
    void Stop();

    void SeekTo(uint_least32_t targetTimeMs);
    void AbortSeek(bool resumePlaybackState = true);

    // Gets the target time milliseconds parameter of the last SeekTo() call or zero.
    uint_least32_t GetLastSeekTargetMs() const;

    State GetState() const;
    State GetResumeState() const;

    /// @brief Gets the current playback time position. Playback buffer size affects the returned value and is typically some milliseconds in the future.
    uint_least32_t GetTime() const;
    double GetPreRenderProgressFactor() const;

    bool TrySetPlaybackSpeed(double factor);
    double GetPlaybackSpeedFactor() const;

    int GetCurrentSubsong() const;
    int GetDefaultSubsong();
    int GetTotalSubsongs() const;

    std::string GetCurrentTuneInfoString(SongInfoCategory category) const;
    std::string GetCurrentTuneMusComments() const;

    std::string GetCurrentTuneSpeedDescription() const;
    std::string GetCurrentTuneSidDescription(bool includeEffective = true) const;
    std::string GetCurrentTuneTypeDescription() const;
    std::filesystem::path GetCurrentTuneFilePath() const;
    const SidTuneInfo& GetCurrentTuneSidInfo() const;
    int GetCurrentTuneSidChipsRequired() const;
    bool IsValidSongLoaded() const;

    SidConfig::sid_model_t GetCurrentlyEffectiveSidModel() const;

    SidDecoder::RomRequirement GetCurrentSongRomRequirement() const;
    bool IsRomLoaded(SidDecoder::RomRequirement requirement) const;

    SidConfig GetSidConfig() const;
    PortAudioOutput::AudioConfig GetAudioConfig() const;

    // Returns Length of raw C64 data without load address. If bulkSize is true, Returns Length of single-file sidtune file.
    int GetCurrentTuneSize(bool bulkSize = false) const;

    float GetVolume() const;
    void SetVolume(float volume);

    bool ToggleVoice(unsigned int sidNum, unsigned int voice, bool enable);
    bool ToggleFilter(unsigned int sidNum, bool enable);

    bool IsVoiceEnabled(unsigned int sidNum, unsigned int voice) const;
    bool IsFilterEnabled(unsigned int sidNum) const;

    /// @brief Checks for voices + digi and filter.
    bool AreRelevantSidsFullyEnabled() const;

    void UnloadActiveTune();

    /// @brief Defines visualization (double) buffer length. Pass 0 to disable and free some resources. Returns size of buffer (calculated from milliseconds and the currently effective sample rate).
    size_t SetVisualizationWaveformWindow(size_t milliseconds);

    /// @brief Copies the latest available waveform data (playback buffer size affects latency) into a target buffer. Returns size of data if successful or 0 if not ready or disabled.
    size_t GetVisualizationWaveform(short* out) const;

private:
    bool TryResetSidDecoder(const SyncedPlaybackConfig& newConfig);
    bool TryResetAudioOutput(const PortAudioOutput::AudioConfig& audioConfig, bool enablePreRender);

    void PrepareTryPlay();
    bool FinalizeTryPlay(bool isSuccessful, int preRenderDurationMs, bool reusePreRender = false);
    bool TryReplayCurrentSongFromBuffer(unsigned int subsong, int preRenderDurationMs, bool reusePreRender = false);

    bool OnSeekStatusReceived(uint_least32_t cTimeMs, bool done);

private:
    static void Warn(const char* message);
    static void DebugInfo(const char* message);

private:
    std::unique_ptr<TuneHolder> _activeTuneHolder;
    std::unique_ptr<SidDecoder> _sidDecoder;
    std::unique_ptr<PortAudioOutput> _portAudioOutput;
    std::unique_ptr<PreRender> _preRender;

    StateHolder _state;
    SeekOperation _seekOperation{};

    double _playbackSpeedFactor = 1.0;

    RomUtil::RomStatus _loadedRoms{};

private:
    struct SeekProcessStatus
    {
        static constexpr bool Abort = true;
        static constexpr bool Resume = false;
    };
};
