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

#include "Components/PlaylistIcons.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/treectrl.h>

#include <functional>
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
		class SongTreeItemData;

		class PlaylistBase : public wxTreeCtrl
		{
		protected:
			PlaylistBase() = delete;
			PlaylistBase(wxPanel& parent, const PlaylistIcons& playlistIcons, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);

			~PlaylistBase() override = default;

		protected:
			void SetItemStrikethrough(const wxTreeItemId& target, bool strikethrough = true);

			void ForceItemIcon(const wxTreeItemId& target, int iconIndex);
			void RestoreItemIcon(const SongTreeItemData& targetData);

		private:
			PlaylistIcons _playlistIcons;
		};
	}
}
