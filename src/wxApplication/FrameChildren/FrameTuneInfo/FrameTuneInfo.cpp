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
#include "../../UIElements/Playlist/Components/PlaylistModel.h"
#include "../../../PlaybackController/PlaybackController.h"
#include "../../../HvscSupport/Stil/Stil.h"
#include <sidplayfp/SidTuneInfo.h>

FrameTuneInfo::FrameTuneInfo(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, const PlaybackController& playbackInfo, Stil& stil)
	: wxDialog(parent, wxID_ANY, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
	_playbackInfo(playbackInfo),
	_stil(stil)
{
	_ui = std::make_unique<FrameElements::ElementsTuneInfo>(*this);

	SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	_ui->propertyGrid->SetFont(GetFont());
	_ui->checkboxFollowPlayback->SetFont(GetFont());

	// Events
	_ui->buttonBrowse->Bind(wxEVT_BUTTON, [this](wxCommandEvent& /*evt*/)
	{
		if (_playbackInfo.IsValidSongLoaded())
		{
			const wxString& dir = wxPathOnly(_playbackInfo.GetCurrentTuneFilePath());
			if (!dir.IsEmpty() && !wxLaunchDefaultApplication(dir))
			{
				wxMessageBox(dir, Strings::Common::GENERIC_NOT_FOUND, wxICON_WARNING);
			}
		}
	});

	// Configure window
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

	// General
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_TITLE, _playbackInfo.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Title));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_AUTHOR, _playbackInfo.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Author));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_RELEASED, _playbackInfo.GetCurrentTuneInfoString(PlaybackController::SongInfoCategory::Released));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_MUS_COMMENT, _playbackInfo.GetCurrentTuneMusComments());
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_PATH_FILE, _playbackInfo.GetCurrentTuneFilePath());

	// Technical
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_ADDR_LOAD, wxString::Format("$%04x", sidInfo.loadAddr()).MakeUpper());
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_ADDR_INIT, wxString::Format("$%04x", sidInfo.initAddr()).MakeUpper());
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_ADDR_PLAY, wxString::Format("$%04x", sidInfo.playAddr()).MakeUpper());

	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_SIZE, wxString::Format("%i ($%04x) / %i ($%04x)", sidInfo.c64dataLen(), sidInfo.c64dataLen(), sidInfo.dataFileLen(), sidInfo.dataFileLen()).MakeUpper());
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_SUBSONG, wxString::Format("%i / %i (%i)", sidInfo.currentSong(), sidInfo.songs(), sidInfo.startSong()));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_MODEL_SID, _playbackInfo.GetCurrentTuneSidDescription(false));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_MODEL_C64, _playbackInfo.GetCurrentTuneSpeedDescription());

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

		_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_ROM, romStr);
	}

	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::TUNE_TYPE, sidInfo.formatString());

	// HVSC
	const Stil::Info& stilInfo = (_stil.IsLoaded() && node != nullptr) ? _stil.Get(node->hvscPath.ToStdString()) : Stil::Info();
	const int subsong = sidInfo.currentSong();

	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::HVSC_CANONICAL, (node != nullptr) ? node->hvscPath : "");
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::HVSC_NAME, stilInfo.GetFieldAsString(stilInfo.names, subsong));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::HVSC_TITLE, stilInfo.GetFieldAsString(stilInfo.titles, subsong));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::HVSC_ARTIST, stilInfo.GetFieldAsString(stilInfo.artists, subsong));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::HVSC_AUTHOR, stilInfo.GetFieldAsString(stilInfo.authors, subsong));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::HVSC_COMMENT, stilInfo.GetFieldAsString(stilInfo.comments, subsong));
	_ui->propertyGrid->SetPropertyValueString(Strings::TuneInfo::HVSC_MD5, (node != nullptr) ? node->md5 : "");

	// FINAL
	_ui->propertyGrid->SetSplitterLeft();
	_ui->propertyGrid->Refresh();
}
