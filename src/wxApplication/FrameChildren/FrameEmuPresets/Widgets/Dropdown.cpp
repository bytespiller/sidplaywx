/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include "Dropdown.h"

#include <wx/choice.h>

namespace FrameElements
{
	Dropdown::Dropdown(const char* uid, const wxString& label, const wxString& tooltip, wxWindow* parent, const wxArrayString& choices, int initialValue) :
		FilterWidgetBase(uid, label, tooltip, parent)
	{
		// Widget sizer
		wxBoxSizer* sizerWidget = new wxBoxSizer(wxHORIZONTAL);
		parent->GetSizer()->Add(sizerWidget, 1, wxEXPAND | wxALL);

		// Dropdown
		_dropdown = static_cast<wxChoice*>(sizerWidget->Add(new wxChoice(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices), 1, 0, 0)->GetWindow());

		// Initial state
		SetValue(initialValue);
		_chkEnabled->SetValue(false);
		Enable(_chkEnabled->GetValue());
	}

	Settings::Option::Var Dropdown::GetValue() const
	{
		return _dropdown->GetSelection();
	}

	void Dropdown::SetValue(Settings::Option::Var value)
	{
		SetValue(std::get<int>(value));
	}

	void Dropdown::SetValue(int value)
	{
		_dropdown->SetSelection(value);
	}

	void Dropdown::Enable(bool enabled)
	{
		FilterWidgetBase::Enable(enabled);
		_dropdown->Enable(enabled);
	}
}
