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

#include "extra/VisualizationBuffer.h"
#include "extra/VirtualStereo/VirtualStereo.h"

#include <assert.h>
#include <iostream>
#include <memory>

static constexpr double LIBSIDPLAYFP_MIN_BUFFER_LATENCY = 5.6 / 1000.0; // Ensure safe minimum buffer size due to libsidplayfp change in commit 1a6d9016e8bc35fa88d429c1ca77f31c5f5f6831 causing crash with ALSA & PulseAudio when using the paFramesPerBufferUnspecified (auto-size).

static PortAudioOutput::AudioConfig currentAudioConfig; // Must be static because the PlaybackCallback is static (PortAudio works that way).
static std::unique_ptr<VisualizationBuffer> visBuffer = nullptr;
static std::unique_ptr<VirtualStereo> virtualStereo = nullptr;

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

bool PortAudioOutput::TryInit(const AudioConfig& audioConfig, IBufferWriter* bufferWriter, double playbackSpeedFactor)
{
    if (_stream != nullptr && Pa_IsStreamActive(_stream))
    {
        StopStream(true);
    }

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
        assert(currentAudioConfig.sampleRate >= 8000 && currentAudioConfig.sampleRate <= 192000); // libsidplayfp supports sample rates in this range only.
        success = ResetStream(currentAudioConfig.sampleRate * playbackSpeedFactor) == paNoError;
    }

    return success;
}

bool PortAudioOutput::TryStartStream()
{
    if (virtualStereo != nullptr)
    {
        virtualStereo->Reset();
    }

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
    Pa_CloseStream(_stream);

    // Open an audio I/O stream.
    const unsigned long safeFramesPerBuffer = samplerate * LIBSIDPLAYFP_MIN_BUFFER_LATENCY;
    PaError err = Pa_OpenStream(&_stream, NULL, &currentAudioConfig, samplerate,
#ifdef _WIN32
                                paFramesPerBufferUnspecified,
#else
                                safeFramesPerBuffer, // Don't use the paFramesPerBufferUnspecified due to libsidplayfp issue (see comment on the constant).
#endif
                                paNoFlag,
                                PlaybackCallback,
                                _bufferWriter);

    const bool failed = LogAnyError("ResetStream: Pa_OpenStream", err);
    if (failed)
    {
        _stream = nullptr;
    }
    else
    {
        SetVirtualStereo(_fxConfig.virtualStereoExpansionOffsetMs, _fxConfig.virtualStereoSideVolumeFactor);
    }

    return err;
}

void PortAudioOutput::SetVirtualStereo(unsigned int offsetMs, float sideVolumeFactor)
{
    _fxConfig.virtualStereoExpansionOffsetMs = offsetMs;
    _fxConfig.virtualStereoSideVolumeFactor = sideVolumeFactor;
    if (_fxConfig.virtualStereoExpansionOffsetMs > 0)
    {
        // Reminder: HaaS domain splits this offset in half, so offset parameter below 2ms is invalid.
        virtualStereo = std::make_unique<VirtualStereo>(currentAudioConfig.sampleRate, _fxConfig.virtualStereoExpansionOffsetMs, _fxConfig.virtualStereoSideVolumeFactor);
    }
    else
    {
        virtualStereo = nullptr;
    }
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

    if (!successful)
    {
        return paAbort;
    }

    // Common
    short* const out = static_cast<short*>(outputBuffer);
    const size_t length = framesPerBuffer * currentAudioConfig.channelCount;

    // Update the visualization buffer
    if (visBuffer != nullptr)
    {
        visBuffer->Write(out, length);
    }

    // Apply VirtualStereo
    if (virtualStereo != nullptr && currentAudioConfig.channelCount == 2)
    {
        virtualStereo->Apply(out, framesPerBuffer);
    }

    // Apply volume (down)scale if needed
    const float volume = currentAudioConfig.volume;
    if (volume != 1.0f)
    {
        for (size_t i = 0; i < length; ++i)
        {
            out[i] *= volume;
        }
    }

    return paContinue; // Reminder: there is also paComplete, so see about it when we reach the end maybe
}
