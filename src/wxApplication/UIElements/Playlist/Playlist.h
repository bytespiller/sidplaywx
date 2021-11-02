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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "PlaylistBase.h"
#include "Components/SongTreeItemData.h"
#include "../../Config/AppSettings.h"

namespace Settings
{
	class Options;
}

namespace UIElements
{
	namespace Playlist
	{
		class Playlist : private PlaylistBase
		{
		public:
			enum class ItemStyle
			{
				Normal,
				MissingBasic,
				MissingKernal
			};

		public:
			Playlist(wxPanel& parent, const PlaylistIcons& playlistIcons, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, Settings::AppSettings& appSettings);
			~Playlist() override = default;

		public:
			PlaylistBase& GetBase();
			wxTreeItemId GetRootItem() const;
			void SelectItem(const wxTreeItemId& item);
			void EnsureVisible(const wxTreeItemId& item);

			bool IsEmpty() const;
			bool IsPlayableItem(const SongTreeItemData& songData) const;

			wxTreeItemId AppendMainSong(SongTreeItemData* itemData, int totalSubsongs, ItemStyle itemStyle);
			wxTreeItemId AppendSubSong(const wxTreeItemId& parent, SongTreeItemData* itemData, ItemStyle itemStyle);

			void IgnoreSong(SongTreeItemData& songData, const int iconId = PlaylistIcons::ICON_ID_NO_ICON);
			void IgnoreSong(const wxTreeItemId& songId, const int iconId = PlaylistIcons::ICON_ID_NO_ICON);

			void RestoreIgnoredSong(SongTreeItemData& songData);
			void RestoreIgnoredSong(const wxTreeItemId& songId);

			const SongTreeItemData& GetParentSong(const SongTreeItemData& item);

			const SongTreeItemData* FindPlaylistSubsongItem(const wxTreeItemId& parent, int subsong);

			void ClearPlaylist();

			const SongTreeItemData* GetNextSong(const SongTreeItemData* const startSong = nullptr);
			const SongTreeItemData* GetPrevSong(const SongTreeItemData* const startSong = nullptr);

			const SongTreeItemData* GetNextSubsong();
			const SongTreeItemData* GetPrevSubsong();

			const SongTreeItemData* GetEffectiveDefaultOrFirstPlayableSubsong(const SongTreeItemData& mainSongItemData);
			const SongTreeItemData* GetEffectiveDefaultOrFirstPlayableSubsong();

			bool TryHighlightPlaylistItem(const wxTreeItemId& songItem, int subsong, bool autoexpand);
			const SongTreeItemData* TryGetCurrentSongTreeItemData() const;

			const SongTreeItemData* TryGetContextMenuSongTreeItemData() const;

			inline SongTreeItemData& GetSongTreeItemData(const wxTreeItemId& target)
			{
				assert(target.IsOk());
				return *static_cast<SongTreeItemData*>(GetItemData(target));
			}

		public:
			using ForEachPredicate = std::function<void(SongTreeItemData&)>;
			void ForEachSibling(const ForEachPredicate& predicate, const wxTreeItemId& idParent, const wxTreeItemId& startSiblingId = wxTreeItemId());

			using FindIfPredicate = std::function<bool(SongTreeItemData&)>;
			const SongTreeItemData* FindSiblingIf(const FindIfPredicate& predicate, const wxTreeItemId& idParent, const wxTreeItemId& startSiblingId = wxTreeItemId());

		public:
			void OnContextMenuOpen(wxContextMenuEvent& evt);

		private:
			inline void ApplySpecialItemStyle(const wxTreeItemId& target, ItemStyle itemStyle)
			{
				SetItemStrikethrough(target, true);
				const char* textColorUnplayable = (itemStyle == ItemStyle::MissingBasic) ? "#054a80" : "#8a5454";
				SetItemTextColour(target, textColorUnplayable);
			}

			wxString emptyWxString;
			inline const SongTreeItemData* TryGetAppropriateParentSong(const SongTreeItemData* const song = nullptr)
			{
				if (song == nullptr)
				{
					const SongTreeItemData* const songData = TryGetCurrentSongTreeItemData();
					return (songData == nullptr) ? nullptr : &GetParentSong(*songData);
				}
				else
				{
					return &GetParentSong(*song);
				}
			}

		private:
			Settings::AppSettings& _appSettings;
			const SongTreeItemData* _highlightedPlaylistTreeItem = nullptr;
			wxTreeItemId _contextMenuItem;
			bool _debugCheckRefreshedContextMenuItem = false;
		};
	}
}
