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

#include "ElementsPlayer.h"
#include "../Config/AppSettings.h"
#include "../Config/UIStrings.h"
#include "../Helpers/DpiSize.h"
#include "../UIElements/ElementsUtil.h"
#include "../Theme/ThemeData/ThemeImage.h"

namespace FrameElements // Static vars
{
	// TODO: these TEMP_* things could be defined in the theme XML?
	static constexpr int TEMP_BUTTON_SIZE = 30;
	static constexpr int TEMP_BUTTON_BORDER_SIZE = 1;
	static constexpr int TEMP_LABEL_BORDER_SIZE = 2;
	static constexpr int TEMP_LABEL_TIME_BORDER_SIZE = 2;
	static constexpr int TEMP_LABEL_ICON_SIZE = 14;
	static constexpr int TEMP_PLAYLIST_ICON_SIZE = 16;
	static constexpr int TEMP_MENU_ICON_SIZE = 16;
}

namespace FrameElements // Static functions
{
	static wxStaticText* AttachIconLabel(const ThemeData::ThemeImage& themeImage, const std::string& tooltip, wxPanel& parent, wxBoxSizer* parentSizer)
	{
		wxBoxSizer* sizerIconLabel = new wxBoxSizer(wxHORIZONTAL);
		parentSizer->Add(sizerIconLabel, 1, wxEXPAND, 0);

		const auto& img = UIElements::Util::LoadRasterizedSvg(themeImage.path.c_str(), DpiSize(TEMP_LABEL_ICON_SIZE), themeImage.offset, themeImage.scale);
		wxStaticBitmap* bitmapIcon = new wxStaticBitmap(&parent, wxID_ANY, *img.get());
		sizerIconLabel->Add(bitmapIcon, 0, wxEXPAND | wxALL, TEMP_LABEL_BORDER_SIZE);
		bitmapIcon->SetToolTip(tooltip);

		wxStaticText* labelPtr = new wxStaticText(&parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
		sizerIconLabel->Add(labelPtr, 1, wxALIGN_CENTER_VERTICAL, TEMP_LABEL_BORDER_SIZE);

		return labelPtr;
	}

	static UIElements::ScrollingLabel* AttachStilIconScrollingLabel(const ThemeData::ThemeImage& themeImage, const ThemeData::ThemedElementData& themedData, const std::string& tooltip, wxPanel& parent, wxBoxSizer* parentSizer, UIElements::ScrollingLabel::TextJustify justify)
	{
		wxBoxSizer* sizerIconLabel = new wxBoxSizer(wxHORIZONTAL);
		parentSizer->Add(sizerIconLabel, 1, wxEXPAND, 0);

		// Create icon
		const wxColor iconColor = themedData.GetPropertyColor("iconColor");
		const std::shared_ptr<wxBitmap>& img = UIElements::Util::LoadRasterizedSvg(themeImage.path.c_str(), DpiSize(TEMP_LABEL_ICON_SIZE), themeImage.offset, themeImage.scale, &iconColor);
		wxStaticBitmap* bitmapIcon = new wxStaticBitmap(&parent, wxID_ANY, *img.get());
		bitmapIcon->SetToolTip(tooltip);

		// Create label
		UIElements::ScrollingLabel* labelPtr = new UIElements::ScrollingLabel(&parent, themedData, justify);

		// Attach & layout the elements
		wxSizerFlags::DisableConsistencyChecks(); // (wxEXPAND | wxALIGN_CENTER_VERTICAL) is effective combination but the check against it is defective. This call disables this check globally (with no way to re-enable it unfortunately).

		if (justify == UIElements::ScrollingLabel::TextJustify::Right)
		{
			sizerIconLabel->Add(labelPtr, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL, TEMP_LABEL_BORDER_SIZE);
			sizerIconLabel->Add(bitmapIcon, 0, wxEXPAND | wxALL, TEMP_LABEL_BORDER_SIZE);
		}
		else
		{
			sizerIconLabel->Add(bitmapIcon, 0, wxEXPAND | wxALL, TEMP_LABEL_BORDER_SIZE);
			sizerIconLabel->Add(labelPtr, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL, TEMP_LABEL_BORDER_SIZE);
		}

		return labelPtr;
	}

	static wxButton* AttachSimplePlaybackControlButton(const ThemeData::ThemeImage& themeImage, wxPanel& parent, wxBoxSizer* parentSizer)
	{
		wxButton* newButton = UIElements::Util::NewSvgButton(themeImage, DpiSize(TEMP_BUTTON_SIZE), parent);
		parentSizer->Add(newButton, 0, wxALL, TEMP_BUTTON_BORDER_SIZE);
		return newButton;
	}

	static void AttachFixedSizeSeparator(const wxSize& size, wxBoxSizer* parentSizer, wxPanel& parent)
	{
		wxPanel* separatorDummyPanel = new wxPanel(&parent, wxID_ANY);
		separatorDummyPanel->SetSize(size);
		separatorDummyPanel->Enable(false);
		separatorDummyPanel->Hide();
		parentSizer->Add(separatorDummyPanel, 0, wxFIXED_MINSIZE | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 0);
	}

	static std::shared_ptr<wxBitmap> LoadMenuIcon(const ThemeData::ThemeData& themeData, const std::string& themeIconName)
	{
		const ThemeData::ThemeImage& img = themeData.GetImage(themeIconName);
		return UIElements::Util::LoadRasterizedSvg(img.path.c_str(), DpiSize(TEMP_MENU_ICON_SIZE), img.offset, img.scale);
	}
}

namespace FrameElements // Player class
{
	ElementsPlayer::ElementsPlayer(wxPanel& panel, Settings::AppSettings& appSettings, const ThemeData::ThemeData& themeData) :
		_parentPanel(panel)
	{
		menuBar = new wxMenuBar();

		{
			// File menu
			{
				wxMenu* fileMenu = new wxMenu();

				fileMenu->Append(static_cast<int>(MenuItemId_Player::OpenFiles), wxString::Format("%s\tCtrl+O",Strings::FramePlayer::MENU_ITEM_OPEN_FILES));
				fileMenu->Append(static_cast<int>(MenuItemId_Player::OpenFolders), wxString::Format("%s\tCtrl+D", Strings::FramePlayer::MENU_ITEM_OPEN_FOLDERS));
				fileMenu->AppendSeparator();
				fileMenu->Append(static_cast<int>(MenuItemId_Player::EnqueueFiles), wxString::Format("%s\tCtrl+Shift+O",Strings::FramePlayer::MENU_ITEM_ENQUEUE_FILES));
				fileMenu->Append(static_cast<int>(MenuItemId_Player::EnqueueFolders), wxString::Format("%s\tCtrl+Shift+D", Strings::FramePlayer::MENU_ITEM_ENQUEUE_FOLDERS));
				fileMenu->AppendSeparator();
				// Playlist submenu
				wxMenu* playlistSubMenu = new wxMenu();
				playlistSubMenu->Append(static_cast<int>(MenuItemId_Player::PlaylistOpen), Strings::FramePlayer::MENU_ITEM_PLAYLIST_OPEN);
				playlistSubMenu->Append(static_cast<int>(MenuItemId_Player::PlaylistSave), Strings::FramePlayer::MENU_ITEM_PLAYLIST_SAVE);
				playlistSubMenu->AppendSeparator();
				playlistSubMenu->Append(static_cast<int>(MenuItemId_Player::PlaylistClear), Strings::FramePlayer::MENU_ITEM_PLAYLIST_CLEAR);
				fileMenu->AppendSubMenu(playlistSubMenu, Strings::FramePlayer::MENU_ITEM_SUBMENU_PLAYLIST);
				// **
				fileMenu->AppendSeparator();
				fileMenu->Append(static_cast<int>(MenuItemId_Player::Exit), wxString::Format("%s\tAlt+F4", Strings::FramePlayer::MENU_ITEM_EXIT));

				menuBar->Append(fileMenu, Strings::FramePlayer::MENU_FILE);
			}

			// Edit menu
			{
				wxMenu* editMenu = new wxMenu();

				editMenu->Append(static_cast<int>(MenuItemId_Player::Find), wxString::Format("%s\tCtrl+F", Strings::FramePlayer::MENU_ITEM_FIND));
				editMenu->Append(static_cast<int>(MenuItemId_Player::FindNext), wxString::Format("%s\tF3", Strings::FramePlayer::MENU_ITEM_FIND_NEXT));
				editMenu->Append(static_cast<int>(MenuItemId_Player::FindPrev), wxString::Format("%s\tShift+F3", Strings::FramePlayer::MENU_ITEM_FIND_PREV));
				editMenu->AppendSeparator();
				editMenu->Append(static_cast<int>(MenuItemId_Player::PlaybackMods), wxString::Format("%s\tF5", Strings::FramePlayer::MENU_ITEM_PLAYBACK_MODS));
				editMenu->Append(static_cast<int>(MenuItemId_Player::Preferences), Strings::FramePlayer::MENU_ITEM_PREFERENCES);

				menuBar->Append(editMenu, Strings::FramePlayer::MENU_EDIT);
			}

			// View menu
			{
				wxMenu* viewMenu = new wxMenu();
				viewMenu->AppendCheckItem(static_cast<int>(MenuItemId_Player::StayTopmost), wxString::Format("%s\tAlt+A", Strings::FramePlayer::MENU_ITEM_STAY_TOPMOST));
				viewMenu->AppendCheckItem(static_cast<int>(MenuItemId_Player::VisualizationEnabled), wxString::Format(Strings::FramePlayer::MENU_ITEM_VISUALIZATION_ENABLED));
				viewMenu->AppendCheckItem(static_cast<int>(MenuItemId_Player::StilInfoEnabled), wxString::Format(Strings::FramePlayer::MENU_ITEM_STIL_INFO));
				viewMenu->AppendSeparator();
				viewMenu->Append(static_cast<int>(MenuItemId_Player::TuneInfo), wxString::Format("%s\tCtrl+I", Strings::FramePlayer::MENU_ITEM_TUNE_INFO));

				menuBar->Append(viewMenu, Strings::FramePlayer::MENU_VIEW);
			}

			// Help menu
			{
				wxMenu* helpMenu = new wxMenu();
				helpMenu->Append(static_cast<int>(MenuItemId_Player::CheckUpdates), wxString::Format("%s", Strings::FramePlayer::MENU_ITEM_CHECK_UPDATES));
				helpMenu->Append(static_cast<int>(MenuItemId_Player::About), wxString::Format("%s", Strings::FramePlayer::MENU_ITEM_ABOUT));

				menuBar->Append(helpMenu, Strings::FramePlayer::MENU_HELP);
			}
		}

		// Sizers
		wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
		AttachFixedSizeSeparator(wxSize(-1, TEMP_LABEL_BORDER_SIZE * 2), sizerMain, panel); // Add some top-margin to clear the song area from the menubar.

		wxBoxSizer* sizerSongArea = new wxBoxSizer(wxHORIZONTAL);
		sizerMain->Add(sizerSongArea, 0, wxEXPAND);

		// Left
		wxBoxSizer* sizerSongInfoLeft = new wxBoxSizer(wxVERTICAL);
		sizerSongArea->Add(sizerSongInfoLeft, 1);

		// Right
		sizerStilRight = new wxBoxSizer(wxVERTICAL);
		sizerSongArea->Add(sizerStilRight, 1, wxEXPAND);

		// Labels
		labelTitle = AttachIconLabel(themeData.GetImage("icon_music"), Strings::FramePlayer::TOOLTIP_PSID_TITLE, _parentPanel, sizerSongInfoLeft);

		{
			wxFont boldFont = labelTitle->GetFont();
			boldFont.MakeBold();
			labelTitle->SetFont(boldFont);
		}

		labelAuthor = AttachIconLabel(themeData.GetImage("icon_author"), Strings::FramePlayer::TOOLTIP_PSID_AUTHOR, _parentPanel, sizerSongInfoLeft);
		labelCopyright = AttachIconLabel(themeData.GetImage("icon_copyright"), Strings::FramePlayer::TOOLTIP_PSID_COPYRIGHT, _parentPanel, sizerSongInfoLeft);

		// STIL scrolling labels
		{
			const ThemeData::ThemedElementData&	theme = themeData.GetThemedElement("StilInfo");
			labelStilNameTitle = AttachStilIconScrollingLabel(themeData.GetImage("icon_stilnametitle"), theme, Strings::FramePlayer::TOOLTIP_STIL_NAME_TITLE, _parentPanel, sizerStilRight, UIElements::ScrollingLabel::TextJustify::Right);
			labelStilArtistAuthor = AttachStilIconScrollingLabel(themeData.GetImage("icon_stilartistauthor"), theme, Strings::FramePlayer::TOOLTIP_STIL_ARTIST_AUTHOR, _parentPanel, sizerStilRight, UIElements::ScrollingLabel::TextJustify::Right);
			labelStilComment = AttachStilIconScrollingLabel(themeData.GetImage("icon_stilcomment"), theme, Strings::FramePlayer::TOOLTIP_STIL_COMMENT, _parentPanel, sizerStilRight, UIElements::ScrollingLabel::TextJustify::Right);
		}

		// Visualization
		waveformVisualization = new UIElements::WaveformVisualization(&_parentPanel, themeData.GetThemedElement("WaveformVisualization"));
		waveformVisualization->Hide();
		waveformVisualization->SetMinClientSize(wxSize(-1, 10));
		waveformVisualization->SetMaxClientSize(wxSize(-1, 72));
		sizerMain->Add(waveformVisualization, 1, wxEXPAND | wxALL, TEMP_LABEL_BORDER_SIZE);

		AttachFixedSizeSeparator(DpiSize(0, 10 + 4), sizerMain, _parentPanel); // TODO: magic numbers

		// Sizer below
		wxBoxSizer* gridSizerPlaybackButtons = new wxBoxSizer(wxHORIZONTAL);

		// Always enabled
		wxBoxSizer* gridSubSizer1Vert = new wxBoxSizer(wxVERTICAL);
		sizerMain->Add(gridSizerPlaybackButtons, 0, wxEXPAND, 0);
		gridSizerPlaybackButtons->Add(gridSubSizer1Vert, 0, wxCENTER, 0);

		sliderVolume = new wxSlider(&_parentPanel, wxID_ANY, 100, 0, 100);
		gridSubSizer1Vert->Add(sliderVolume, 0, wxEXPAND | wxALL, 0);

		// Playback controls & other buttons
		btnStop = AttachSimplePlaybackControlButton(themeData.GetImage("btn_stop"), _parentPanel, gridSizerPlaybackButtons);

		btnPlayPause = new UIElements::PlayPauseButton(themeData.GetImage("btn_play"), themeData.GetImage("btn_pause"), DpiSize(TEMP_BUTTON_SIZE), _parentPanel);
		gridSizerPlaybackButtons->Add(btnPlayPause, 0, wxALL, TEMP_BUTTON_BORDER_SIZE);

		gridSizerPlaybackButtons->Add(0, 0, wxEXPAND, 0); // This "spacer" makes the playback control buttons horizontally centered (without it, they'd be left-aligned).

		btnPrevTrack = AttachSimplePlaybackControlButton(themeData.GetImage("btn_skipback"), _parentPanel, gridSizerPlaybackButtons);
		btnPrevSubsong = AttachSimplePlaybackControlButton(themeData.GetImage("btn_prevsubsong"), _parentPanel, gridSizerPlaybackButtons);

		labelSubsong = new wxStaticText(&_parentPanel, wxID_ANY, "00 / 00", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxST_NO_AUTORESIZE);
		gridSizerPlaybackButtons->Add(labelSubsong, 1, wxALIGN_CENTER_VERTICAL, TEMP_LABEL_BORDER_SIZE);

		btnNextSubsong = AttachSimplePlaybackControlButton(themeData.GetImage("btn_nextsubsong"), _parentPanel, gridSizerPlaybackButtons);
		btnNextTrack = AttachSimplePlaybackControlButton(themeData.GetImage("btn_skipforward"), _parentPanel, gridSizerPlaybackButtons);

		gridSizerPlaybackButtons->Add(0, 0, wxEXPAND, 0); // This "spacer" makes the playback control buttons horizontally centered (without it, they'd be right-aligned).

		// Playback modifier button
		btnPlaybackMod = AttachSimplePlaybackControlButton(themeData.GetImage("btn_eq"), _parentPanel, gridSizerPlaybackButtons);

		AttachFixedSizeSeparator(DpiSize(0, 1 + 4), sizerMain, _parentPanel); // TODO: magic number

		// Seekbar area...
		wxBoxSizer* sizerSeekbar = new wxBoxSizer(wxHORIZONTAL);
		sizerMain->Add(sizerSeekbar, 0, wxEXPAND, 0);

		// RepeatMode button
		{
			using RepeatMode = UIElements::RepeatModeButton::RepeatMode;
			using ButtonState = UIElements::RepeatModeButton::ButtonState;
			const UIElements::RepeatModeButton::ButtonStates repeatModes =
			{
				{RepeatMode::Normal, ButtonState(themeData.GetImage("btn_sequential"), Strings::FramePlayer::REPEAT_MODE_SEQUENTIAL)},
				{RepeatMode::RepeatOne, ButtonState(themeData.GetImage("btn_repeatone"), Strings::FramePlayer::REPEAT_MODE_REPEAT_ONE)},
				{RepeatMode::RepeatAll, ButtonState(themeData.GetImage("btn_repeatall"), Strings::FramePlayer::REPEAT_MODE_REPEAT_ALL)},
				{RepeatMode::InfiniteDuration, ButtonState(themeData.GetImage("btn_infinite"), Strings::FramePlayer::REPEAT_MODE_LEAVE_RUNNING)},
				{RepeatMode::PlayOnce, ButtonState(themeData.GetImage("btn_playonce"), Strings::FramePlayer::REPEAT_MODE_PLAY_ONCE)}
			};

			// ------------------------------------------------

			using ExtraOption = UIElements::RepeatModeButton::ExtraOptionsHandler::ExtraOption;
			using ExtraOptionId = UIElements::RepeatModeButton::ExtraOptionsHandler::ExtraOptionId;
			using ExtraOptionsHandler = UIElements::RepeatModeButton::ExtraOptionsHandler;
			const bool PREPEND_SEPARATOR = true;

			const ExtraOptionsHandler::ExtraOptions extraOptions
			{
				{ExtraOptionId::DefaultSubsong, ExtraOption(Strings::Preferences::OPT_START_DEFAULT_SUBSONG, false)},
				{ExtraOptionId::IncludeSubsongs, ExtraOption(Strings::Preferences::OPT_INCLUDE_SUBSONGS, false)},
				// ----
				{ExtraOptionId::PreRenderEnabled, ExtraOption(Strings::Preferences::OPT_PRERENDER, false, PREPEND_SEPARATOR)},
			};

			ExtraOptionsHandler extraOptionsHandler(extraOptions, [&appSettings](UIElements::RepeatModeButton& btnRepeatMode)
			{ // Action callbacks
				{ // RepeatModeIncludeSubsongs
					const bool enabled = appSettings.GetOption(Settings::AppSettings::ID::RepeatModeIncludeSubsongs)->GetValueAsBool();
					btnRepeatMode.SetExtraOptionEnabled(ExtraOptionId::IncludeSubsongs, enabled);
				}

				{ // RepeatModeDefaultSubsong
					const bool enabled = appSettings.GetOption(Settings::AppSettings::ID::RepeatModeDefaultSubsong)->GetValueAsBool();
					btnRepeatMode.SetExtraOptionEnabled(ExtraOptionId::DefaultSubsong, enabled);
				}

				{ // PreRenderEnabled
					const bool enabled = appSettings.GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool();
					btnRepeatMode.SetExtraOptionEnabled(ExtraOptionId::PreRenderEnabled, enabled);
				}
			});

			// ------------------------------------------------

			btnRepeatMode = new UIElements::RepeatModeButton(repeatModes, DpiSize(TEMP_BUTTON_SIZE), _parentPanel, extraOptionsHandler);
			sizerSeekbar->Add(btnRepeatMode, 0, wxALL, TEMP_LABEL_TIME_BORDER_SIZE);
		}

		// Seekbar itself
		compositeSeekbar = new UIElements::CompositeSeekBar(&_parentPanel, themeData.GetThemedElement("Seekbar"));
		sizerSeekbar->Add(compositeSeekbar, 1, wxEXPAND | wxALL, TEMP_LABEL_TIME_BORDER_SIZE);

		labelTime = new wxStaticText(&_parentPanel, wxID_ANY, wxT("000:00"), wxDefaultPosition, wxDefaultSize, wxSTB_DEFAULT_STYLE | wxTEXT_ALIGNMENT_CENTER); // Extra padding in text on purpose!
		sizerSeekbar->Add(labelTime, 0, wxFIXED_MINSIZE | wxALIGN_CENTER_VERTICAL, TEMP_LABEL_TIME_BORDER_SIZE);

		AttachFixedSizeSeparator(DpiSize(TEMP_LABEL_BORDER_SIZE, 0), sizerSeekbar, _parentPanel);

 		// Playlist
		{
			UIElements::Playlist::PlaylistIcons playlistIcons(TEMP_PLAYLIST_ICON_SIZE);
			playlistIcons.RegisterSvgIcon(UIElements::Playlist::PlaylistIconId::DefaultSubsongIndicator, themeData.GetImage("icon_defaultsubsong"));
			playlistIcons.RegisterSvgIcon(UIElements::Playlist::PlaylistIconId::ChipIcon, themeData.GetImage("icon_chip"));
			playlistIcons.RegisterSvgIcon(UIElements::Playlist::PlaylistIconId::SkipShort, themeData.GetImage("icon_skipshort"));
			playlistIcons.RegisterSvgIcon(UIElements::Playlist::PlaylistIconId::RemoveSong, themeData.GetImage("icon_removesong"));

			treePlaylist = new UIElements::Playlist::Playlist(&_parentPanel, playlistIcons, appSettings, wxDV_SINGLE);
			sizerMain->Add(treePlaylist->GetWxWindow(), 1, wxEXPAND | wxALL, 0);

			// Context menu icons
			playlistContextMenuIcons = {
				{PlaylistContextMenuIconId::RemoveMainSong, LoadMenuIcon(themeData, "icon_removemainsong")},
				{PlaylistContextMenuIconId::RemoveAllAbove, LoadMenuIcon(themeData, "icon_removeallabove")},
				{PlaylistContextMenuIconId::RemoveAllBelow, LoadMenuIcon(themeData, "icon_removeallbelow")},
				{PlaylistContextMenuIconId::ExpandAll, LoadMenuIcon(themeData, "icon_expandall")},
				{PlaylistContextMenuIconId::CollapseAll, LoadMenuIcon(themeData, "icon_collapseall")},
				{PlaylistContextMenuIconId::ScrollToCurrent, LoadMenuIcon(themeData, "icon_scrolltocurrent")},
				{PlaylistContextMenuIconId::SkipSubsong, LoadMenuIcon(themeData, "icon_removesongmenu")}
			};
		}

 		// Search bar
		{
			searchBar = new UIElements::SearchBar(&_parentPanel, false);
			sizerMain->Add(searchBar->GetPanel(), 0, wxEXPAND, 0);
		}

		// Setup
		_parentPanel.SetSizer(sizerMain);
		_parentPanel.Layout();

		// Simple UI bindings
		sliderVolume->Bind(wxEVT_CONTEXT_MENU, &OnVolumeSliderContextMenuOpen, this);
		_parentPanel.Bind(wxEVT_CONTEXT_MENU, &OnPanelContextMenuOpen, this);
	}

