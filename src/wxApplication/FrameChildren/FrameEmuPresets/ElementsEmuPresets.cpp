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
#include <wx/collpane.h>
#include <wx/splitter.h>

namespace
{
	const wxArrayString items =  // TEMP TEST
	{
		"Chris Huelsbeck",
		"Lasse Oorni"
	};

	struct CollapsiblePane
	{
		wxCollapsiblePane* object = nullptr;
		wxWindow* pane = nullptr;
		wxPanel* content = nullptr;
		wxString captionText;
	};

	constexpr int PANE_SPACER = 10;

	CollapsiblePane CreateCollapsiblePane(const wxString& caption, wxSizer* sizer, wxWindow* parent)
	{
		static constexpr int LABEL_OFFSET = 25; // 25 on MSW, have to check others

		wxCollapsiblePane* collpane = new wxCollapsiblePane(parent, wxID_ANY, caption);
		sizer->Add(collpane, 0, wxEXPAND | wxALL);

		collpane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, [parent](wxCollapsiblePaneEvent &) { parent->Layout(); });
		collpane->GetPane()->SetSizer(new wxBoxSizer(wxVERTICAL));

		// Add internal padding
		wxPanel* contentPanel = new wxPanel(collpane->GetPane(), wxID_ANY);
		wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
		contentPanel->SetSizer(panelSizer);
		collpane->GetPane()->GetSizer()->Add(contentPanel, 1, wxEXPAND | wxLEFT, LABEL_OFFSET);

		collpane->Expand();

		return CollapsiblePane
		{
			.object = collpane,
			.pane = collpane->GetPane(),
			.content = contentPanel,
			.captionText = caption
		};
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
		presetsList = new wxListBox(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, items, wxLB_SINGLE);
		sizerLeftPane->Add(presetsList, 1, wxEXPAND | wxALL, 0);
		leftPane->SetSizer(sizerLeftPane);

		// Controls (right)
        wxPanel* rightPane = new wxPanel(splitter, wxID_ANY);
		wxBoxSizer* sizerRightPane = new wxBoxSizer(wxVERTICAL);
		rightPane->SetSizer(sizerRightPane);

		wxScrolled<wxPanel>* scrollPanel = new wxScrolled<wxPanel>(rightPane, wxID_ANY);
		sizerRightPane->Add(scrollPanel, 1, wxEXPAND | wxALL, 0);

		wxBoxSizer* sizerControls = new wxBoxSizer(wxVERTICAL);
		scrollPanel->SetSizer(sizerControls);

		#pragma region Filter settings

		// Common
		CollapsiblePane collpaneCommon = CreateCollapsiblePane("Common overrides", sizerControls, scrollPanel);
		sliderDacLeakage = std::make_unique<DoubleSlider>(Strings::Preferences::OPT_DAC_LEAKAGE, Strings::Preferences::DESC_DAC_LEAKAGE, collpaneCommon.content, 1.0);

		collpaneCommon.pane->GetSizer()->AddSpacer(PANE_SPACER);

		sliderDacLeakage->Bind(wxEVT_FILTER_WIDGET_TOGGLED, [collpaneCommon](wxCommandEvent& event)
		{
			collpaneCommon.object->SetLabelText(wxString::Format("%s (%i) (TEST)", collpaneCommon.captionText, event.GetInt())); // TEST, REMOVE ME
			collpaneCommon.object->Refresh();
		});

		// 6581
		CollapsiblePane collpane6581 = CreateCollapsiblePane("6581 overrides", sizerControls, scrollPanel);
		sliderFilterCurve6581 = std::make_unique<DoubleSlider>(Strings::Preferences::OPT_FILTER_CURVE_6581, Strings::Preferences::DESC_FILTER_CURVE, collpane6581.content, 0.5);
		sliderFilterRange6581 = std::make_unique<DoubleSlider>(Strings::Preferences::OPT_FILTER_RANGE_6581, Strings::Preferences::DESC_FILTER_RANGE, collpane6581.content, 0.5);
		sliderWaveOffset6581 = std::make_unique<DoubleSlider>(Strings::Preferences::OPT_WAVE_OFFSET_6581, Strings::Preferences::DESC_WAVE_OFFSET_6581, collpane6581.content, 0);

		collpane6581.pane->GetSizer()->AddSpacer(PANE_SPACER);

		// 8580
		CollapsiblePane collpane8580 = CreateCollapsiblePane("8580 overrides", sizerControls, scrollPanel);
		sliderFilterCurve8580 = std::make_unique<DoubleSlider>(Strings::Preferences::OPT_FILTER_CURVE_8580, Strings::Preferences::DESC_FILTER_CURVE, collpane8580.content, 0.5);

		collpane8580.pane->GetSizer()->AddSpacer(PANE_SPACER);

		#pragma endregion

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
