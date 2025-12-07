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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <map>
#include <memory>

namespace ThemeData
{
	struct ThemeImage;
}

namespace UIElements
{
	namespace Playlist
	{
		enum class PlaylistIconId : int
		{
			// Playlist items status indicators
			NoIcon = -1,
			DefaultSubsongIndicator,
			ChipIcon,
			SkipShort,
			RemoveSong,
			MusOrStr, // indicates a companion STR file for MUS
			MusAndStr, // indicates a MUS+STR (instead of DefaultSubsongIndicator)

			// Column sorting indicators
			SortAscending,
			SortDescending
		};

		struct Icon
		{
			Icon() = delete;
			Icon(std::shared_ptr<wxBitmapBundle> aBitmap, const wxString& aTooltip) :
				bitmap(std::move(aBitmap)),
				tooltip(aTooltip)
			{
			}

			std::shared_ptr<wxBitmapBundle> bitmap;
			wxString tooltip;
		};

		class PlaylistIcons
		{
		public:
			using AssignedIconList = std::map<PlaylistIconId, Icon>; // Reminder: don't use unsorted_map here!

		public:
			PlaylistIcons() = delete;
			explicit PlaylistIcons(int commonIconSize);

		public:
			void RegisterSvgIcon(PlaylistIconId iconId, const ThemeData::ThemeImage& themeImage, const wxString& tooltip = wxEmptyString);
			const AssignedIconList& GetIconList() const;
			const wxSize& GetIconSize() const;

		private:
			wxSize _iconSize;
			AssignedIconList _assignedIcons;
		};
	}
}
