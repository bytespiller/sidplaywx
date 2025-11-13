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

#include "PlaylistIcons.h"
#include "../../ElementsUtil.h"
#include "../../../Theme/ThemeData/ThemeImage.h"

namespace UIElements
{
	namespace Playlist
	{
		PlaylistIcons::PlaylistIcons(int commonIconSize) :
			_iconSize(commonIconSize, commonIconSize)
		{
		}

		void PlaylistIcons::RegisterSvgIcon(UIElements::Playlist::PlaylistIconId iconId, const ThemeData::ThemeImage& themeImage, const wxString& tooltip)
		{
			assert(iconId > UIElements::Playlist::PlaylistIconId::NoIcon);
			_assignedIcons.emplace(iconId, Icon(UIElements::Util::LoadColorizedSvg(themeImage.path.c_str(), _iconSize), tooltip));
		}

		const PlaylistIcons::AssignedIconList& PlaylistIcons::GetIconList() const
		{
			return _assignedIcons;
		}

		const wxSize& PlaylistIcons::GetIconSize() const
		{
			return _iconSize;
		}
	}
}
