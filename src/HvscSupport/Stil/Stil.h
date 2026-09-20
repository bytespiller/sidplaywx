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
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Stil
{
public:
	/// @brief Field that may contain multiple entries.
	using Field = std::unordered_map<int, std::vector<std::string>>;

	struct Info
	{
		Info() = default;

		/// @brief Returns subsong-specific STIL field entries if possible, or the main field entries.
		std::vector<std::string> GetField(const Field& field, int subsong) const;

		/// @brief Returns as a comma-separated (by default) string: subsong-specific STIL field entries if possible, or the main field entries.
		std::string GetFieldAsString(const Field& field, int subsong, const std::string& separator = ", ", bool countSeparators = false) const;

		Field names;
		Field titles;
		Field artists;
		Field authors;
		Field comments;
	};

public:
	Stil();
	~Stil();

public:
	bool TryLoad(const std::filesystem::path& stilFilepath);
	void Unload();

	bool IsLoaded() const;

	/// @brief Thread-safe: opens its own local file stream per call, so it can be called concurrently (e.g., from multiple background parser threads and/or the GUI thread) and safely alongside TryLoad/Unload.
	Info Get(const std::string& tuneHvscPath);

private:
	// Guards _stilFilepath and _hvscPathsIndex (both replaced wholesale by Unload()/TryLoad(), e.g. when the user changes the STIL path in Preferences while a background parse is in flight).
	// Only the index lookup (fast) happens under the lock; the actual file I/O in Get() happens afterwards, unlocked, on a call-local stream.
	mutable std::mutex _dataMutex;
	std::filesystem::path _stilFilepath;
	HvscPathsIndex _hvscPathsIndex;
};
