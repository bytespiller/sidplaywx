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

#pragma once

#include "../IBufferWriter.h"
#include <portaudio.h>

class PortAudioOutput
{
public:
    struct AudioConfig: public PaStreamParameters
    {
        float volume = 1.0f;
        double sampleRate = 0.0;
        bool lowLatency = false;
        PaDeviceIndex preferredOutputDevice = paNoDevice;
    };

    struct FxConfig
    {
        unsigned int virtualStereoExpansionOffsetMs = 0;
        float virtualStereoSideVolumeFactor = 0;
    };

public:
    ~PortAudioOutput();

public:
    float GetVolume();
    void SetVolume(float volume);

    /** @brief
     * Pass length 0 to disable.
     * Ideally, length shouldn't be below the playback buffer size to avoid inefficiency (if the playback routine writes to this buffer in a fixed-size chunk, a too-small buffer would only retain the latest data it can fit so any write operations before that would be wasted).
     */
    void InitVisualizationBuffer(size_t length);

    /// @brief Copies the latest waveform data (playback buffer size affects latency). Returns size of data (can be 0 if not ready or disabled).
    size_t GetVisualizationWaveform(short* out) const;

public:
    bool PreInitPortAudioLibrary();
    bool TryInit(const AudioConfig& audioConfig, IBufferWriter* bufferWriter);
    bool TryStartStream();
    void StopStream(bool immediate);
    PaError ResetStream(double samplerate);

    /// @brief Pass zero offsetMs to disable.
    void SetVirtualStereo(unsigned int offsetMs, float sideVolumeFactor);

    const AudioConfig& GetAudioConfig() const;
    bool IsOutputSampleRateSupported(double samplerate) const;

private:
    static bool LogAnyError(const char* tag, const PaError& err);

private:
    static int PlaybackCallback(const void* inputBuffer, void* outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void* userData);

private:
    PaStream* _stream = nullptr;
    IBufferWriter* _bufferWriter = nullptr;
    bool _paInitialized = false;
    FxConfig _fxConfig;
};
