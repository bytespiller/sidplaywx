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

#include "PlaylistBase.h"
#include "Components/PlaylistIcons.h"
#include "Components/SongTreeItemData.h"

namespace UIElements
{
	namespace Playlist
	{
		static constexpr const char* const ROOT_NAME = "Root";

		PlaylistBase::PlaylistBase(wxPanel& parent, const PlaylistIcons& playlistIcons, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
			wxTreeCtrl(&parent, id, pos, size, style),
			_playlistIcons(playlistIcons)
		{
			AddRoot(ROOT_NAME);

			// PlaylistBase icons
			const wxSize& iconSize = _playlistIcons.GetIconSize();
			wxImageList* images = new wxImageList(iconSize.GetWidth(), iconSize.GetHeight(), false);
			for (const auto& icon : _playlistIcons.GetIconList())
			{
				images->Add(*icon.second.get());
			}
			AssignImageList(images);
		}

		void PlaylistBase::SetItemStrikethrough(const wxTreeItemId& target, bool strikethrough)
		{
			wxFont font = GetFont(); // Must get default (global) window font, item's font would be too small.
			font.SetStrikethrough(strikethrough);
			SetItemFont(target, font);
		}

		void PlaylistBase::ForceItemIcon(const wxTreeItemId& target, int iconIndex)
		{
			SetItemImage(target, iconIndex, wxTreeItemIcon::wxTreeItemIcon_Normal);
			SetItemImage(target, iconIndex, wxTreeItemIcon::wxTreeItemIcon_Selected);
		}

		void PlaylistBase::RestoreItemIcon(const SongTreeItemData& targetData)
		{
			SetItemImage(targetData.GetId(), targetData.GetCanonicalIconIndex(), wxTreeItemIcon::wxTreeItemIcon_Normal);
			SetItemImage(targetData.GetId(), targetData.GetCanonicalIconIndex(), wxTreeItemIcon::wxTreeItemIcon_Selected);
		}
	}
}
