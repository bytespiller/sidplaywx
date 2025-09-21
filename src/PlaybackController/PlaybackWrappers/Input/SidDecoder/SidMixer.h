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

class sidplayfp;

class SidMixer
{
public:
	static constexpr unsigned int LIBSIDPLAYFP_PLAY_CYCLES = 3000; // Roughly 3ms buffer granularity.

public:
	SidMixer() = delete;
	SidMixer(SidMixer&) = delete;
	SidMixer& operator=(const SidMixer&) = delete;

	explicit SidMixer(sidplayfp& sidEngine);

public:
	void FillBuffer(void* buffer, unsigned long framesPerBuffer);

private:
	sidplayfp& _sidEngine;

	const unsigned int _numSidChips = 0;
	const float _sidVolumeFactor = 0;
	const unsigned int _numChannels = 0;
	short* _sidChipsBuffers[3];

	int _samplesPos = 0;
	int _samplesLen = 0;
};
