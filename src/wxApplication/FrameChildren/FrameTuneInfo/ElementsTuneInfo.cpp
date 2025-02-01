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

#include "ElementsTuneInfo.h"
#include "../../Config/UIStrings.h"
#include <wx/statline.h>

namespace FrameElements
{
	static constexpr int BORDER = 6;

	ElementsTuneInfo::ElementsTuneInfo(wxDialog& dialog)
	{
		// Setup layout
		{
			// Main vertical sizer
			wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);

			// PropertyGrid
			propertyGrid = new wxPropertyGridManager(&dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_LIMITED_EDITING | wxPG_NO_INTERNAL_BORDER | wxPG_HIDE_MARGIN | wxPG_DESCRIPTION);
			sizer->Add(propertyGrid, 1, wxEXPAND);

			// Secondary horizontal sizer below the PropertyGrid
			{
				wxBoxSizer* const sizerHorzBelowPg = new wxBoxSizer(wxHORIZONTAL);
				sizer->Add(sizerHorzBelowPg, 0, wxEXPAND | wxALL, dialog.FromDIP(BORDER));

				// Checkbox: Follow playback
				checkboxFollowPlayback = new wxCheckBox(&dialog, wxID_ANY, Strings::TuneInfo::CHECKBOX_FOLLOW_PLAYBACK);
				checkboxFollowPlayback->SetValue(true);
				sizerHorzBelowPg->Add(checkboxFollowPlayback, 0, wxALIGN_CENTER_VERTICAL);

				//sizerHorzBelowPg->AddStretchSpacer();
				wxStaticLine* verticalSeparator = new wxStaticLine(&dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
				sizerHorzBelowPg->Add(verticalSeparator, 0, wxEXPAND | wxLEFT | wxRIGHT, dialog.FromDIP(BORDER));

				// Button: Open location
				buttonBrowse = new wxButton(&dialog, wxID_ANY, Strings::Common::ACTION_BROWSE_LOCATION);
				sizerHorzBelowPg->Add(buttonBrowse, 0, wxALIGN_CENTER_VERTICAL);
			}

			// Set the main dialog sizer
			dialog.SetSizer(sizer);
		}

		// Fill the PropertyGrid
		{
			wxPropertyGridPage* const page = propertyGrid->AddPage();

			// General
			page->Append(new wxPropertyCategory(Strings::TuneInfo::CATEGORY_BASIC));
			{
				AddProperty(page, Strings::TuneInfo::TUNE_TITLE);
				AddProperty(page, Strings::TuneInfo::TUNE_AUTHOR);
				AddProperty(page, Strings::TuneInfo::TUNE_RELEASED);
				AddProperty(page, Strings::TuneInfo::TUNE_MUS_COMMENT);
				AddProperty(page, Strings::TuneInfo::TUNE_PATH_FILE);
			}

			// Technical
			page->Append(new wxPropertyCategory(Strings::TuneInfo::CATEGORY_TECHNICAL));
			{
				AddProperty(page, Strings::TuneInfo::TUNE_ADDR_LOAD);
				AddProperty(page, Strings::TuneInfo::TUNE_ADDR_INIT);
				AddProperty(page, Strings::TuneInfo::TUNE_ADDR_PLAY);

				AddProperty(page, Strings::TuneInfo::TUNE_SIZE);
				AddProperty(page, Strings::TuneInfo::TUNE_SUBSONG);
				AddProperty(page, Strings::TuneInfo::TUNE_MODEL_SID);
				AddProperty(page, Strings::TuneInfo::TUNE_MODEL_C64);
				AddProperty(page, Strings::TuneInfo::TUNE_TYPE);
				AddProperty(page, Strings::TuneInfo::TUNE_ROM);
			}

			// HVSC
			page->Append(new wxPropertyCategory(Strings::TuneInfo::CATEGORY_HVSC));
			{
				AddProperty(page, Strings::TuneInfo::HVSC_CANONICAL);
				AddProperty(page, Strings::TuneInfo::HVSC_NAME);
				AddProperty(page, Strings::TuneInfo::HVSC_TITLE);
				AddProperty(page, Strings::TuneInfo::HVSC_ARTIST);
				AddProperty(page, Strings::TuneInfo::HVSC_AUTHOR);
				AddProperty(page, Strings::TuneInfo::HVSC_COMMENT);
				AddProperty(page, Strings::TuneInfo::HVSC_MD5);
			}
		}
	}

	void ElementsTuneInfo::AddProperty(wxPropertyGridPage* page, const char* const title)
	{
		propertyGrid->SetPropertyReadOnly(page->Append(new wxStringProperty(title)));
	}
}