	void ElementsPlayer::EnablePlaybackControls(bool enabled)
	{
		btnPrevTrack->Enable(enabled);
		btnPrevSubsong->Enable(enabled);
		btnStop->Enable(enabled);
		btnPlayPause->Enable(enabled);
		btnNextSubsong->Enable(enabled);
		btnNextTrack->Enable(enabled);
		labelSubsong->Enable(enabled);
		labelTime->Enable(enabled);
	}

	void ElementsPlayer::OnVolumeSliderContextMenuOpen(wxContextMenuEvent& /*evt*/)
	{
		wxMenu menu;

		// Title (don't use the built-in title functionality, it crashes on selection)
		{
			const std::string strVol = std::to_string(sliderVolume->GetValue());
			wxMenuItem* title = new wxMenuItem(&menu, wxID_ANY, wxString::Format("%s %s%", Strings::FramePlayer::VOL_MENU_PREFIX, strVol));
			title->SetFont(title->GetFont().MakeBold());
			menu.Append(title);
			menu.AppendSeparator();
		}

		// Set max volume
		menu.Append(static_cast<int>(PopupMenuItemId_VolumeSlider::ResetVolume), Strings::FramePlayer::VOL_SET_MAX);

		// Toggle slider
		if (sliderVolume->IsEnabled())
		{
			menu.Append(static_cast<int>(PopupMenuItemId_VolumeSlider::DisableControl), Strings::FramePlayer::VOL_SLIDER_DISABLE);
		}
		else
		{
			menu.Append(static_cast<int>(PopupMenuItemId_VolumeSlider::EnableControl), Strings::FramePlayer::VOL_SLIDER_ENABLE);
		}

		// Bind & show
		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &OnVolumeSliderPopupMenuItemClick, this);
		sliderVolume->PopupMenu(&menu);
	}

