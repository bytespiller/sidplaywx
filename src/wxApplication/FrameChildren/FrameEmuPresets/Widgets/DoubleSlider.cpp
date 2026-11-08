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

#include "DoubleSlider.h"

#include <wx/spinctrl.h>

namespace
{
	constexpr double SPIN_MIN = 0.0;
	constexpr double SPIN_MAX = 1.0;
	constexpr double SPIN_STEP = 0.01;
}

namespace FrameElements
{
	DoubleSlider::DoubleSlider(const wxString& label, const wxString& tooltip, wxWindow* parent, double initialValue)
		: FilterWidgetBase(label, tooltip, parent)
	{
		// Widget sizer
		wxBoxSizer* sizerWidget = new wxBoxSizer(wxHORIZONTAL);
		parent->GetSizer()->Add(sizerWidget, 1, wxEXPAND | wxALL);

		// Slider
		_slider = static_cast<wxSlider*>(sizerWidget->Add(new wxSlider(parent, wxID_ANY, initialValue * 100, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL), 1, 0, 0)->GetWindow());

		_slider->Bind(wxEVT_MOUSEWHEEL, [](wxMouseEvent& event)
		{
			// Prevent scroll wheel altering the slider, and allow the parent scrollable container to scroll instead
			if (wxWindow* const parent = dynamic_cast<wxWindow*>(event.GetEventObject())->GetParent())
			{
				wxMouseEvent parentEvent(event);
				parent->GetGrandParent()->GetEventHandler()->ProcessEvent(parentEvent);
			}
		});

		_slider->Bind(wxEVT_SLIDER, [this](wxCommandEvent& event)
		{
			_spinCtrl->SetValue(event.GetInt() / 100.0);
		});

		// Spin ctrl
		_spinCtrl = static_cast<wxSpinCtrlDouble*>(sizerWidget->Add(new wxSpinCtrlDouble(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, SPIN_MIN, SPIN_MAX, initialValue, SPIN_STEP))->GetWindow());
		_spinCtrl->SetDigits(2);

		_spinCtrl->Bind(wxEVT_SPINCTRLDOUBLE, [this](wxSpinDoubleEvent& event)
		{
			_slider->SetValue(event.GetValue() * 100);
		});

		_spinCtrl->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&)
		{
			double val = 0.0;
			if (_spinCtrl->GetTextValue().ToDouble(&val)) // The spin control gets updated internally on focus loss only, so we have to use this workaround (or fake the focus loss).
			{
				SetValue(val); // Update both the spinCtrl internally and the slider.
			}
		});

		// Initial state
		_chkEnabled->SetValue(false);
		Enable(_chkEnabled->GetValue());
	}

	double DoubleSlider::GetValue() const
	{
		return _spinCtrl->GetValue();
	}

	void DoubleSlider::SetValue(double value)
	{
		_spinCtrl->SetValue(value);
		_slider->SetValue(_spinCtrl->GetValue() * 100);
	}

	void DoubleSlider::Enable(bool enabled)
	{
		FilterWidgetBase::Enable(enabled);
		_spinCtrl->Enable(enabled);
		_slider->Enable(enabled);
	}
}
