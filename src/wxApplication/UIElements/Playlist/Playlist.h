/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "Components\PlaylistModel.h"
#include "..\..\Config\AppSettings.h"
#include <wx/dataview.h>

#include <memory>
#include <vector>

namespace UIElements
{
	namespace Playlist
	{
		class Playlist : public wxDataViewCtrl
		{
		public:
			Playlist() = delete;
			Playlist(wxPanel* parent, const UIElements::Playlist::PlaylistIcons& playlistIcons, Settings::AppSettings& appSettings, unsigned long style);

			~Playlist() override = default;

		public:
			PlaylistTreeModelNode& AddMainSong(const wxString& title, const wxString& filepath, int defaultSubsong, uint_least32_t duration, const wxString& author, const wxString& copyright, PlaylistTreeModelNode::RomRequirement romRequirement, bool playable);

			/// @brief Efficiently adds multiple subsongs at once.
			void AddSubsongs(const std::vector<uint_least32_t>& durations, PlaylistTreeModelNode& parent);

			/// @brief Removes a main song or a subsong item.
			void Remove(PlaylistTreeModelNode& item);

			/// @brief Removes all items from the playlist.
			void Clear();

			/// @brief Expands the tree node.
			void ExpandSongNode(const PlaylistTreeModelNode& node);

			/// @brief Expands all top-level nodes.
			void ExpandAll();

			/// @brief Collapses all top-level nodes.
			void CollapseAll();

			/// @brief Returns song-default or first playable subsong (depending on the user setting) or a nullptr.
			PlaylistTreeModelNode* GetEffectiveInitialSubsong(const PlaylistTreeModelNode& mainSongItem) const;

			/// @brief Returns all top-level song nodes.
			const PlaylistTreeModelNodePtrArray& GetSongs() const;

			/// @brief Returns a song (or nullptr) by filepath.
			PlaylistTreeModelNode* GetSong(const wxString& filepath) const;

			/// @brief Returns a subsong (or nullptr) by filepath. Returns an effective default subsong by default.
			PlaylistTreeModelNode* GetSubsong(const wxString& filepath, int subsong = 0) const;

			/// @brief Returns the currently active song or subsong item or nullptr.
			PlaylistTreeModelNode* GetActiveSong() const;

			/// @brief Returns the next playable song (a default or first subsong depending on the user setting) or nullptr.
			PlaylistTreeModelNode* GetNextSong() const;

			/// @brief Returns the next playable song (a default or first subsong depending on the user setting) or nullptr after the specified one.
			PlaylistTreeModelNode* GetNextSong(const PlaylistTreeModelNode& fromSong) const;

			/// @brief Returns the previous playable song (a default or first subsong depending on the user setting) or nullptr.
			PlaylistTreeModelNode* GetPrevSong() const;

			/// @brief Returns the previous playable song (a default or first subsong depending on the user setting) or nullptr before the specified one.
			PlaylistTreeModelNode* GetPrevSong(const PlaylistTreeModelNode& fromSong) const;

			/// @brief Returns the next playable subsong item (or nullptr) for the currently active song.
			PlaylistTreeModelNode* GetNextSubsong() const;

			/// @brief Returns the next playable subsong item (or nullptr) for the specified active song.
			PlaylistTreeModelNode* GetNextSubsong(const PlaylistTreeModelNode& fromSubsong) const;

			/// @brief Returns the previous playable subsong item (or nullptr) for the currently active song.
			PlaylistTreeModelNode* GetPrevSubsong() const;

			/// @brief Returns the previous playable subsong item (or nullptr) for the specified active song.
			PlaylistTreeModelNode* GetPrevSubsong(const PlaylistTreeModelNode& fromSubsong) const;

			/// @brief Gets the main song node index (zero based). Returns -1 if not found.
			int GetSongIndex(const wxString& filepath) const;

			/// @brief Sets the node as currently playing (sub)song if playable. Returns true if successful.
			bool TrySetActiveSong(const PlaylistTreeModelNode& node, bool autoexpand);

			/// @brief Returns true if there aren't any top-level items.
			bool IsEmpty() const;

			/// @brief Applies the tag to the node with corresponding functional and visual changes. Ignores unplayable nodes by default unless forced.
			void SetItemTag(PlaylistTreeModelNode& node, PlaylistTreeModelNode::ItemTag tag, bool force = false); // TODO: consider renaming to SetItemStatus (and "tag" concept to "status" concept)?

			/// @brief Soft-selects (highlights) a node in the tree.
			bool Select(const PlaylistTreeModelNode& node);

			/// @brief Scrolls a node into view.
			bool EnsureVisible(const PlaylistTreeModelNode& node);

			/// @brief Convenience getter for use with wx bindings such as sizers and events.
			wxWindow* GetWxWindow();

		private:
			wxDataViewColumn* _AddBitmapColumn(PlaylistTreeModel::ColumnId columnIndex, wxAlignment align = wxALIGN_CENTER, int flags = 0);

			// Reminder 1: wxCOL_SORTABLE would mess up the navigation since the wxDataViewCtrl is feature-incomplete (it sorts visually only and its tree path is inaccessible). Using the wxEVT_DATAVIEW_COLUMN_SORTED to manually sort the model entries proved to be problematic so I gave up for now.
			// Reminder 2: wxCOL_REORDERABLE is crashy due to use of OnColumnsCountChanged().
			wxDataViewColumn* _AddTextColumn(PlaylistTreeModel::ColumnId columnIndex, const wxString& title, wxAlignment align = wxALIGN_LEFT, int flags = wxCOL_RESIZABLE);

			void _OverrideScrollWheel(wxMouseEvent& evt);

		private:
			PlaylistTreeModel& _model;
			Settings::AppSettings& _appSettings;
			wxDataViewItem _activeItem;
		};
	}
}
