/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021 Jasmin Rutic (bytespiller@gmail.com)
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

#include <string>
#include <unordered_map>

namespace RomUtil
{
	static constexpr const size_t ROM_SIZE_KERNAL = 8192;
    static constexpr const size_t ROM_SIZE_BASIC = 8192;
    static constexpr const size_t ROM_SIZE_CHARGEN = 4096;

	enum class RomType : int
	{
		Kernal = 0,
		Basic = 1,
		Chargen = 2
	};

	class RomStatus
	{
	public:
		RomStatus() = default;

	public:
		bool IsValidated(RomType type) const;
		bool AreAllValidated() const;
		void Mark(RomType type, bool value);

	private:
		std::unordered_map<RomType, bool> _romStatus{
			{RomType::Kernal, false},
			{RomType::Basic, false},
			{RomType::Chargen, false}
		};
	};

	// Sanity check if non-empty paths point to a missing or obviously incorrect files. Paths should be absolute.
    RomStatus PreCheckRoms(const std::wstring& pathKernal, const std::wstring& pathBasic, const std::wstring& pathChargen);
}
