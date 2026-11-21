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

#include "CollapsiblePane.h"
#include "Widgets/DoubleSlider.h"
#include "Widgets/Dropdown.h"
#include "../../Config/AppSettings.h"
#include "../../Config/UIStrings.h"

#include <wx/artprov.h>
#include <wx/splitter.h>

static constexpr int VERTICAL_SPACER = 10;

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
		presetsList = new wxListBox(leftPane, wxID_ANY, wxDefaultPosition, wxDefaultSize, {}, wxLB_SINGLE | wxWANTS_CHARS);
		sizerLeftPane->Add(presetsList, 1, wxEXPAND | wxALL, 0);
		leftPane->SetSizer(sizerLeftPane);

		wxBoxSizer* sizerPresetsButtons = new wxBoxSizer(wxHORIZONTAL);
		sizerLeftPane->Add(sizerPresetsButtons, 0);

		btnPresetAdd = new wxButton(leftPane, wxID_ANY);
		btnPresetAdd->SetBitmapCurrent(wxArtProvider::GetBitmap(wxART_PLUS, wxART_BUTTON));
		sizerPresetsButtons->Add(btnPresetAdd);

		btnPresetRemove = new wxButton(leftPane, wxID_ANY);
		btnPresetRemove->SetBitmapCurrent(wxArtProvider::GetBitmap(wxART_MINUS, wxART_BUTTON));
		btnPresetRemove->Disable();
		sizerPresetsButtons->Add(btnPresetRemove);

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
		CollapsiblePane* const collpaneCommon = RememberCollapsiblePane(new CollapsiblePane(Strings::EmuPresets::CAT_COMMON, sizerControls, scrollPanel));
		{
			wxPanel* const parent = collpaneCommon->GetContentPanel();
			std::vector<std::unique_ptr<FilterWidgetBase>> widgets;

			widgets.emplace_back(std::make_unique<Dropdown>(Settings::AppSettings::ID::DefaultC64Model, Strings::Preferences::OPT_DEFAULT_C64_MODEL, wxEmptyString, parent,
				wxArrayString
				{
					Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_PAL,
					Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_NTSC,
					Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_OLD_NTSC,
					Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_DREAN,
					Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_PAL_M
				}
			));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			widgets.emplace_back(std::make_unique<Dropdown>(Settings::AppSettings::ID::DefaultSidModel, Strings::Preferences::OPT_DEFAULT_SID_MODEL, wxEmptyString, parent,
				wxArrayString
				{
					Strings::Preferences::ITEM_DEFAULT_SID_MODEL_FORCE_6581,
					Strings::Preferences::ITEM_DEFAULT_SID_MODEL_FORCE_8580,
				}
			));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			widgets.emplace_back(std::make_unique<Dropdown>(Settings::AppSettings::ID::CombinedWaveformsStrength, Strings::Preferences::OPT_COMBINED_WAVEFORMS_STRENGTH, Strings::Preferences::DESC_COMBINED_WAVEFORMS_STRENGTH, parent,
				wxArrayString
				{
					Strings::Preferences::ITEM_COMBINED_WAVEFORMS_STRENGTH_AVERAGE,
					Strings::Preferences::ITEM_COMBINED_WAVEFORMS_STRENGTH_WEAK,
					Strings::Preferences::ITEM_COMBINED_WAVEFORMS_STRENGTH_STRONG,
				}
			));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::DacLeakage, Strings::Preferences::OPT_DAC_LEAKAGE, Strings::Preferences::DESC_DAC_LEAKAGE, parent, 1.0));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			collpaneCommon->AdoptWidgets(widgets);
		}

		// 6581
		CollapsiblePane* const collpane6581 = RememberCollapsiblePane(new CollapsiblePane(Strings::EmuPresets::CAT_6581, sizerControls, scrollPanel));
		{
			wxPanel* const parent = collpane6581->GetContentPanel();
			std::vector<std::unique_ptr<FilterWidgetBase>> widgets;

			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::FilterCurve6581, Strings::Preferences::OPT_FILTER_CURVE_6581, Strings::Preferences::DESC_FILTER_CURVE, parent, 0.5));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::FilterRange6581, Strings::Preferences::OPT_FILTER_RANGE_6581, Strings::Preferences::DESC_FILTER_RANGE, parent, 0.5));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::WaveOffset6581, Strings::Preferences::OPT_WAVE_OFFSET_6581, Strings::Preferences::DESC_WAVE_OFFSET_6581, parent, 0));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			widgets.emplace_back(std::make_unique<Dropdown>(Settings::AppSettings::ID::Old6581caps, Strings::Preferences::OPT_OLD_CAPS, Strings::Preferences::DESC_OLD_CAPS, parent,
				wxArrayString
				{
					Strings::EmuPresets::OPT_DISABLED,
					Strings::EmuPresets::OPT_ENABLED
				}
			));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			collpane6581->AdoptWidgets(widgets);
		}

		// 8580
		CollapsiblePane* const collpane8580 = RememberCollapsiblePane(new CollapsiblePane(Strings::EmuPresets::CAT_8580, sizerControls, scrollPanel));
		{
			wxPanel* const parent = collpane8580->GetContentPanel();
			std::vector<std::unique_ptr<FilterWidgetBase>> widgets;

			widgets.emplace_back(std::make_unique<DoubleSlider>(Settings::AppSettings::ID::FilterCurve8580, Strings::Preferences::OPT_FILTER_CURVE_8580, Strings::Preferences::DESC_FILTER_CURVE, parent, 0.5));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			widgets.emplace_back(std::make_unique<Dropdown>(Settings::AppSettings::ID::DigiBoost, Strings::Preferences::OPT_DIGIBOOST, Strings::Preferences::DESC_DIGIBOOST, parent,
				wxArrayString
				{
					Strings::EmuPresets::OPT_DISABLED,
					Strings::EmuPresets::OPT_ENABLED
				}
			));
			parent->GetSizer()->AddSpacer(VERTICAL_SPACER);

			collpane8580->AdoptWidgets(widgets);
		}

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

	FilterWidgetBase* ElementsEmuPresets::TryGetWidget(const char* uid)
	{
		for (FrameElements::CollapsiblePane* pane : _widgetHolders)
		{
			if (FrameElements::FilterWidgetBase* widget = pane->TryGetWidget(uid))
			{
				return widget;
			}
		}
		return nullptr;
	}

	std::vector<FrameElements::FilterWidgetBase*> ElementsEmuPresets::GetAllWidgets()
	{
		std::vector<FrameElements::FilterWidgetBase*> widgets;
		for (FrameElements::CollapsiblePane* pane : _widgetHolders)
		{
			auto paneWidgets = pane->GetAllWidgets();
			widgets.insert(widgets.end(), paneWidgets.begin(), paneWidgets.end());
		}
		return widgets;
	}

	CollapsiblePane* ElementsEmuPresets::RememberCollapsiblePane(CollapsiblePane* newPanePtr)
	{
		_widgetHolders.push_back(newPanePtr);
		return newPanePtr;
	}
}
