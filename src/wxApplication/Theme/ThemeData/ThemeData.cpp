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

#include "ThemeData.h"

#include "ThemedElementData.h"
#include "ThemeImage.h"

namespace ThemeData
{
	ThemeData::ThemeData(const std::string& displayName, std::unique_ptr<ThemeImages>& adoptThemeImages, std::unique_ptr<ThemedElements>& adoptThemedElements) :
		_displayName(displayName),
		_images(std::move(adoptThemeImages)),
		_themedElements(std::move(adoptThemedElements))
	{
	}

	const std::string& ThemeData::GetDisplayName() const
	{
		return _displayName;
	}

	const ThemeImage& ThemeData::GetImage(const std::string& name) const
	{
		const auto it = std::find_if(_images->begin(), _images->end(), [&name](const ThemeImage& themeImage)
		{
			return themeImage.name == name;
		});

		assert(it != _images->end());
		return *it;
	}

	const ThemedElementData& ThemeData::GetThemedElement(const std::string& name) const
	{
		const auto it = _themedElements->find(name);
		assert(it != _themedElements->end());
		return it->second;
	}
}
