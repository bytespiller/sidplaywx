/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "PlaybackWrappers/IBufferWriter.h"
#include <atomic>
#include <memory>
#include <thread>

class PreRender : public IBufferWriter
{
public:
	PreRender() = default;
	PreRender(PreRender&) = delete;

	~PreRender();

public:
	void DoPreRender(IBufferWriter& renderer, int sampleRate, int numChannels, int durationMs);
	bool TryFillBuffer(void* buffer, unsigned long framesPerBuffer) override;

public:
	int GetCurrentSongTimeMs() const;
	double GetPreRenderProgressFactor() const;

	void Stop();
	void SeekTo(int timeMs, const SeekStatusCallback& callback);

private:
	void AbortPreRender();
	void DestroyData();

private:
	int _numChannels = 0;
	double _stridePerMs = 0; // _sampleRatePerMs * _numChannels

	size_t _playbackPosition = 0;

	std::thread _thread;
	std::atomic<short*> _waveBufferContent = nullptr;
	std::atomic_size_t _waveBufferSize = 0;
	std::atomic_size_t _preRenderedSize = 0;
	std::atomic_bool _abortPreRenderFlag = false;
};
