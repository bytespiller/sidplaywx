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

#include "FrameTuneInfo.h"
#include "../../Config/UIStrings.h"
#include "../../Helpers/HelpersWx.h"
#include "../../UIElements/Playlist/Components/PlaylistModel.h"
#include "../../../PlaybackController/PlaybackController.h"
#include "../../../HvscSupport/Stil/Stil.h"
#include <sidplayfp/SidTuneInfo.h>

FrameTuneInfo::FrameTuneInfo(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, const PlaybackController& playbackInfo, Stil& stil)
	: wxDialog(parent, wxID_ANY, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
	_playbackInfo(playbackInfo),
	_stil(stil)
{
	SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL)); // Must be done before elements init.
	_ui = std::make_unique<FrameElements::ElementsTuneInfo>(*this);

	// *** Events ***

	// Close this window on ESC key
	Bind(wxEVT_CHAR_HOOK, [this](wxKeyEvent& evt)
	{
		if (evt.GetKeyCode() == WXK_ESCAPE)
		{
			Close();
		}

		evt.Skip();
	});

	// Clear the PropertyGrid selection so that its Description Box defaults to HVSC comment again when this window is re-opened.
	Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& evt)
	{
		_ui->propertyGrid->ClearSelection();
		evt.Skip();
	});

	// "Browse location" button
	_ui->buttonBrowse->Bind(wxEVT_BUTTON, [this](wxCommandEvent& /*evt*/)
	{
		if (_playbackInfo.IsValidSongLoaded())
		{
			const wxString& dir = wxPathOnly(wxString::FromUTF8(_playbackInfo.GetCurrentTuneFilePath().generic_u8string()));
			if (!dir.IsEmpty() && !wxLaunchDefaultApplication(dir))
			{
				wxMessageBox(dir, Strings::Common::GENERIC_NOT_FOUND, wxICON_WARNING);
			}
		}
	});

	// *** Configure window ***
	//SetDoubleBuffered(true); // No need at the moment.
	CenterOnParent();
	Layout();
}

bool FrameTuneInfo::Show(bool show, const PlaylistTreeModelNode* const node)
{
	if (show)
	{
		return ShowAndUpdate(node);
	}
	else
	{
		return wxDialog::Show(show);
	}
}

bool FrameTuneInfo::ShowAndUpdate(const PlaylistTreeModelNode* const node)
{
	UpdateInfo(node);
	return wxDialog::Show();
}

void FrameTuneInfo::Hide()
{
	wxDialog::Hide();
}

bool FrameTuneInfo::IsVisible() const
{
	return wxDialog::IsVisible();
}

bool FrameTuneInfo::ShouldFollowPlayback() const
{
	return IsVisible() && _ui->checkboxFollowPlayback->IsChecked();
}

