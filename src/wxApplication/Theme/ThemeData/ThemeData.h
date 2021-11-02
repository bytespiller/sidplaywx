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

#include "ThemedElementData.h"
#include "ThemeImage.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ThemeData
{
	class ThemeData
	{
	public:
		using ThemeImages = std::vector<ThemeImage>;
		using ThemedElements = std::map<std::string, ThemedElementData>;

	public:
		ThemeData() = delete;
		ThemeData(const std::string& displayName, std::unique_ptr<ThemeImages>& adoptThemeImages, std::unique_ptr<ThemedElements>& adoptThemedElements);

	public:
		const std::string& GetDisplayName() const;
		const ThemeImage& GetImage(const std::string& name) const;
		const ThemedElementData& GetThemedElement(const std::string& name) const;

	private:
		const std::string _folderName;
		const std::string _displayName;
		std::unique_ptr<ThemeImages> _images;
		std::unique_ptr<ThemedElements> _themedElements;
	};
}
