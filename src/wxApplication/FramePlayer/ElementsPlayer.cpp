/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2022 Jasmin Rutic (bytespiller@gmail.com)
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
	static constexpr int TEMP_BUTTON_SIZE = 30;
	static constexpr int TEMP_BUTTON_BORDER_SIZE = 1;
	static constexpr int TEMP_LABEL_BORDER_SIZE = 2;
	static constexpr int TEMP_LABEL_TIME_BORDER_SIZE = 2;
	static constexpr int TEMP_LABEL_ICON_SIZE = 14;
	static constexpr int TEMP_PLAYLIST_ICON_SIZE = 16;
}

namespace FrameElements // Static functions
{
	static wxStaticText* AttachIconLabel(const ThemeData::ThemeImage& themeImage, wxPanel& parent, wxBoxSizer* parentSizer)
	{
		wxBoxSizer* sizerIconLabel = new wxBoxSizer(wxHORIZONTAL);
		parentSizer->Add(sizerIconLabel, 1, wxEXPAND, 0);

		const auto& img = UIElements::Util::LoadRasterizedSvg(themeImage.path.c_str(), DpiSize(TEMP_LABEL_ICON_SIZE, TEMP_LABEL_ICON_SIZE), themeImage.offset, themeImage.scale);
		wxStaticBitmap* bitmapIcon = new wxStaticBitmap(&parent, wxID_ANY, *img.get());
		sizerIconLabel->Add(bitmapIcon, 0, wxEXPAND | wxALL, TEMP_LABEL_BORDER_SIZE);

		wxStaticText* labelPtr = new wxStaticText(&parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);
		sizerIconLabel->Add(labelPtr, 1, wxALIGN_CENTER_VERTICAL, TEMP_LABEL_BORDER_SIZE);

		return labelPtr;
	}

	static wxButton* AttachSimplePlaybackControlButton(const ThemeData::ThemeImage& themeImage, wxPanel& parent, wxBoxSizer* parentSizer)
	{
		wxButton* newButton = UIElements::Util::NewSvgButton(themeImage, DpiSize(TEMP_BUTTON_SIZE, TEMP_BUTTON_SIZE), parent);
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

				editMenu->Append(static_cast<int>(MenuItemId_Player::PlaybackMods), wxString::Format("%s\tF5", Strings::FramePlayer::MENU_ITEM_PLAYBACK_MODS));
				editMenu->AppendSeparator();
				editMenu->Append(static_cast<int>(MenuItemId_Player::Preferences), Strings::FramePlayer::MENU_ITEM_PREFERENCES);

				menuBar->Append(editMenu, Strings::FramePlayer::MENU_EDIT);
			}