	void ElementsPlayer::OnVolumeSliderPopupMenuItemClick(wxCommandEvent& evt)
	{
		switch(static_cast<PopupMenuItemId_VolumeSlider>(evt.GetId()))
		{
			case PopupMenuItemId_VolumeSlider::ResetVolume:
			{
				sliderVolume->SetValue(sliderVolume->GetMax()); // This won't trigger an event.
				wxCommandEvent evtNew(wxEVT_SLIDER, evt.GetId());
				evtNew.SetEventObject(sliderVolume);
				evtNew.SetInt(sliderVolume->GetMax());
				sliderVolume->GetEventHandler()->AddPendingEvent(evtNew);
				break;
			}

			case PopupMenuItemId_VolumeSlider::DisableControl:
			{
				sliderVolume->Enable(false);
				break;
			}

			case PopupMenuItemId_VolumeSlider::EnableControl:
			{
				sliderVolume->Enable(true);
				break;
			}

			default:
				std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
				break;
		}
	}

	void ElementsPlayer::OnPanelContextMenuOpen(wxContextMenuEvent& evt)
	{
		if (!sliderVolume->IsEnabled() && sliderVolume->GetScreenRect().Contains(evt.GetPosition())) // Clicked on a disabled volume slider.
		{
			OnVolumeSliderContextMenuOpen(evt);
		}
	}
}
