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

#include "PortAudioOutput.h"
#include <assert.h>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <string.h> // memcpy (Linux)
#include <memory>

static constexpr double LIBSIDPLAYFP_MIN_BUFFER_LATENCY = 5.6 / 1000.0; // Ensure safe minimum buffer size due to libsidplayfp change in commit 1a6d9016e8bc35fa88d429c1ca77f31c5f5f6831 causing crash with ALSA & PulseAudio when using the paFramesPerBufferUnspecified (auto-size).

class VisualizationBuffer
{
public:
	VisualizationBuffer() = delete;
    VisualizationBuffer(VisualizationBuffer&) = delete;
	VisualizationBuffer& operator=(const VisualizationBuffer&) = delete;

    explicit VisualizationBuffer(size_t aLength) :
        maxLength(aLength)
    {
        _first = new std::atomic_short[maxLength];
        _second = new std::atomic_short[maxLength];
    }

    ~VisualizationBuffer()
    {
        delete[] _first;
        _first = nullptr;

        delete[] _second;
        _second = nullptr;
    }

public:
    /// @brief Copies front-buffer data with an initialized constant maxLength size and returns maxLength. If no initial data is ready yet, doesn't copy anything and returns 0.
    size_t Read(short* out) const
    {
        if (!_ready)
        {
            return 0;
        }

        const std::atomic_short* const buffer = (_flipped) ? _second : _first;
        memcpy(out, buffer, maxLength * sizeof(short));
        return maxLength;
    }

    void Write(const short* const data, size_t dataLength)
    {
        const size_t remaining = maxLength - _level;
        const size_t overflow = (dataLength > remaining) ? (dataLength - remaining) : 0;

        // Fill the active back-buffer up to its maximum
        const size_t amount = (overflow == 0) ? dataLength : remaining;
        std::atomic_short* buffer = (_flipped) ? _first : _second;
        for (int i = 0; i < amount; ++i)
        {
            buffer[_level + i] = data[i];
        }

        _level += amount;

        // Flip the front/back buffer roles if the current back-buffer is maxed out
        if (_level >= maxLength)
        {
            _flipped = !_flipped;
            _level = 0;
            _ready = true;

            // Keep overwriting any previous data until we're left with only the latest data portion stored across both buffers
            if (overflow != 0)
            {
                Write(&data[amount], dataLength - amount);
            }
        }
    }

public:
    const size_t maxLength;

private:
    std::atomic_bool _ready = false; // Whether the front-buffer is ready (remains true permanently).
    std::atomic_bool _flipped = false; // Indicates a front-buffer and back-buffer role inversion.
    std::atomic_size_t _level = 0; // Fill-level of the active back-buffer.

    std::atomic_short* _first; // Front/back buffer #1.
    std::atomic_short* _second; // Front/back buffer #2.
};


static PortAudioOutput::TPortAudioConfig currentAudioConfig; // Must be static because the PlaybackCallback is static (PortAudio works that way).
static std::unique_ptr<VisualizationBuffer> visBuffer = nullptr;

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

void PortAudioOutput::SetVolume(float volume)
{
    assert(volume >= 0.0f && volume <= 1.0f);
    currentAudioConfig.volume = volume;
}

void PortAudioOutput::InitVisualizationBuffer(size_t length)
{
    if (length == 0)
    {
        visBuffer = nullptr;
    }
    else
    {
        visBuffer = std::make_unique<VisualizationBuffer>(length);
    }
}

size_t PortAudioOutput::GetVisualizationWaveform(short* out) const
{
    if (visBuffer == nullptr)
    {
        return 0;
    }

    return visBuffer->Read(out);
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
    const unsigned long safeFramesPerBuffer = samplerate * LIBSIDPLAYFP_MIN_BUFFER_LATENCY;
    PaError err = Pa_OpenStream(&_stream, NULL, &currentAudioConfig, samplerate,
                                safeFramesPerBuffer, // Don't use the paFramesPerBufferUnspecified due to libsidplayfp issue (see comment on the constant).
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
    // Write to output device
    IBufferWriter* externalSource = static_cast<IBufferWriter*>(userData);
    const bool successful = externalSource->TryFillBuffer(outputBuffer, framesPerBuffer);

    // Common
    short* const out = static_cast<short*>(outputBuffer);
    const uint_least32_t length = framesPerBuffer * currentAudioConfig.channelCount;

    if (successful)
    {
        // Update the visualization buffer
        if (visBuffer != nullptr)
        {
            visBuffer->Write(out, length);
        }

        // Apply volume scale if needed
        const float volume = currentAudioConfig.volume;
        if (volume != 1.0f)
        {
            for (uint_least32_t i = 0; i < length; ++i)
            {
                out[i] *= volume;
            }
        }
    }

    return (successful) ? paContinue : paAbort; // Reminder: there is also paComplete, so see about it when we reach the end maybe
}
