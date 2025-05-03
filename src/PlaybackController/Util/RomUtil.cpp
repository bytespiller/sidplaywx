/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "RomUtil.h"
#include <algorithm>
#include <filesystem>

namespace RomUtil
{
	bool RomStatus::IsValidated(RomType type) const
	{
		return _romStatus.at(type);
	}

	bool RomStatus::AreAllValidated() const
	{
		return std::all_of(_romStatus.cbegin(), _romStatus.cend(), [](const auto& status){return status.second;});
	}

	void RomStatus::Mark(RomType type, bool value)
	{
		_romStatus.at(type) = value;
	}
}

namespace RomUtil
{
	RomStatus PreCheckRoms(const std::filesystem::path& pathKernal, const std::filesystem::path& pathBasic, const std::filesystem::path& pathChargen)
	{
		using PathSizePair = std::pair<std::filesystem::path, size_t>;
		const std::unordered_map<RomType, PathSizePair> info
		{
			{RomType::Kernal, {pathKernal, ROM_SIZE_KERNAL}},
			{RomType::Basic, {pathBasic, ROM_SIZE_BASIC}},
			{RomType::Chargen, {pathChargen, ROM_SIZE_CHARGEN}}
		};

		RomStatus preCheckStatus;
		for (const auto& romInfo : info)
		{
			const RomType romType = romInfo.first;
			const PathSizePair& pathSize = romInfo.second;
			const std::filesystem::path& romPath = pathSize.first;
			const size_t romSize = pathSize.second;

			const bool preCheckPassed = romPath.empty() || (std::filesystem::exists(romPath) && std::filesystem::file_size(romPath) == romSize);
			preCheckStatus.Mark(romType, preCheckPassed);
		}

		return preCheckStatus;
	}
}
