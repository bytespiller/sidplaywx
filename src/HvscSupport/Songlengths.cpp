/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "Songlengths.h"
#include "../Util/Const.h"
#include "../Util/HelpersGeneral.h"
#include <sidplayfp/SidTuneInfo.h>
#include <fstream>
#include <vector>

static constexpr uint_least8_t MD5_LEN = 32; // Standard length of MD5 hashes is 32 characters.

static constexpr const char CHAR_INI_COMMENT = ';';
static constexpr const char CHAR_INI_SECTION = '[';

static constexpr const char CHAR_HASH_DURATION_SEPARATOR = '=';
static constexpr const char CHAR_SUBSONG_DURATION_SEPARATOR = ' ';

static constexpr const char* const INI_SECTION_DATABASE = "[Database]";

// -----------------------------------------------------------

bool Songlengths::TryLoad(const std::wstring& songlengthsMd5Filepath)
{
	Unload();

	std::ifstream file(songlengthsMd5Filepath.c_str());

	bool inDatabaseSection = false;
	std::string line;

	std::string songHvscPath;
	while (std::getline(file, line))
	{
		const char& firstChar = line.front();

		// Ignore if not within Database section(s)
		if (firstChar == CHAR_INI_SECTION)
		{
			inDatabaseSection = line == INI_SECTION_DATABASE;
			continue;
		}

		if (!inDatabaseSection)
		{
			continue;
		}

		// Get HVSC path (or skip the comment)
		if (firstChar == CHAR_INI_COMMENT)
		{
			if (line.length() > 2 && line.at(1) == ' ' && line.at(2) == '/')
			{
				songHvscPath = line.substr(2);
			}

			continue;
		}

		// Ignore if the line doesn't appear to contain MD5 hash and duration
		if (line.length() > MD5_LEN && line.at(MD5_LEN) != CHAR_HASH_DURATION_SEPARATOR)
		{
			continue;
		}

		// ---------------------------------------------------

		const std::string& md5Value = line.substr(0, MD5_LEN);
		const std::string& durations = line.substr(MD5_LEN + 1);

		_database[md5Value] = std::move(std::make_unique<HvscInfoRaw>(durations, songHvscPath));
	}

	file.close();

	return IsLoaded();
}

void Songlengths::Unload()
{
	_database.clear();
}

bool Songlengths::IsLoaded() const
{
	return !_database.empty();
}

// mm:ss[.SSS]
//
// Examples of song length values:
//
// 1:02
//
// 1:02.5
//
// 1:02.500
//
uint_least32_t Songlengths::GetDurationMs(const std::string& preformattedDuration)
{
	// Convert string duration to integers
	std::vector<std::string> subsongDurationSliced = Helpers::General::SplitString(preformattedDuration, ':');
	const std::string& secondsRaw = subsongDurationSliced.back();
	const bool hasMillis = secondsRaw.find('.') != std::string::npos;

	int minutes = stoi(subsongDurationSliced.front());
	int seconds = 0;
	int extraMillis = 0;

	if (hasMillis)
	{
		std::vector<std::string> secondsMillis = Helpers::General::SplitString(secondsRaw, '.');
		seconds = stoi(secondsMillis.front());
		extraMillis = stoi(secondsMillis.back());
	}
	else
	{
		seconds = stoi(secondsRaw);
	}

	// Convert integer duration to milliseconds
	return (minutes * Const::MILLISECONDS_IN_MINUTE) + (seconds * Const::MILLISECONDS_IN_SECOND) + extraMillis;
}

Songlengths::HvscInfo Songlengths::GetHvscInfo(const char* tuneMd5, int subsong) const
{
	if (_database.empty())
	{
		throw std::runtime_error("Database wasn't loaded!");
	}

	// Sanity checks
	if (tuneMd5 == 0)
	{
		return HvscInfo(); // Tune wasn't loaded.
	}

	const auto& it = _database.find(tuneMd5);
	if (it == _database.cend())
	{
		return HvscInfo(); // No tune in database.
	}

	const std::string& durationsRaw = it->second->durations;
	std::vector<std::string> subsongsDurations = Helpers::General::SplitString(durationsRaw, CHAR_SUBSONG_DURATION_SEPARATOR);
	if (subsongsDurations.size() < subsong)
	{
		return HvscInfo(); // Database doesn't contain this subsong.
	}

	const std::string& preformattedDuration = subsongsDurations.at(subsong - 1);
	return HvscInfo(GetDurationMs(preformattedDuration), it->second->hvscPath, tuneMd5);
}
