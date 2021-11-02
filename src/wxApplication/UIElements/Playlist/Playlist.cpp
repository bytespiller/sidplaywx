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

#include "Playlist.h"
#include "Components/PlaylistIcons.h"
#include "../../Config/UIStrings.h"
#include "../../Helpers/HelpersWx.h"

namespace UIElements
{
	namespace Playlist
	{
		Playlist::Playlist(wxPanel& parent, const PlaylistIcons& playlistIcons, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, Settings::AppSettings& appSettings) :
			PlaylistBase(parent, playlistIcons, id, pos, size, style),
			_appSettings(appSettings)
		{
		}

		PlaylistBase& Playlist::GetBase()
		{
			return *this;
		}

		wxTreeItemId Playlist::GetRootItem() const
		{
			return PlaylistBase::GetRootItem();
		}

		void Playlist::SelectItem(const wxTreeItemId& item)
		{
			PlaylistBase::SelectItem(item);
		}

		void Playlist::EnsureVisible(const wxTreeItemId& item)
		{
			PlaylistBase::EnsureVisible(item);
		}

		bool Playlist::IsEmpty() const
		{
			return !ItemHasChildren(PlaylistBase::GetRootItem());
		}

		bool Playlist::IsPlayableItem(const SongTreeItemData& songData) const
		{
			return songData.GetStatus() != SongTreeItemData::ItemStatus::Unplayable && (songData.GetType() == SongTreeItemData::SongItemType::Song || songData.GetType() == SongTreeItemData::SongItemType::Subsong);
		}

		wxTreeItemId Playlist::AppendMainSong(SongTreeItemData* itemData, int totalSubsongs, ItemStyle itemStyle)
		{
			assert(!itemData->IsSubsong());
			wxString durationStr = (totalSubsongs == 1) ? " - " + Helpers::Wx::GetTimeFormattedString(itemData->GetDuration(), true) : wxString::Format(" (%i)", totalSubsongs);
			const int playlistIconIndex = itemData->GetCanonicalIconIndex();
			wxTreeItemId itemId = AppendItem(GetRootItem(), wxString::Format("%s%s", itemData->GetTitle(), durationStr), playlistIconIndex, playlistIconIndex, itemData);
			if (itemStyle != ItemStyle::Normal)
			{
				ApplySpecialItemStyle(itemId, itemStyle);
			}
			return itemId;
		}

		wxTreeItemId Playlist::AppendSubSong(const wxTreeItemId& parent, SongTreeItemData* itemData, ItemStyle itemStyle)
		{
			assert(itemData->IsSubsong());
			const wxString& durationStr = Helpers::Wx::GetTimeFormattedString(itemData->GetDuration(), true);
			wxTreeItemId itemId = AppendItem(parent, wxString::Format("%s: %s %i - %s", itemData->GetTitle(), Strings::PlaylistTree::SUBSONG, itemData->GetDefaultSubsong(), durationStr), itemData->GetCanonicalIconIndex(), PlaylistIcons::ICON_ID_NO_ICON, itemData);
			if (itemStyle != ItemStyle::Normal)
			{
				ApplySpecialItemStyle(itemId, itemStyle);
			}
			return itemId;
		}

		void Playlist::IgnoreSong(SongTreeItemData& songData, const int iconId)
		{
			if (songData.GetStatus() == SongTreeItemData::ItemStatus::Normal)
			{
				songData.ChangeStatus(SongTreeItemData::ItemStatus::IgnoredPlayable);

				if (iconId != PlaylistIcons::ICON_ID_NO_ICON)
				{
					ForceItemIcon(songData.GetId(), iconId);
				}
			}
		}

		void Playlist::IgnoreSong(const wxTreeItemId& songId, const int iconId)
		{
			IgnoreSong(GetSongTreeItemData(songId), iconId);
		}

		void Playlist::RestoreIgnoredSong(SongTreeItemData& songData)
		{
			if (songData.GetStatus() == SongTreeItemData::ItemStatus::IgnoredPlayable)
			{
				songData.ChangeStatus(SongTreeItemData::ItemStatus::Normal);
				RestoreItemIcon(songData);
			}
		}

		void Playlist::RestoreIgnoredSong(const wxTreeItemId& songId)
		{
			RestoreIgnoredSong(GetSongTreeItemData(songId));
		}

		const SongTreeItemData& Playlist::GetParentSong(const SongTreeItemData& item)
		{
			const wxTreeItemId& root = PlaylistBase::GetRootItem();
			assert(item.GetId() != root.GetID());

			const wxTreeItemId& parent = GetItemParent(item.GetId());
			return (parent.GetID() == root.GetID()) ? item : GetSongTreeItemData(parent);
		}

