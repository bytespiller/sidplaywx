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

#include "ThemedElementData.h"

namespace ThemeData
{
	ThemedElementData::ThemedElementData(const ColoredProperties& colors) :
		_colors(colors)
	{
	}

	wxColor ThemedElementData::GetPropertyColor(const std::string& property) const
	{
		const auto it = _colors.find(property);
		return (it == _colors.cend()) ? wxColor() : (*it).second;
	}
}
