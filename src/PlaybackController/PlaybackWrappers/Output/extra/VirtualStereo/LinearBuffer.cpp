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

#include "LinearBuffer.h"

#include <cstring> // memcpy
#include <stdexcept>

LinearBuffer::~LinearBuffer()
{
	free(_data);
}

void LinearBuffer::CopyFrom(const short* const src, size_t length)
{
#ifndef NDEBUG
	if (src == nullptr) [[unlikely]]
	{
		throw std::invalid_argument("Source is nullptr.");
	}

	if (length == 0) [[unlikely]]
	{
		throw std::invalid_argument("Length must be above zero.");
	}
#endif

	if (length > _length) [[unlikely]]
	{
		_data = static_cast<short*>(realloc(_data, length * sizeof(short)));
		if (_data == nullptr) [[unlikely]]
		{
#ifndef NDEBUG
			free(_data);
			throw std::runtime_error("realloc failed.");
#endif
			return;
		}
	}

	std::memcpy(_data, src, length * sizeof(short));
	_length = length;
}

void LinearBuffer::CopyTo(short* out)
{
#ifndef NDEBUG
	if (out == nullptr) [[unlikely]]
	{
		throw std::invalid_argument("Output is nullptr.");
	}

	if (_length == 0) [[unlikely]]
	{
		throw std::runtime_error("Attempting to read data from empty buffer.");
	}
#endif
	std::memcpy(out, _data, _length * sizeof(short));
}

const short* LinearBuffer::GetData() const
{
	return _data;
}