		const SongTreeItemData* Playlist::FindPlaylistSubsongItem(const wxTreeItemId& parent, int subsong)
		{
			wxTreeItemIdValue cookie;
			wxTreeItemId item = GetFirstChild(parent, cookie);

			while (item.IsOk())
			{
				const SongTreeItemData& subsongItemData = GetSongTreeItemData(item);
				if (subsongItemData.IsSubsong() && subsongItemData.GetDefaultSubsong() == subsong)
				{
					return &subsongItemData;
				}
				item = GetNextSibling(item);
			}

			return nullptr;
		}

		void Playlist::ClearPlaylist()
		{
			_highlightedPlaylistTreeItem = nullptr;
			DeleteChildren(GetRootItem());
		}

		const SongTreeItemData* Playlist::GetNextSong(const SongTreeItemData* const startSong)
		{
			if (const SongTreeItemData* const parentItemData = TryGetAppropriateParentSong(startSong))
			{
				const SongTreeItemData* const retSongData = FindSiblingIf([](const SongTreeItemData& cSongData)
				{
					if (cSongData.GetType() == SongTreeItemData::SongItemType::Song && cSongData.GetStatus() == SongTreeItemData::ItemStatus::Normal)
					{
						return true;
					}
					return false;
				}, GetRootItem(), parentItemData->GetId());

				if (retSongData != nullptr)
				{
					return GetEffectiveDefaultOrFirstPlayableSubsong(*retSongData);
				}
			}

			return nullptr;
		}

		const SongTreeItemData* Playlist::GetPrevSong(const SongTreeItemData* const startSong)
		{
			const SongTreeItemData* lastGoodPrevSong = nullptr;
			if (const SongTreeItemData* const parentItemData = TryGetAppropriateParentSong(startSong))
			{
				FindSiblingIf([&parentItemData, &lastGoodPrevSong](const SongTreeItemData& cSongData)
				{
					if (cSongData.GetId() == parentItemData->GetId())
					{
						return true; // Just break the iteration early.
					}

					if (cSongData.GetType() == SongTreeItemData::SongItemType::Song && cSongData.GetStatus() == SongTreeItemData::ItemStatus::Normal)
					{
						lastGoodPrevSong = &cSongData;
					}

					return false;
				}, GetRootItem());
			}

			if (lastGoodPrevSong != nullptr)
			{
				return GetEffectiveDefaultOrFirstPlayableSubsong(*lastGoodPrevSong);
			}

			return nullptr;
		}

		const SongTreeItemData* Playlist::GetNextSubsong()
		{
			if (const SongTreeItemData* const activeSongData = TryGetCurrentSongTreeItemData())
			{
				const int cSubsongIndex = activeSongData->GetDefaultSubsong();
				const wxTreeItemId& parentItemId = GetParentSong(*activeSongData).GetId();

				const SongTreeItemData* const retSongData = FindSiblingIf([cSubsongIndex](const SongTreeItemData& cSongData)
				{
					return cSongData.IsSubsong() && cSongData.GetStatus() == SongTreeItemData::ItemStatus::Normal && cSongData.GetDefaultSubsong() > cSubsongIndex;
				}, parentItemId);

				if (retSongData != nullptr)
				{
					return retSongData;
				}
			}

			return nullptr;
		}

		const SongTreeItemData* Playlist::GetPrevSubsong()
		{
			if (const SongTreeItemData* const activeSongData = TryGetCurrentSongTreeItemData())
			{
				const int cSubsongIndex = activeSongData->GetDefaultSubsong();
				const wxTreeItemId& parentItemId = GetParentSong(*activeSongData).GetId();
				const SongTreeItemData* lastGoodPrevSubsong = nullptr;

				FindSiblingIf([cSubsongIndex, &lastGoodPrevSubsong](const SongTreeItemData& cSongData)
				{
					if (cSongData.IsSubsong() && cSongData.GetStatus() == SongTreeItemData::ItemStatus::Normal)
					{
						if (cSongData.GetDefaultSubsong() == cSubsongIndex)
						{
							return true; // Just break the iteration early.
						}

						lastGoodPrevSubsong = &cSongData;
					}

					return false;
				}, parentItemId);

				if (lastGoodPrevSubsong != nullptr)
				{
					return lastGoodPrevSubsong;
				}
			}

			return nullptr;
		}

		const SongTreeItemData* Playlist::GetEffectiveDefaultOrFirstPlayableSubsong(const SongTreeItemData& mainSongItemData)
		{
			assert(!mainSongItemData.IsSubsong());

			const SongTreeItemData* preferredStartSubsongData = nullptr;

			{
				const int preferredStartSubsong = (_appSettings.GetOption(Settings::AppSettings::ID::RepeatModeDefaultSubsong)->GetValueAsBool()) ? mainSongItemData.GetDefaultSubsong() : 1;
				preferredStartSubsongData = FindPlaylistSubsongItem(mainSongItemData.GetId(), preferredStartSubsong);
			}

			if (preferredStartSubsongData != nullptr && preferredStartSubsongData->GetStatus() == SongTreeItemData::ItemStatus::Normal)
			{
				return preferredStartSubsongData;
			}
			else
			{
				const SongTreeItemData* const retSongData = FindSiblingIf([](const SongTreeItemData& cSongData)
				{
					return cSongData.IsSubsong() && cSongData.GetStatus() == SongTreeItemData::ItemStatus::Normal;
				}, mainSongItemData.GetId());

				if (retSongData != nullptr)
				{
					return retSongData;
				}
			}

			return &mainSongItemData;
		}

