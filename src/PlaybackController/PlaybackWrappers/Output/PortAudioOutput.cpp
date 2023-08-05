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

#include "PortAudioOutput.h"
#include <assert.h>
#include <cstdint>
#include <iostream>

static PortAudioOutput::TPortAudioConfig currentAudioConfig; // Must be static because the PlaybackCallback is static (PortAudio works that way).

PortAudioOutput::~PortAudioOutput()
{
    Pa_CloseStream(_stream);
    LogAnyError("~PortAudioOutput -> Pa_Terminate", Pa_Terminate());
    _stream = nullptr;
    _bufferWriter = nullptr;
}

float PortAudioOutput::GetVolume()
{
    return currentAudioConfig.volume;
}

float PortAudioOutput::GetVolumeMultiplier()
{
    return currentAudioConfig.volumeMultiplier;
}

void PortAudioOutput::SetVolume(float volume)
{
    assert(volume >= 0.0f && volume <= 1.0f);
    currentAudioConfig.volume = volume;
}

void PortAudioOutput::SetVolumeMultiplier(float multiplier)
{
    assert(multiplier >= 1.0f);
    currentAudioConfig.volumeMultiplier = multiplier;
}

bool PortAudioOutput::PreInitPortAudioLibrary()
{
    if (_paInitialized)
    {
        throw std::runtime_error("Pa_Initialize already called!");
    }

    bool failed = LogAnyError("TryInit: Pa_Initialize", Pa_Initialize());
    _paInitialized = !failed;
    return _paInitialized;
}

bool PortAudioOutput::TryInit(const AudioConfig& audioConfig, IBufferWriter* bufferWriter)
{
    _bufferWriter = bufferWriter;

    bool success = _paInitialized || PreInitPortAudioLibrary();
    if (success)
    {
        PaDeviceIndex outputDevice = (audioConfig.preferredOutputDevice == paNoDevice) ? Pa_GetDefaultOutputDevice() : audioConfig.preferredOutputDevice;
        if (outputDevice == paNoDevice)
        {
            LogAnyError("TryInit: selected outputDevice", Pa_GetLastHostErrorInfo()->errorCode);
            return false;
        }

        const PaDeviceInfo& deviceInfo = *Pa_GetDeviceInfo(outputDevice);
        currentAudioConfig = AudioConfig(audioConfig);
        currentAudioConfig.hostApiSpecificStreamInfo = NULL; // Without this you get an error in the release mode.
        currentAudioConfig.device = outputDevice;
        currentAudioConfig.sampleFormat = paInt16; // Must be 16 bit (libsidplayfp expects 16 bit buffer).
        currentAudioConfig.suggestedLatency = (currentAudioConfig.lowLatency) ? deviceInfo.defaultLowOutputLatency : deviceInfo.defaultHighOutputLatency;

        // Open an audio I/O stream.
        assert(currentAudioConfig.sampleRate > 8000); // libsidplayfp supports sample rates *above* 8kHz only.
        success = ResetStream(currentAudioConfig.sampleRate) == paNoError;
    }

    return success;
}

bool PortAudioOutput::TryStartStream()
{
    PaError err = Pa_StartStream(_stream);
    return !LogAnyError("TryStartStream", err);
}

void PortAudioOutput::StopStream(bool immediate)
{
    PaError err = (immediate) ? Pa_AbortStream(_stream) : Pa_StopStream(_stream);
    LogAnyError("StopStream", err);
}

PaError PortAudioOutput::ResetStream(double samplerate)
{
    if (_stream != nullptr && Pa_IsStreamActive(_stream))
    {
        PaError err = Pa_CloseStream(_stream);
        if (LogAnyError("ResetStream: Pa_CloseStream", err))
        {
            return err;
        }
    }

    // Open an audio I/O stream.
    PaError err = Pa_OpenStream(&_stream, NULL, &currentAudioConfig, samplerate,
                                paFramesPerBufferUnspecified,
                                paNoFlag,
                                PlaybackCallback,
                                _bufferWriter);

    const bool failed = LogAnyError("ResetStream: Pa_OpenStream", err);
    if (failed)
    {
        _stream = nullptr;
    }

    return err;
}

const PortAudioOutput::AudioConfig& PortAudioOutput::GetAudioConfig() const
{
    return currentAudioConfig;
}

bool PortAudioOutput::IsOutputSampleRateSupported(double samplerate) const
{
    return Pa_IsFormatSupported(NULL, &currentAudioConfig, samplerate) == paFormatIsSupported;
}

bool PortAudioOutput::LogAnyError(const char* tag, const PaError& err)
{
    if (err != paNoError)
    {
        std::cerr << "PortAudio error (" << tag << "): " << Pa_GetErrorText(err) << std::endl;
        return true;
    }

    return false;
}

int PortAudioOutput::PlaybackCallback(const void* /*inputBuffer*/, void* outputBuffer,
                                      unsigned long framesPerBuffer,
                                      const PaStreamCallbackTimeInfo* /*timeInfo*/,
                                      PaStreamCallbackFlags /*statusFlags*/,
                                      void* userData)
{
    IBufferWriter* externalSource = static_cast<IBufferWriter*>(userData);
    bool successful = externalSource->TryFillBuffer(outputBuffer, framesPerBuffer);

    const float volume = currentAudioConfig.volume * currentAudioConfig.volumeMultiplier;
    if (successful && volume != 1.0f) // Apply volume scale.
    {
        short* const out = static_cast<short*>(outputBuffer);
        const uint_least32_t length = framesPerBuffer * currentAudioConfig.channelCount;
        for (uint_least32_t i = 0; i < length; ++i)
        {
            out[i] *= volume;
        }
    }

    return (successful) ? paContinue : paAbort; // Reminder: there is also paComplete, so see about it when we reach the end maybe
}
