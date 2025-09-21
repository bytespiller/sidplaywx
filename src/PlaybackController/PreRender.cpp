/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "PreRender.h"

#include <algorithm>
#include <cmath>
#include <cstring> // memcpy & memset

static size_t GRANULARITY = 4096; // Buffer granularity in thread fill-loop.
constexpr std::chrono::milliseconds SEEK_CHECK_SLEEP_MS(15); // Note: increasing this value decreases the indicator smoothness. 15ms should be ideal (especially due to MSW system timer resolution).

PreRender::~PreRender()
{
	DestroyData();
}

void PreRender::DoPreRender(IBufferWriter& renderer, int sampleRate, int numChannels, int durationMs)
{
	AbortPreRender();

	const double sampleRatePerMs = sampleRate / 1000.0;
	_numChannels = numChannels;
	_stridePerMs = sampleRatePerMs * numChannels;

	const int frames = std::ceil(durationMs * sampleRatePerMs);
	const size_t size = frames * _numChannels * sizeof(short);

	short* result = static_cast<short*>(realloc(_waveBufferContent, size));
	if (result == nullptr)
	{
		DestroyData();
		return; // Out of memory. Should probably throw an exception rather than refusing to play?
	}

	_waveBufferContent = result;
	_waveBufferSize = size;

	_playbackPosition = 0;
	_preRenderedSize = 0;
	_abortPreRenderFlag = false;

	_thread = std::thread([this, size, &renderer]
    {
		const size_t maxFramesPerBuffer = std::min(GRANULARITY, size);

		while (!_abortPreRenderFlag && _preRenderedSize < size)
		{
			size_t chunk = maxFramesPerBuffer;

			// Ensure the last one is trimmed-down
			const size_t payloadSize = chunk * _numChannels * sizeof(short);
			size_t newSize = _preRenderedSize + payloadSize;
			if (newSize > size)
			{
				chunk = (size - _preRenderedSize) / _numChannels / sizeof(short);
				newSize = size;
			}

			const bool success = renderer.TryFillBuffer(static_cast<short*>(_waveBufferContent) + (_preRenderedSize / sizeof(short)), chunk); // Reminder: calls SidDecoder's TryFillBuffer(), not ours.
			if (success)
			{
				_preRenderedSize = newSize;
			}
			else
			{
				break;
			}
		}
	});
}

bool PreRender::TryFillBuffer(void* buffer, unsigned long framesPerBuffer)
{
	const size_t length = framesPerBuffer * _numChannels;
	const size_t framesTo = length * sizeof(short);
	_playbackPosition += length;

	if ((_playbackPosition * _numChannels) + framesTo > _preRenderedSize)
	{
		std::memset(buffer, 0, framesTo);
		return true;
	}

	std::memcpy(buffer, _waveBufferContent + _playbackPosition, framesTo);
	return true;
}

int PreRender::GetCurrentSongTimeMs() const
{
	return _playbackPosition / _stridePerMs;
}

double PreRender::GetPreRenderProgressFactor() const
{
	if (_waveBufferSize == 0)
	{
		return 0.0;
	}

	return std::clamp(static_cast<double>(_preRenderedSize) / _waveBufferSize, 0.0, 1.0);
}

void PreRender::Stop()
{
	AbortPreRender();
	_playbackPosition = 0;
}

void PreRender::SeekTo(int timeMs, const SeekStatusCallback& callback)
{
	int availTimeMs = (_preRenderedSize / sizeof(short)) / _stridePerMs;
	while (availTimeMs < timeMs)
	{
		if (callback(availTimeMs, false))
		{
			return;
		}

		std::this_thread::sleep_for(SEEK_CHECK_SLEEP_MS);

		availTimeMs = (_preRenderedSize / sizeof(short)) / _stridePerMs;
	}

	const size_t wantedPlaybackPosition = timeMs * _stridePerMs;
	_playbackPosition = std::min(wantedPlaybackPosition, static_cast<size_t>(_waveBufferSize) / sizeof(short));
	callback(timeMs, true);
}

void PreRender::AbortPreRender()
{
	if (_thread.joinable())
	{
		_abortPreRenderFlag = true;
		_thread.join();
	}

	_abortPreRenderFlag = false;
}

void PreRender::DestroyData()
{
	AbortPreRender();

	free(_waveBufferContent);
	_waveBufferContent = nullptr;
	_waveBufferSize = 0;
	_preRenderedSize = 0;
}
