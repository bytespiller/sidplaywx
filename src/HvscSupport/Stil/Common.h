/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include <unordered_map>
#include <string>

using HvscPathsIndex = std::unordered_map<std::string, int>;

// Removes final CR from the string if present.
inline void ClipCarriageReturn(std::wstring& str)
{
	if (str.back() == L'\r')
	{
		str.pop_back();
	}
}

// Removes final CR from the string if present.
inline void ClipCarriageReturn(std::string& str)
{
	if (str.back() == '\r')
	{
		str.pop_back();
	}
}