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

#include "SidMixer.h"

#include <sidplayfp/sidplayfp.h>
#include <sidplayfp/SidInfo.h>
#include <cmath>
#include <cstdint>

static constexpr float VOLUME_FACTOR_1SID = 1.0f;
static const float VOLUME_FACTOR_2SID = 1.0f / std::sqrt(2.0f);
static const float VOLUME_FACTOR_3SID = 1.0f / std::sqrt(3.0f);

SidMixer::SidMixer(sidplayfp& sidEngine) :
	_sidEngine(sidEngine),
	_numSidChips(_sidEngine.installedSIDs()),
	_sidVolumeFactor((_numSidChips == 1) ? VOLUME_FACTOR_1SID : (_numSidChips == 2) ? VOLUME_FACTOR_2SID : VOLUME_FACTOR_3SID),
	_numChannels(_sidEngine.info().channels())
{
	_sidEngine.buffers(_sidChipsBuffers);
}

void SidMixer::FillBuffer(void* buffer, unsigned long framesPerBuffer)
{
	short* const out = static_cast<short*>(buffer);
	const unsigned long capacity = (_numChannels == 2) ? framesPerBuffer << 1 : framesPerBuffer; // *2 for stereo

	unsigned int written = 0;
	while (written < capacity)
	{
		_samplesLen = (_samplesLen == 0) ? _sidEngine.play(LIBSIDPLAYFP_PLAY_CYCLES) : _samplesLen;

		for (; _samplesPos < _samplesLen; ++_samplesPos)
		{
			for (unsigned int channel = 0; channel < _numChannels; ++channel)
			{
				int_least32_t tmp = _sidChipsBuffers[0][_samplesPos] * _sidVolumeFactor; // SID 1
				for (unsigned int chip = 1; chip < _numSidChips; ++chip) // SID 2 & 3 (if used)
				{
					tmp += std::lrintf(_sidChipsBuffers[chip][_samplesPos] * _sidVolumeFactor);
				}

				out[written++] = static_cast<short>(tmp);

				if (written == capacity) [[unlikely]]
				{
					++_samplesPos;
					return; // overflow pending
				}
			}
		}

		_samplesPos = 0;
		_samplesLen = 0;
	}
}
