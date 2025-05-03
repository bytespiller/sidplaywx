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

#pragma once

#include "Common.h"
#include <fstream>

class PreIndex
{
public:
	PreIndex() = delete;
	PreIndex(PreIndex&) = delete;

public:
	static bool TryLoadFromCache(const std::string& stilVersion, HvscPathsIndex& pathsIndex);
	static void RebuildIndexAndCache(const std::string& stilVersion, HvscPathsIndex& pathsIndex, std::ifstream& stilDataStream);

private:
	// Reads the next line of a stream (advancing it) and checks its content against a given one. This function allows for chained one-liners.
	static inline bool NextLineEquals(std::ifstream& stream, std::string& lineBuffer, const char* compareWith)
	{
		std::getline(stream, lineBuffer);
		return (lineBuffer == compareWith);
	}

	// Reads the next line of a stream (advancing it) and returns the value as an integer if possible, otherwise returns the fallbackValue.
	static inline int NextLineAsInt(std::ifstream& stream, std::string& lineBuffer, int fallbackValue = -1)
	{
		std::getline(stream, lineBuffer);
		try
		{
			return std::stoi(lineBuffer);
		}
		catch (std::invalid_argument)
		{
			return fallbackValue;
		}
	}
};
