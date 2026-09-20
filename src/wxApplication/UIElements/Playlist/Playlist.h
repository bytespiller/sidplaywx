/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include "Components/PlaylistModel.h"
#include "../../Config/AppSettings.h"
#include <wx/dataview.h>

#include <memory>
#include <string>
#include <vector>

namespace UIElements
{
	namespace Playlist
	{
		/// @brief Mirrors AddMainSong()'s parameters; used by the bulk AddMainSongs() below.
		struct MainSongData
		{
			wxString title;
			wxString filepath;
			int defaultSubsong = 0;
			uint_least32_t duration = 0;
			wxString hvscPath;
			std::string md5;
			wxString author;
			wxString copyright;
			PlaylistTreeModelNode::RomRequirement romRequirement = PlaylistTreeModelNode::RomRequirement::None;
			bool playable = false;
			wxString musCompanionStrFilePath;
		};

		/// @brief One parent's worth of subsongs to add; used by the bulk AddSubsongsBatch() below.
		struct SubsongBatchEntry
		{
			PlaylistTreeModelNode* parent = nullptr;
			std::vector<uint_least32_t> durations;
			std::vector<wxString> titles;
		};

		class Playlist : public wxDataViewCtrl
		{
		public:
			Playlist() = delete;
			Playlist(wxPanel* parent, const UIElements::Playlist::PlaylistIcons& playlistIcons, Settings::AppSettings& appSettings, unsigned long style);

			~Playlist() override = default;

		public:
			PlaylistTreeModelNode& AddMainSong(const wxString& title, const wxString& filepath, int defaultSubsong, uint_least32_t duration, const wxString& hvscPath, const char* md5, const wxString& author, const wxString& copyright, PlaylistTreeModelNode::RomRequirement romRequirement, bool playable, const wxString& musCompanionStrFilePath = wxEmptyString);

			/// @brief Efficiently adds multiple main (top-level) songs at once via a single model-change notification for the whole batch.
			/// @remark Critical for bulk-loading large playlists: repeated AddMainSong() calls are each O(current top-level song count) - wxDataViewCtrl (at least on wxGTK) recomputes the new item's sibling position from scratch on every single ItemAdded notification, making N sequential top-level insertions O(N^2) overall. Batching into one ItemsAdded() call avoids that.
			std::vector<PlaylistTreeModelNode*> AddMainSongs(const std::vector<MainSongData>& songs);

			/// @brief Efficiently adds multiple subsongs at once.
			void AddSubsongs(const std::vector<uint_least32_t>& durations, const std::vector<wxString>& titles, PlaylistTreeModelNode& parent);

			/// @brief Adds subsongs for potentially many different parent songs at once, via a single Before/AfterReset cycle for the whole batch.
			/// @remark Critical for bulk-loading large playlists: on wxGTK, PlaylistTreeModel::PrepareDirty's Before/AfterReset bracket (which AddSubsongs() relies on for GTK stability) fully detaches and reattaches the underlying GtkTreeView model - an operation whose cost scales with the *entire* current playlist size, not just the subsongs being added. Calling AddSubsongs() once per song is therefore fine for a handful of songs, but becomes catastrophically slow (one full-list reset per multi-subsong song) once thousands of such songs already share a large playlist. Batching many parents' subsongs into one reset here avoids that.
			void AddSubsongsBatch(const std::vector<SubsongBatchEntry>& batch);

			/// @brief Removes a main song or a subsong item.
			void Remove(PlaylistTreeModelNode* item);

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

			/// @brief Returns a song (or nullptr) by its position (order) in the playlist.
			PlaylistTreeModelNode* GetSongAtPlaylistPosition(unsigned int position) const;

			/// @brief Returns a subsong (the parameter can also be a subsong). Returns an effective default subsong by default.
			PlaylistTreeModelNode& GetSubsong(PlaylistTreeModelNode& song, int subsong = 0) const;

			/// @brief Returns the currently active song or subsong item or nullptr.
			PlaylistTreeModelNode* GetActiveSong() const;

			/// @brief Returns the next playable song (a default or first subsong depending on the user setting) or nullptr.
			PlaylistTreeModelNode* GetNextSong() const;

