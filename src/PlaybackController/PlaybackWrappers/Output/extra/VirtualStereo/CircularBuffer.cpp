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

#include "CircularBuffer.h"

#include <cstring> // memcpy
#include <stdexcept>

CircularBuffer::CircularBuffer(size_t capacity) :
	_capacity(capacity),
	_data(new short[_capacity])
{
	if (_capacity == 0)
	{
		delete[] _data;
		throw std::invalid_argument("Capacity is zero.");
	}
}

CircularBuffer::~CircularBuffer()
{
	delete[] _data;
}

void CircularBuffer::CopyFrom(const short* const payload, size_t length)
{
#ifndef NDEBUG
	if (length > _capacity) throw std::overflow_error("Too much data.");
#endif

	const size_t chunkForward = std::min(length, _capacity - _writePos);
	const size_t chunkWrapped = length - chunkForward;

	std::memcpy(_data + _writePos, payload, chunkForward * sizeof(short));

	if (chunkWrapped > 0)
	{
		std::memcpy(_data, payload + chunkForward, chunkWrapped * sizeof(short));
	}

	_writePos = (_writePos + length) % _capacity;
	_length = std::min(_capacity, _length + length);
}

void CircularBuffer::Peek(short*& out1, size_t& len1, short*& out2, size_t& len2, size_t length) const
{
	const size_t chunkForward = std::min(length, _capacity - _readPos);
	const size_t chunkWrapped = length - chunkForward;

	out1 = _data + _readPos;
	len1 = chunkForward;

	if (chunkWrapped > 0)
	{
		out2 = _data;
		len2 = chunkWrapped;
	}
	else
	{
		out2 = nullptr;
		len2 = 0;
	}
}

void CircularBuffer::Advance(size_t length)
{
	_readPos = (_readPos + length) % _capacity;
}

size_t CircularBuffer::GetCapacity() const
{
	return _capacity;
}

size_t CircularBuffer::GetLength() const
{
	return _length;
}

/// @brief Whether the buffer is completely filled (further writes will go in circles).
bool CircularBuffer::IsSaturated() const
{
	return _length == _capacity;
}

void CircularBuffer::Reset()
{
	_length = 0;
	_writePos = 0;
	_readPos = 0;
}
