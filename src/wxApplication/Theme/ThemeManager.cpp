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

#include "ThemeManager.h"
#include "ThemeLoader.h"
#include "ThemeData/ThemeData.h"
#include <stdexcept>

static const std::string THEMES_FOLDER = "theme/";

void ThemeManager::LoadTheme(const std::string& foldername)
{
	if (_loadedThemes.count(foldername) != 0)
	{
		throw std::runtime_error("Theme already loaded!");
	}

	_loadedThemes.emplace(foldername, ThemeLoader::LoadTheme(THEMES_FOLDER + foldername));
}

const ThemeData::ThemeData& ThemeManager::GetTheme(const std::string& foldername)
{
	if (_loadedThemes.count(foldername) != 1)
	{
		throw std::runtime_error("Theme wasn't loaded before!");
	}

	return _loadedThemes.at(foldername);
}
