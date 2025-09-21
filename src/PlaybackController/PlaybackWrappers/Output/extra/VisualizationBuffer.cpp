/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2024-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "VisualizationBuffer.h"

#include <cstring> // memcpy

VisualizationBuffer::VisualizationBuffer(size_t aLength) :
	maxLength(aLength)
{
	_first = new std::atomic_short[maxLength];
	_second = new std::atomic_short[maxLength];
}

VisualizationBuffer::~VisualizationBuffer()
{
	delete[] _first;
	_first = nullptr;

	delete[] _second;
	_second = nullptr;
}

size_t VisualizationBuffer::Read(short* out) const
{
	if (!_ready)
	{
		return 0;
	}

	const std::atomic_short* const buffer = (_flipped) ? _second : _first;
	std::memcpy(out, buffer, maxLength * sizeof(short));
	return maxLength;
}

void VisualizationBuffer::Write(const short* const data, size_t dataLength)
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
