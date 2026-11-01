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

	static wxSlider* AddSlider(const wxString& label, const wxString& tooltip, wxBoxSizer* sizer, wxWindow* parent)
	{
		const wxString tooltipText = wxString::Format("%s: %s", label, tooltip);

		// Title
		wxStaticText* caption = new wxStaticText(parent, wxID_ANY, label);
		caption->SetToolTip(tooltipText);
		caption->SetCursor(wxCURSOR_QUESTION_ARROW);
		sizer->Add(caption, 0, wxLEFT | wxRIGHT | wxTOP, 5);

		// Slider
		wxSlider* const slider = static_cast<wxSlider*>(sizer->Add(new wxSlider(parent, wxID_ANY, 25, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_VALUE_LABEL), 1, wxEXPAND, 0)->GetWindow());
		slider->SetToolTip(tooltipText);

		slider->Bind(wxEVT_MOUSEWHEEL, [](wxMouseEvent& event)
		{
			// Prevent scroll wheel altering the slider, and allow the parent scrollable container to scroll instead
			if (wxWindow* const parent = dynamic_cast<wxWindow*>(event.GetEventObject())->GetParent())
			{
				wxMouseEvent parentEvent(event);
        		parent->GetEventHandler()->ProcessEvent(parentEvent);
			}
		});

		return slider;
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

		AddSlider("Title 1", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 2", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 3", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 4", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 5", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 6", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 7", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 8", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 9", "Tooltip helptext", sizerControls, scrollPanel);
		AddSlider("Title 10", "Tooltip helptext", sizerControls, scrollPanel);

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