			// Help menu
			{
				wxMenu* helpMenu = new wxMenu();
				helpMenu->Append(static_cast<int>(MenuItemId_Player::About), wxString::Format("%s", Strings::FramePlayer::MENU_ITEM_ABOUT));
				menuBar->Append(helpMenu, Strings::FramePlayer::MENU_HELP);
			}
		}

		// Sizers
		wxBoxSizer* sizerMain = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* sizerSongInfo = new wxBoxSizer(wxVERTICAL);
		sizerMain->Add(sizerSongInfo, 0, wxEXPAND, 0);

		// Labels
		AttachFixedSizeSeparator(DpiSize(0, 4), sizerSongInfo, _parentPanel); // TODO: magic numbers

		labelTitle = AttachIconLabel(themeData.GetImage("icon_music"), _parentPanel, sizerSongInfo);

		{
			wxFont boldFont = labelTitle->GetFont();
			boldFont.MakeBold();
			labelTitle->SetFont(boldFont);
		}

		labelAuthor = AttachIconLabel(themeData.GetImage("icon_author"), _parentPanel, sizerSongInfo);
		labelCopyright = AttachIconLabel(themeData.GetImage("icon_copyright"), _parentPanel, sizerSongInfo);

		AttachFixedSizeSeparator(DpiSize(0, 10), sizerSongInfo, _parentPanel); // TODO: magic numbers

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

		btnPlayPause = new UIElements::PlayPauseButton(themeData.GetImage("btn_play"), themeData.GetImage("btn_pause"), DpiSize(TEMP_BUTTON_SIZE, TEMP_BUTTON_SIZE), _parentPanel);
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

		// Seekbar area...
		AttachFixedSizeSeparator(DpiSize(0, 10), sizerMain, _parentPanel); // TODO: magic number

		wxBoxSizer* sizerSeekbar = new wxBoxSizer(wxHORIZONTAL);
		sizerMain->Add(sizerSeekbar, 0, wxEXPAND, 0);

		{ // RepeatMode button
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

			const ExtraOptionsHandler::ExtraOptions extraOptions
			{
				{ExtraOptionId::DefaultSubsong, ExtraOption(Strings::Preferences::OPT_START_DEFAULT_SUBSONG, false)},
				{ExtraOptionId::IncludeSubsongs, ExtraOption(Strings::Preferences::OPT_INCLUDE_SUBSONGS, false)},
			};

			ExtraOptionsHandler extraOptionsHandler(extraOptions, [&appSettings](UIElements::RepeatModeButton& btnRepeatMode)
			{
				{ // RepeatModeIncludeSubsongs
					const bool enabled = appSettings.GetOption(Settings::AppSettings::ID::RepeatModeIncludeSubsongs)->GetValueAsBool();
					btnRepeatMode.SetExtraOptionEnabled(ExtraOptionId::IncludeSubsongs, enabled);
				}

				{ // RepeatModeDefaultSubsong
					const bool enabled = appSettings.GetOption(Settings::AppSettings::ID::RepeatModeDefaultSubsong)->GetValueAsBool();
					btnRepeatMode.SetExtraOptionEnabled(ExtraOptionId::DefaultSubsong, enabled);
				}
			});

			// ------------------------------------------------

			btnRepeatMode = new UIElements::RepeatModeButton(repeatModes, DpiSize(TEMP_BUTTON_SIZE, TEMP_BUTTON_SIZE), _parentPanel, extraOptionsHandler);
			sizerSeekbar->Add(btnRepeatMode, 0, wxALL, TEMP_LABEL_TIME_BORDER_SIZE);
		}

		// Seekbar itself
		compositeSeekbar = new UIElements::CompositeSeekBar(&_parentPanel, themeData.GetThemedElement("Seekbar"));
		sizerSeekbar->Add(compositeSeekbar, 1, wxEXPAND | wxALL, TEMP_LABEL_TIME_BORDER_SIZE);

		labelTime = new wxStaticText(&_parentPanel, wxID_ANY, wxT("000:00"), wxDefaultPosition, wxDefaultSize, wxSTB_DEFAULT_STYLE | wxTEXT_ALIGNMENT_CENTER); // Extra padding in text on purpose!
		sizerSeekbar->Add(labelTime, 0, wxFIXED_MINSIZE | wxALIGN_CENTER_VERTICAL, TEMP_LABEL_TIME_BORDER_SIZE);

		AttachFixedSizeSeparator(DpiSize(TEMP_LABEL_BORDER_SIZE, 0), sizerSeekbar, _parentPanel);

		{ // Playlist
			UIElements::Playlist::PlaylistIcons playlistIcons(TEMP_PLAYLIST_ICON_SIZE);
			playlistIcons.RegisterSvgIcon(static_cast<int>(PlaylistIconId::DefaultSubsongIndicator), themeData.GetImage("icon_defaultsubsong"));
			playlistIcons.RegisterSvgIcon(static_cast<int>(PlaylistIconId::ChipIcon), themeData.GetImage("icon_chip"));
			playlistIcons.RegisterSvgIcon(static_cast<int>(PlaylistIconId::SkipShort), themeData.GetImage("icon_skipshort"));
			playlistIcons.RegisterSvgIcon(static_cast<int>(PlaylistIconId::RemoveSong), themeData.GetImage("icon_removesong"));

			treePlaylist = new UIElements::Playlist::Playlist(_parentPanel, playlistIcons, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_FULL_ROW_HIGHLIGHT | wxTR_HAS_BUTTONS | wxTR_HIDE_ROOT | wxTR_NO_LINES | wxTR_ROW_LINES | wxTR_SINGLE | wxTR_TWIST_BUTTONS, appSettings);
			sizerMain->Add(&treePlaylist->GetBase(), 1, wxEXPAND | wxALL, 0);
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
		const std::string strVol = std::to_string(sliderVolume->GetValue());
		wxMenu menu(wxString::Format("%s %s%", Strings::FramePlayer::VOL_MENU_PREFIX, strVol));

		menu.Append(static_cast<int>(PopupMenuItemId_VolumeSlider::ResetVolume), Strings::FramePlayer::VOL_SET_MAX);

		if (sliderVolume->IsEnabled())
		{
			menu.Append(static_cast<int>(PopupMenuItemId_VolumeSlider::DisableControl), Strings::FramePlayer::VOL_SLIDER_DISABLE);
		}
		else
		{
			menu.Append(static_cast<int>(PopupMenuItemId_VolumeSlider::EnableControl), Strings::FramePlayer::VOL_SLIDER_ENABLE);
		}

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