		const SongTreeItemData* Playlist::GetEffectiveDefaultOrFirstPlayableSubsong()
		{
			const SongTreeItemData* const parentItemData = TryGetAppropriateParentSong(nullptr);
			if (parentItemData != nullptr)
			{
				return GetEffectiveDefaultOrFirstPlayableSubsong(*parentItemData);
			}

			return nullptr;
		}

		bool Playlist::TryHighlightPlaylistItem(const wxTreeItemId& songItem, int subsong, bool autoexpand)
		{
			if (TryGetCurrentSongTreeItemData() != nullptr)
			{
				SetItemBold(_highlightedPlaylistTreeItem->GetId(), false);
				SetItemBold(GetItemParent(_highlightedPlaylistTreeItem->GetId()), false); // Main song if this is a subsong, or Root if not (which is invisible anyway).
			}

			const SongTreeItemData* targetItem = &GetParentSong(GetSongTreeItemData(songItem));

			if (GetChildrenCount(targetItem->GetId(), false) >= subsong)
			{
				const SongTreeItemData* const targetSubItem = FindPlaylistSubsongItem(targetItem->GetId(), subsong);
				if (targetSubItem != nullptr)
				{
					SetItemBold(targetItem->GetId(), true);
					if (autoexpand)
					{
						Expand(targetItem->GetId()); // Expand parent item.
					}
					targetItem = targetSubItem; // Canonize subitem.
				}
				else
				{
					return false;
				}
			}

			if (TryGetCurrentSongTreeItemData() != nullptr)
			{
				const SongTreeItemData* const prevSelectedItemTuneData = TryGetCurrentSongTreeItemData();
				assert(prevSelectedItemTuneData != nullptr);
				if (prevSelectedItemTuneData->GetFilePath() != targetItem->GetFilePath())
				{
					if (autoexpand)
					{
						if (prevSelectedItemTuneData->IsSubsong())
						{
							Collapse(GetParentSong(*prevSelectedItemTuneData).GetId()); // Collapse parent if subtune, avoids root if not subtune.
						}
					}
					Refresh(); // Must be done here to prevent visual corruption on Collapse while also auto-Expanding.
				}
			}

			SetItemBold(targetItem->GetId(), true);
			_highlightedPlaylistTreeItem = targetItem;

			return true;
		}

		const SongTreeItemData* Playlist::TryGetCurrentSongTreeItemData() const
		{
			if (_highlightedPlaylistTreeItem != nullptr && _highlightedPlaylistTreeItem->GetId().IsOk())
			{
				return _highlightedPlaylistTreeItem;
			}

			return nullptr;
		}

		const SongTreeItemData* Playlist::TryGetContextMenuSongTreeItemData() const
		{
			assert(_debugCheckRefreshedContextMenuItem); // Don't forget to call our OnContextMenuOpen in your outside handler first!

			if (_contextMenuItem.IsOk())
			{
				return static_cast<SongTreeItemData*>(GetItemData(_contextMenuItem));
			}

			return nullptr;
		}

		void Playlist::ForEachSibling(const ForEachPredicate& predicate, const wxTreeItemId& idParent, const wxTreeItemId& startSiblingId)
		{
			wxTreeItemIdValue cookie;
			wxTreeItemId id = (startSiblingId.IsOk()) ? GetNextSibling(startSiblingId) : GetFirstChild(idParent, cookie);

			while (id.IsOk())
			{
				predicate(GetSongTreeItemData(id));
				id = GetNextSibling(id);
			}
		}

		const SongTreeItemData* Playlist::FindSiblingIf(const FindIfPredicate& predicate, const wxTreeItemId& idParent, const wxTreeItemId& startSiblingId)
		{
			wxTreeItemIdValue cookie;
			wxTreeItemId id = (startSiblingId.IsOk()) ? GetNextSibling(startSiblingId) : GetFirstChild(idParent, cookie);

			while (id.IsOk())
			{
				SongTreeItemData& data = GetSongTreeItemData(id);
				if (predicate(data))
				{
					return &data;
				}
				else
				{
					id = GetNextSibling(id);
				}
			}

			return nullptr;
		}

		void Playlist::OnContextMenuOpen(wxContextMenuEvent& evt)
		{
			_debugCheckRefreshedContextMenuItem = true;
			_contextMenuItem = HitTest(ScreenToClient(evt.GetPosition()));
			if (!_contextMenuItem.IsOk())
			{
				_contextMenuItem = GetFocusedItem(); // In case the context menu wasn't opened using the mouse.
			}
		}
	}
}
