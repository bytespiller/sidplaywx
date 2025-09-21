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

class LinearBuffer
{
public:
	LinearBuffer() = default;
	LinearBuffer(const LinearBuffer&) = delete;
	LinearBuffer& operator=(const LinearBuffer&) = delete;

	~LinearBuffer();

public:
	void CopyFrom(const short* const src, size_t length);
	void CopyTo(short* out);

	const short* GetData() const;

private:
	short* _data = nullptr;
	size_t _length = 0;
};
