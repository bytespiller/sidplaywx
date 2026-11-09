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
#include <wx/splitter.h>

#include "../../Config/AppSettings.h"
#include "../../Config/UIStrings.h"

#include "CollapsiblePane.h"

namespace
{
	const wxArrayString items =  // TEMP TEST
	{
		"Chris Huelsbeck",
		"Lasse Oorni"
	};

	constexpr int PANE_SPACER = 10;
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
		CollapsiblePane* const collpaneCommon = new CollapsiblePane("Common overrides", sizerControls, scrollPanel);
		{
			std::vector<std::unique_ptr<FilterWidgetBase>> widgets;
			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::DacLeakage, Strings::Preferences::OPT_DAC_LEAKAGE, Strings::Preferences::DESC_DAC_LEAKAGE, collpaneCommon->GetContentPanel(), 1.0));
			collpaneCommon->AdoptWidgets(widgets);
		}
		collpaneCommon->GetContentPanel()->GetSizer()->AddSpacer(PANE_SPACER);

		// 6581
		CollapsiblePane* const collpane6581 = new CollapsiblePane("6581 overrides", sizerControls, scrollPanel);
		{
			std::vector<std::unique_ptr<FilterWidgetBase>> widgets;
			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::FilterRange6581, Strings::Preferences::OPT_FILTER_CURVE_6581, Strings::Preferences::DESC_FILTER_CURVE, collpane6581->GetContentPanel(), 0.5));
			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::FilterRange6581, Strings::Preferences::OPT_FILTER_RANGE_6581, Strings::Preferences::DESC_FILTER_RANGE, collpane6581->GetContentPanel(), 0.5));
			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::WaveOffset6581, Strings::Preferences::OPT_WAVE_OFFSET_6581, Strings::Preferences::DESC_WAVE_OFFSET_6581, collpane6581->GetContentPanel(), 0));
			collpane6581->AdoptWidgets(widgets);
		}
		collpane6581->GetPane()->GetSizer()->AddSpacer(PANE_SPACER);

		// 8580
		CollapsiblePane* const collpane8580 = new CollapsiblePane("8580 overrides", sizerControls, scrollPanel);
		{
			std::vector<std::unique_ptr<FilterWidgetBase>> widgets;
			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::FilterCurve8580, Strings::Preferences::OPT_FILTER_CURVE_8580, Strings::Preferences::DESC_FILTER_CURVE, collpane8580->GetContentPanel(), 0.5));
			collpane8580->AdoptWidgets(widgets);
		}
		collpane8580->GetPane()->GetSizer()->AddSpacer(PANE_SPACER);

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
