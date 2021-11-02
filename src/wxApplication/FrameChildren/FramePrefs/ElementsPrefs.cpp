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

#include "ElementsPrefs.h"

namespace FrameElements
{
	static constexpr int BORDER = 10; // To make the resizing widget easily accessible on MSW.

	ElementsPrefs::ElementsPrefs(wxDialog& dialog)
	{
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		propertyGrid = new wxPropertyGridManager(&dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_LIMITED_EDITING | wxPG_BOLD_MODIFIED | wxPG_DESCRIPTION);
		//propertyGrid->SetExtraStyle(wxPG_EX_HELP_AS_TOOLTIPS);
		sizer->Add(propertyGrid, 1, wxEXPAND, 0);
		sizer->Add(dialog.CreateButtonSizer(wxAPPLY | wxCANCEL | wxOK), 0, wxEXPAND | wxALL, BORDER);
		dialog.SetSizer(sizer);
	}
}
