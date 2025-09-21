/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "CircularBuffer.h"
#include "LinearBuffer.h"

class VirtualStereo
{
public:
	VirtualStereo() = delete;
	VirtualStereo(unsigned int sampleRate, unsigned int offsetMs, float sideVolumeFactor);

public:
	/// @brief Applies the effect. 16bit stereo stream is assumed.
	void Apply(short* const out, const size_t framesPerBuffer);

	/// @brief Flushes buffers so it's ready for new playback.
	void Reset();

	void ChangeSideVolumeFactor(float factor);

private:
	const size_t _framesOffset = 0;
	size_t _warmupReadSamples = 0;
	CircularBuffer _delayBuffer;
	LinearBuffer _snapshotBuffer;

	float _sideVolumeFactor = 0.5; // 0.5 is maximum
	float _centerVolumeFactor = 0.5;
};
