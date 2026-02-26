/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include <sidplayfp/SidTune.h>
#include <string>

class TuneUtil
{
public:
    enum class SongInfoCategory
    {
        Title = 0,
        Author,
        Released // a.k.a. Copyright
    };

	enum class RomRequirement
    {
        None,
        BasicRom,
        R64
    };

public:
	static std::string GetTuneInfoString(const SidTune& tune, SongInfoCategory category);
    static std::string GetTuneMusComments(const SidTune& tune);
	static RomRequirement GetTuneRomRequirement(const SidTune& tune); // TODO: perhaps this function is now redundant?

	inline static void trimString(std::string& str)
	{
		str.erase(str.find_last_not_of(' ') + 1); // ltrim
		str.erase(0, str.find_first_not_of(' ')); // rtrim
	}
};