void FrameTuneInfo::UpdateInfo(const PlaylistTreeModelNode* const node)
{
	if (!_playbackInfo.IsValidSongLoaded())
	{
		return;
	}

	const SidTuneInfo& sidInfo = _playbackInfo.GetCurrentTuneSidInfo();
	const SidInfo& engineInfo = _playbackInfo.GetEngineInfo();

	// General
	SetPropertyValue(Strings::TuneInfo::TUNE_TITLE, _playbackInfo.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Title));
	SetPropertyValue(Strings::TuneInfo::TUNE_AUTHOR, _playbackInfo.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Author));
	SetPropertyValue(Strings::TuneInfo::TUNE_RELEASED, _playbackInfo.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Released));
	SetPropertyValue(Strings::TuneInfo::TUNE_MUS_COMMENT, _playbackInfo.GetCurrentTuneMusComments());

	_ui->propertyGrid->SetPropertyValue(Strings::TuneInfo::TUNE_PATH_FILE, wxString::FromUTF8(_playbackInfo.GetCurrentTuneFilePath().generic_u8string()));
	_ui->propertyGrid->GetProperty(Strings::TuneInfo::TUNE_PATH_FILE)->SetHelpString(_ui->propertyGrid->GetPropertyValue(Strings::TuneInfo::TUNE_PATH_FILE));

	// Technical
	SetPropertyValue(Strings::TuneInfo::TUNE_ADDR_DRIVER, wxString::Format("$%04x - $%04x", engineInfo.driverAddr(), engineInfo.driverAddr() + engineInfo.driverLength() - 1).MakeUpper().ToStdString());
	SetPropertyValue(Strings::TuneInfo::TUNE_ADDR_LOAD, wxString::Format("$%04x - $%04x", sidInfo.loadAddr(), sidInfo.loadAddr() + sidInfo.c64dataLen() - 1).MakeUpper().ToStdString());
	SetPropertyValue(Strings::TuneInfo::TUNE_ADDR_INIT, wxString::Format("$%04x", sidInfo.initAddr()).MakeUpper().ToStdString());
	SetPropertyValue(Strings::TuneInfo::TUNE_ADDR_PLAY, wxString::Format("$%04x", sidInfo.playAddr()).MakeUpper().ToStdString());

	SetPropertyValue(Strings::TuneInfo::TUNE_SIZE, wxString::Format("%i ($%04x) / %i ($%04x)", sidInfo.c64dataLen(), sidInfo.c64dataLen(), sidInfo.dataFileLen(), sidInfo.dataFileLen()).MakeUpper().ToStdString());
	SetPropertyValue(Strings::TuneInfo::TUNE_SUBSONG, wxString::Format("%i / %i (%i)", sidInfo.currentSong(), sidInfo.songs(), sidInfo.startSong()).ToStdString());
	SetPropertyValue(Strings::TuneInfo::TUNE_MODEL_SID, _playbackInfo.GetCurrentTuneSidDescription(false));
	SetPropertyValue(Strings::TuneInfo::TUNE_MODEL_C64, _playbackInfo.GetCurrentTuneSpeedDescription());

	{ // ROM requirement
		wxString romStr("-");
		switch (_playbackInfo.GetCurrentSongRomRequirement())
		{
			case SidDecoder::RomRequirement::BasicRom:
				romStr = "BASIC";
				break;
			case SidDecoder::RomRequirement::R64:
				romStr = "KERNAL";
				break;
		}

		SetPropertyValue(Strings::TuneInfo::TUNE_ROM, romStr.ToStdString());
	}

	SetPropertyValue(Strings::TuneInfo::TUNE_TYPE, sidInfo.formatString());

	// HVSC
	const Stil::Info& stilInfo = (_stil.IsLoaded() && node != nullptr) ? _stil.Get(node->hvscPath.ToStdString()) : Stil::Info();
	const int subsong = sidInfo.currentSong();

	SetPropertyValue(Strings::TuneInfo::HVSC_CANONICAL, (node != nullptr) ? node->hvscPath.ToStdString() : "");
	SetPropertyValue(Strings::TuneInfo::HVSC_NAME, stilInfo.GetFieldAsString(stilInfo.names, subsong, "\n", true));
	SetPropertyValue(Strings::TuneInfo::HVSC_TITLE, stilInfo.GetFieldAsString(stilInfo.titles, subsong, "\n", true));
	SetPropertyValue(Strings::TuneInfo::HVSC_ARTIST, stilInfo.GetFieldAsString(stilInfo.artists, subsong, "\n", true));
	SetPropertyValue(Strings::TuneInfo::HVSC_AUTHOR, stilInfo.GetFieldAsString(stilInfo.authors, subsong, "\n", true));
	SetPropertyValue(Strings::TuneInfo::HVSC_COMMENT, stilInfo.GetFieldAsString(stilInfo.comments, subsong, "\n", true));
	SetPropertyValue(Strings::TuneInfo::HVSC_MD5, (node != nullptr) ? node->md5.ToStdString() : "");

	// *** FINAL ***

	// Refresh the Description Box (must be done manually)
	{
		const wxPGProperty& prop = (_ui->propertyGrid->GetSelection() != nullptr)
			? *_ui->propertyGrid->GetSelection() // Use selection.
			: *_ui->propertyGrid->GetProperty(Strings::TuneInfo::HVSC_COMMENT); // No selection, display STIL comment by default.

		_ui->propertyGrid->SetDescription(prop.GetLabel(), prop.GetHelpString());
	}

	_ui->propertyGrid->SetSplitterLeft();
	_ui->propertyGrid->Refresh();
}

void FrameTuneInfo::SetPropertyValue(const char* name, const std::string_view& value)
{
	const wxString& valueDecoded = Helpers::Wx::StringFromWin1252(value);

	// Single-line copyable property
	{
		wxString valueNewlinesToSpaces(valueDecoded);
		valueNewlinesToSpaces.Replace('\n', "  ");
		_ui->propertyGrid->SetPropertyValue(name, valueNewlinesToSpaces);
	}

	// Multi-line Description Box
	_ui->propertyGrid->GetProperty(name)->SetHelpString(valueDecoded);
}