			/// @brief Returns the next playable song (a default or first subsong depending on the user setting) or nullptr after the specified one.
			PlaylistTreeModelNode* GetNextSong(PlaylistTreeModelNode& fromSong) const;

			/// @brief Returns the previous playable song (a default or first subsong depending on the user setting) or nullptr.
			PlaylistTreeModelNode* GetPrevSong() const;

			/// @brief Returns the previous playable song (a default or first subsong depending on the user setting) or nullptr before the specified one.
			PlaylistTreeModelNode* GetPrevSong(PlaylistTreeModelNode& fromSong) const;

			/// @brief Returns the next playable subsong item (or nullptr) for the currently active song.
			PlaylistTreeModelNode* GetNextSubsong() const;

			/// @brief Returns the next playable subsong item (or nullptr) for the specified active song.
			PlaylistTreeModelNode* GetNextSubsong(PlaylistTreeModelNode& fromSubsong) const;

			/// @brief Returns the previous playable subsong item (or nullptr) for the currently active song.
			PlaylistTreeModelNode* GetPrevSubsong() const;

			/// @brief Returns the previous playable subsong item (or nullptr) for the specified active song.
			PlaylistTreeModelNode* GetPrevSubsong(PlaylistTreeModelNode& fromSubsong) const;

			/// @brief Gets the main song node playlist position (parameter can also be a subsong). Returns -1 if not found.
			int GetMainSongPlaylistPosition(PlaylistTreeModelNode& song) const;

			/// @brief Sets the node as currently playing (sub)song if playable. Returns true if successful.
			bool TrySetActiveSong(const PlaylistTreeModelNode& node, bool autoexpand);

			/// @brief Returns true if there aren't any top-level items.
			bool IsEmpty() const;

			/// @brief Applies the tag to the node with corresponding functional and visual changes. Ignores unplayable nodes by default unless forced.
			void SetItemTag(PlaylistTreeModelNode& node, PlaylistTreeModelNode::ItemTag tag, bool force = false);

			/// @brief Soft-selects (highlights) a node in the tree.
			bool Select(const PlaylistTreeModelNode& node);

			/// @brief Scrolls a node into view.
			bool EnsureVisible(const PlaylistTreeModelNode& node);

			/// @brief Convenience getter for use with wx bindings such as sizers and events.
			wxWindow* GetWxWindow();

			/// @brief Use this for auto-fitting the **text** columns, since the GetBestColumnWidth is unavailable on Linux (wxGTK).
			void AutoFitTextColumn(PlaylistTreeModel::ColumnId column);

			/// @brief Shuffles the main songs.
			void Shuffle();

		private:
			/// @brief There is no GetBestColumnWidth on Linux for some reason, so we've rolled our own here that should work everywhere for text columns at least.
			int _GetBestTextColumnWidth(PlaylistTreeModel::ColumnId column);

			wxDataViewColumn* _AddBitmapColumn(PlaylistTreeModel::ColumnId column, wxAlignment align = wxALIGN_CENTER, int flags = 0);

			// Reminder: wxCOL_REORDERABLE is crashy due to use of OnColumnsCountChanged().
			wxDataViewColumn* _AddTextColumn(PlaylistTreeModel::ColumnId column, const wxString& title, wxAlignment align = wxALIGN_LEFT, int flags = wxCOL_RESIZABLE);

			void _SortByColumn(wxDataViewColumn& viewColumn);
			void _ResetColumnSortingIndicator();

			/// @brief Increases the global item UID counter and returns the new value.
			inline unsigned int _NextFreeItemUid()
			{
				return ++_lastFreeItemUid;
			}

#ifdef WIN32
			void _OverrideScrollWheel(wxMouseEvent& evt);
#endif
			inline void _ClearTooltip()
			{
				_lastTooltipItem.Unset();
				UnsetToolTip();
				SetCursor(wxCURSOR_DEFAULT);
			}

		private:
			unsigned int _lastFreeItemUid = 0;
			PlaylistTreeModel& _model;
			Settings::AppSettings& _appSettings;
			wxDataViewItem _activeItem;
			wxDataViewItem _lastTooltipItem;

			struct ColumnSortState {
				PlaylistTreeModel::ColumnId columnId = PlaylistTreeModel::ColumnId::Undefined;
				bool ascending = true;
			} _columnSortState;
		};
	}
}
