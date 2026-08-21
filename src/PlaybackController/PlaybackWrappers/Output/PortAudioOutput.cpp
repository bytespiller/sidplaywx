/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2026 Jasmin Rutic (bytespiller@gmail.com)
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

static constexpr double LIBSIDPLAYFP_MIN_BUFFER_LATENCY_SECONDS = 5.6 / 1000.0; // Ensure safe minimum buffer size due to libsidplayfp change in commit 1a6d9016e8bc35fa88d429c1ca77f31c5f5f6831 causing crash with ALSA & PulseAudio when using the paFramesPerBufferUnspecified (auto-size).

namespace PlaybackThreadState // Static because the PlaybackCallback is static (PortAudio works that way)
{
    static PortAudioOutput::AudioConfig currentAudioConfig;
    static std::unique_ptr<VisualizationBuffer> visBuffer = nullptr;
    static std::unique_ptr<VirtualStereo> virtualStereo = nullptr;
}

PortAudioOutput::~PortAudioOutput()
{
    Pa_CloseStream(_stream);
    LogAnyError("~PortAudioOutput -> Pa_Terminate", Pa_Terminate());
    _stream = nullptr;
    _bufferWriter = nullptr;
}

float PortAudioOutput::GetVolume()
{
    return PlaybackThreadState::currentAudioConfig.volume;
}

void PortAudioOutput::SetVolume(float volume)
{
    assert(volume >= 0.0f && volume <= 1.0f);
    PlaybackThreadState::currentAudioConfig.volume = volume;
}

void PortAudioOutput::InitVisualizationBuffer(size_t length)
{
    if (length == 0)
    {
        PlaybackThreadState::visBuffer = nullptr;
    }
    else
    {
        PlaybackThreadState::visBuffer = std::make_unique<VisualizationBuffer>(length);
    }
}

size_t PortAudioOutput::GetVisualizationWaveform(short* out) const
{
    if (PlaybackThreadState::visBuffer == nullptr)
    {
        return 0;
    }

    return PlaybackThreadState::visBuffer->Read(out);
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
        PlaybackThreadState::currentAudioConfig = AudioConfig(audioConfig);
        PlaybackThreadState::currentAudioConfig.hostApiSpecificStreamInfo = NULL; // Without this you get an error in the release mode.
        PlaybackThreadState::currentAudioConfig.device = outputDevice;
        PlaybackThreadState::currentAudioConfig.sampleFormat = paInt16; // Must be 16 bit (libsidplayfp expects 16 bit buffer).

        PlaybackThreadState::currentAudioConfig.suggestedLatency = (PlaybackThreadState::currentAudioConfig.lowLatency) ? deviceInfo.defaultLowOutputLatency : deviceInfo.defaultHighOutputLatency;
        PlaybackThreadState::currentAudioConfig.suggestedLatency = std::max(LIBSIDPLAYFP_MIN_BUFFER_LATENCY_SECONDS, PlaybackThreadState::currentAudioConfig.suggestedLatency); // See comment on the constant for why we do this.

        // Open an audio I/O stream.
        assert(PlaybackThreadState::currentAudioConfig.sampleRate >= 8000 && PlaybackThreadState::currentAudioConfig.sampleRate <= 192000); // libsidplayfp supports sample rates in this range only.
        success = ResetStream(PlaybackThreadState::currentAudioConfig.sampleRate * playbackSpeedFactor) == paNoError;
    }

    return success;
}

bool PortAudioOutput::TryStartStream()
{
    if (PlaybackThreadState::virtualStereo != nullptr)
    {
        PlaybackThreadState::virtualStereo->Reset();
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
    PaError err = Pa_OpenStream(&_stream, NULL, &PlaybackThreadState::currentAudioConfig, samplerate,
                                paFramesPerBufferUnspecified,
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
        PlaybackThreadState::virtualStereo = std::make_unique<VirtualStereo>(PlaybackThreadState::currentAudioConfig.sampleRate, _fxConfig.virtualStereoExpansionOffsetMs, _fxConfig.virtualStereoSideVolumeFactor);
    }
    else
    {
        PlaybackThreadState::virtualStereo = nullptr;
    }
}

const PortAudioOutput::AudioConfig& PortAudioOutput::GetAudioConfig() const
{
    return PlaybackThreadState::currentAudioConfig;
}

bool PortAudioOutput::IsOutputSampleRateSupported(double samplerate) const
{
    return Pa_IsFormatSupported(NULL, &PlaybackThreadState::currentAudioConfig, samplerate) == paFormatIsSupported;
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
    const size_t length = framesPerBuffer * PlaybackThreadState::currentAudioConfig.channelCount;

    // Update the visualization buffer
    if (PlaybackThreadState::visBuffer != nullptr)
    {
        PlaybackThreadState::visBuffer->Write(out, length);
    }

    // Apply VirtualStereo
    if (PlaybackThreadState::virtualStereo != nullptr && PlaybackThreadState::currentAudioConfig.channelCount == 2)
    {
        PlaybackThreadState::virtualStereo->Apply(out, framesPerBuffer);
    }

    // Apply volume (down)scale if needed
    const float volume = PlaybackThreadState::currentAudioConfig.volume;
    if (volume != 1.0f)
    {
        for (size_t i = 0; i < length; ++i)
        {
            out[i] *= volume;
        }
    }

    /* Commented-out because it's not worth it currently (no benefit for additional complexity upstream), but I plan to revisit it.
    // Precise playback duration
    const unsigned long framesTotal = PlaybackThreadState::framesTotal.load(std::memory_order_relaxed);
    if (framesTotal > 0)
    {
        const unsigned long framesRendered = PlaybackThreadState::framesRendered.load(std::memory_order_relaxed) + framesPerBuffer;

        if (framesRendered >= framesTotal) [[unlikely]]
        {
            const unsigned long framesOverflow = framesRendered - framesTotal;
            const unsigned long framesToSilence = (framesOverflow > framesPerBuffer) ? framesPerBuffer : framesOverflow;
            const unsigned long samplesToSilenceStart = length - (framesToSilence * PlaybackThreadState::currentAudioConfig.channelCount);

            std::fill(out + samplesToSilenceStart, out + length, 0.0f);

            PlaybackThreadState::framesRendered.store(framesTotal, std::memory_order_relaxed);

            PlaybackThreadState::notifyPlaybackStatusChanged(PortAudioOutput::PlaybackStatusChanged::Complete);
            return paComplete;
        }

        PlaybackThreadState::framesRendered.store(framesRendered, std::memory_order_relaxed);
    }
    */

    return paContinue;
}
