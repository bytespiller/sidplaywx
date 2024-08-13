/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "../Theme/ThemeData/ThemeData.h"
#include "../UIElements/CompositeSeekBar.h"
#include "../UIElements/PlayPauseButton.h"
#include "../UIElements/RepeatModeButton.h"
#include "../UIElements/ScrollingLabel.h"
#include "../UIElements/SearchBar.h"
#include "../UIElements/WaveformVisualization.h"
#include "../UIElements/Playlist/Playlist.h"

namespace Settings
{
	class Options;
}

namespace FrameElements
{
	enum class PlaylistContextMenuIconId : int
	{
		RemoveMainSong = 0,
		RemoveAllAbove,
		RemoveAllBelow,
		ExpandAll,
		CollapseAll,
		ScrollToCurrent,
		SkipSubsong
	};

	class ElementsPlayer
	{
	public:
		enum class PopupMenuItemId_VolumeSlider : int
		{
			ResetVolume = 0,
			DisableControl,
			EnableControl
		};

		enum class MenuItemId_Player : int
		{
			// File
			OpenFiles,
			OpenFolders,
			// ----------------
			EnqueueFiles,
			EnqueueFolders,
			// Playlist submenu
			PlaylistOpen,
			PlaylistSave,
			PlaylistClear,
			// ----------------
			Exit,

			// Edit
			PlaybackMods,
			Preferences,
			Find,
			FindNext,
			FindPrev,

			// View
			StayTopmost,
			VisualizationEnabled,
			StilInfoEnabled,

			// Help
			CheckUpdates,
			About,
		};

	public:
		ElementsPlayer() = delete;
		ElementsPlayer(const ElementsPlayer&) = delete;
		ElementsPlayer& operator=(const ElementsPlayer&) = delete;

	    explicit ElementsPlayer(wxPanel& panel, Settings::AppSettings& appSettings, const ThemeData::ThemeData& themeData);

	public:
		void EnablePlaybackControls(bool enabled);

	private: // Event handlers
		void OnVolumeSliderContextMenuOpen(wxContextMenuEvent& evt);
		void OnVolumeSliderPopupMenuItemClick(wxCommandEvent& evt);

		void OnPanelContextMenuOpen(wxContextMenuEvent& evt);

	public:
		// Menu
		wxMenuBar* menuBar;

		// Labels
		wxStaticText* labelTitle;
		wxStaticText* labelAuthor;
		wxStaticText* labelCopyright;
		wxStaticText* labelSubsong;
		wxStaticText* labelTime;

		// Always enabled
		UIElements::RepeatModeButton* btnRepeatMode;
	    wxSlider* sliderVolume;
		UIElements::WaveformVisualization* waveformVisualization;

		UIElements::ScrollingLabel* labelStilNameTitle; // NAME - TITLE
		UIElements::ScrollingLabel* labelStilArtistAuthor; // ARTIST (AUTHOR)
		UIElements::ScrollingLabel* labelStilComment;

		// Playback controls
		wxButton* btnPrevTrack;
		wxButton* btnPrevSubsong;
		wxButton* btnStop;
		UIElements::PlayPauseButton* btnPlayPause;
		wxButton* btnNextSubsong;
		wxButton* btnNextTrack;
		wxButton* btnPlaybackMod;

		// Other disableable
	    UIElements::CompositeSeekBar* compositeSeekbar;
	    UIElements::Playlist::Playlist* treePlaylist;
		UIElements::SearchBar* searchBar;

		// Context menu icons
		std::unordered_map<PlaylistContextMenuIconId, std::shared_ptr<wxBitmap>> playlistContextMenuIcons;

	private:
		wxPanel& _parentPanel;
	};
}
