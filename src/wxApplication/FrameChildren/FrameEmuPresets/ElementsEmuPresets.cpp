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

#include "ElementsEmuPresets.h"
#include "../../Config/UIStrings.h"
#include <wx/spinctrl.h>
#include <wx/splitter.h>
//#include <wx/statline.h>

namespace
{
	static constexpr int BOX_BORDER_SIZE = 2;

	static const wxArrayString items =  // TEMP TEST
	{
		"Chris Huelsbeck",
		"Lasse Oorni"
	};
}

namespace FrameElements
{
	DoubleSlider::DoubleSlider(const wxString& label, const wxString& tooltip, wxBoxSizer* sizer, wxWindow* parent, double initialValue)
	{
		const wxString tooltipText = wxString::Format("%s: %s", label, tooltip);

		// Title
		wxStaticText* caption = new wxStaticText(parent, wxID_ANY, label);
		caption->SetToolTip(tooltipText);
		caption->SetCursor(wxCURSOR_QUESTION_ARROW);
		sizer->Add(caption, 0, wxLEFT | wxRIGHT | wxTOP, 5);

		// Sizer
		wxBoxSizer* widgetSizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(widgetSizer, 1, wxEXPAND | wxALL, 0);

		// Slider
		_slider = static_cast<wxSlider*>(widgetSizer->Add(new wxSlider(parent, wxID_ANY, initialValue / 100, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL), 1, wxEXPAND, 0)->GetWindow());
		_slider->SetToolTip(tooltipText);

		_slider->Bind(wxEVT_MOUSEWHEEL, [](wxMouseEvent& event)
		{
			// Prevent scroll wheel altering the slider, and allow the parent scrollable container to scroll instead
			if (wxWindow* const parent = dynamic_cast<wxWindow*>(event.GetEventObject())->GetParent())
			{
				wxMouseEvent parentEvent(event);
				parent->GetEventHandler()->ProcessEvent(parentEvent);
			}
		});

		_slider->Bind(wxEVT_SLIDER, [this](wxCommandEvent& event)
		{
			_spinCtrl->SetValue(event.GetInt() / 100.0);
		});

		// Spin ctrl
		static constexpr double MIN = 0.0;
		static constexpr double MAX = 1.0;
		static constexpr double STEP = 0.01;

		_spinCtrl = static_cast<wxSpinCtrlDouble*>(widgetSizer->Add(new wxSpinCtrlDouble(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, MIN, MAX, initialValue, STEP))->GetWindow());
		_spinCtrl->SetDigits(2);
		_spinCtrl->SetToolTip(tooltipText);

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
}

namespace FrameElements
{
	ElementsEmuPresets::ElementsEmuPresets(wxPanel& panel) :
		_parentPanel(panel)
	{
	    wxBoxSizer* sizerParent = new wxBoxSizer(wxHORIZONTAL);

		wxSplitterWindow* splitter = new wxSplitterWindow(&_parentPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
		sizerParent->Add(splitter, 1, wxEXPAND | wxALL, 0);

		// Presets (left)
        wxPanel* leftPane = new wxPanel(splitter, wxID_ANY);
		wxBoxSizer* sizerLeftPane = new wxBoxSizer(wxVERTICAL);
		_presetsList = new wxListBox(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, items, wxLB_SINGLE);
		sizerLeftPane->Add(_presetsList, 1, wxEXPAND | wxALL, 0);
		leftPane->SetSizer(sizerLeftPane);

		// Controls (right)
        wxPanel* rightPane = new wxPanel(splitter, wxID_ANY);
		wxBoxSizer* sizerRightPane = new wxBoxSizer(wxVERTICAL);
		rightPane->SetSizer(sizerRightPane);

		wxScrolled<wxPanel>* scrollPanel = new wxScrolled<wxPanel>(rightPane, wxID_ANY);
		sizerRightPane->Add(scrollPanel, 1, wxEXPAND | wxALL, BOX_BORDER_SIZE);

		wxBoxSizer* sizerControls = new wxBoxSizer(wxVERTICAL);
		scrollPanel->SetSizer(sizerControls);

		_sliderTest = std::make_unique<DoubleSlider>("Title 1", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 2", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 3", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 4", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 5", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 6", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 7", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 8", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 9", "Tooltip helptext", sizerControls, scrollPanel);
		new DoubleSlider("Title 10", "Tooltip helptext", sizerControls, scrollPanel);

		scrollPanel->SetScrollRate(0, wxSystemSettings::GetMetric(wxSYS_VSCROLL_ARROW_Y, &_parentPanel)); // Enable vertical scrolling.

		// Init splitter
        splitter->SetMinimumPaneSize(50);
        splitter->SplitVertically(leftPane, rightPane, 150);

		// Setup
		wxBoxSizer* sizerPadding = new wxBoxSizer(wxVERTICAL);
		sizerPadding->Add(sizerParent, 1, wxEXPAND | wxALL);
	    _parentPanel.SetSizer(sizerPadding);
	    _parentPanel.Layout();
	}
}
