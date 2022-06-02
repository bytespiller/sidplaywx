/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2022 Jasmin Rutic (bytespiller@gmail.com)
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
    typedef struct TPortAudioConfig
    {
        int outputChannels = 0;
        double sampleRate = 0.0; // Reminder: libsidplayfp supports only *above* 8kHz (limit it in GUI).
        PaDeviceIndex preferredOutputDevice = paNoDevice;
        bool lowLatency = false;
    } AudioConfig;

public:
    ~PortAudioOutput();

public:
    static float GetVolume();
    static float GetVolumeMultiplier();
    static void SetVolume(float volume);
    static void SetVolumeMultiplier(float multiplier);

public:
    bool PreInitPortAudioLibrary();
    bool TryInit(const AudioConfig& audioConfig, IBufferWriter* bufferWriter);
    bool TryStartStream();
    void StopStream(bool immediate);
    PaError ResetStream(double samplerate);

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
    PaStreamParameters _outputParameters;
    PaStream* _stream = nullptr;
    IBufferWriter* _bufferWriter = nullptr;
    bool _paInitialized = false;
    AudioConfig _currentAudioConfig;
};
