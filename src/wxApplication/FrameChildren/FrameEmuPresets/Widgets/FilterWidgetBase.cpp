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

#include "FilterWidgetBase.h"

namespace FrameElements
{
	wxDEFINE_EVENT(wxEVT_FILTER_WIDGET_TOGGLED, wxCommandEvent);

	FilterWidgetBase::FilterWidgetBase(const char* uid, const wxString& label, const wxString& tooltip, wxWindow* parent) :
		wxEvtHandler(),
		_uid(uid),
		_parent(parent)
	{
		wxBoxSizer* sizerTitle = new wxBoxSizer(wxHORIZONTAL);
		parent->GetSizer()->Add(sizerTitle, 1, wxEXPAND | wxALL);

		_chkEnabled = new wxCheckBox(parent, wxID_ANY, label);
		_chkEnabled->SetToolTip(wxString::Format("%s: %s", label, tooltip));
		sizerTitle->Add(_chkEnabled, 1, wxEXPAND | wxALL);

		_chkEnabled->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& event)
		{
			Enable(!!event.GetInt());
		});
	}

	bool FilterWidgetBase::IsEnabled() const
	{
		return _chkEnabled->IsChecked();
	}

	const char* FilterWidgetBase::GetUid() const
	{
		return _uid;
	}

	void FilterWidgetBase::Enable(bool enabled)
	{
		_chkEnabled->SetValue(enabled);

		wxCommandEvent* event = new wxCommandEvent(wxEVT_FILTER_WIDGET_TOGGLED);
		event->SetInt(_chkEnabled->GetValue());
		event->SetClientData(static_cast<void*>(this));
		QueueEvent(event);
	}
}
