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

#include <cstddef>

class CircularBuffer
{
public:
	CircularBuffer() = delete;
	CircularBuffer(const CircularBuffer&) = delete;
	CircularBuffer& operator=(const CircularBuffer&) = delete;

	explicit CircularBuffer(size_t capacity);

	~CircularBuffer();

public:
	void CopyFrom(const short* const payload, size_t length);
	void Peek(short*& out1, size_t& len1, short*& out2, size_t& len2, size_t length) const;
	void Advance(size_t length);

	size_t GetCapacity() const;

	/// @brief Returns length of written payload (up to capacity).
	size_t GetLength() const;

	/// @brief Whether the buffer is completely filled (further writes will go in circles).
	bool IsSaturated() const;

	void Reset();

private:
	const size_t _capacity = 0;
	short* const _data = nullptr;

	size_t _length = 0;
	size_t _writePos = 0;
	size_t _readPos = 0;
};
