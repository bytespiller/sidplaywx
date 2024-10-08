/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "ElementsPlaybackMods.h"
#include "../../Config/UIStrings.h"
#include <wx/statline.h>

namespace FrameElements // Static functions.
{
	static constexpr int BOX_BORDER_SIZE = 2;

	static wxCheckBox* NewVoiceCheckBox(wxStaticBoxSizer& parent, const wxString& label, unsigned int sidNum, unsigned int voice)
	{
	    wxCheckBox* chkBox = new wxCheckBox(parent.GetStaticBox(), wxID_ANY, label);
		chkBox->SetRefData(new ElementsPlaybackMods::VoiceCheckBoxData(*chkBox, sidNum, voice));
	    parent.Add(chkBox, 0, wxEXPAND, 0);
		return chkBox;
	}

	static wxStaticLine* NewHorizontalLine(wxStaticBoxSizer& parent)
	{
		parent.Add(0, BOX_BORDER_SIZE * 2);
		wxStaticLine* line = new wxStaticLine(parent.GetStaticBox());
		parent.Add(line, 0, wxEXPAND, 0);
		parent.Add(0, BOX_BORDER_SIZE * 2);
		return line;
	}

	static wxStaticText* NewActiveSidLabel(wxStaticBoxSizer& parent)
	{
		parent.Add(0, 0, 1, 0, 0);
		wxStaticText* retLabel = new wxStaticText(parent.GetStaticBox(), wxID_ANY, Strings::PlaybackMods::VOICE_ACTIVE, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL | wxST_NO_AUTORESIZE);
		retLabel->Enable(false);
		parent.Add(retLabel, 0, wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 0);
		return retLabel;
	}
}

namespace FrameElements
{
	namespace Toggle
	{
		static constexpr unsigned int VOICE1 = 0;
		static constexpr unsigned int VOICE2 = 1;
		static constexpr unsigned int VOICE3 = 2;
		static constexpr unsigned int DIGI = 3;
		static constexpr unsigned int FILTER = 4;
	}

	static constexpr int PLAYBACK_SPEED_NORMAL_PERCENTAGE = 100;

	ElementsPlaybackMods::ElementsPlaybackMods(wxPanel& panel) :
		_parentPanel(panel)
	{
	    wxBoxSizer* sizerParent = new wxBoxSizer(wxVERTICAL);
	    wxStaticBoxSizer* sizerChannels = new wxStaticBoxSizer(wxHORIZONTAL, &_parentPanel, Strings::PlaybackMods::SID_VOICES_TITLE);
	    sizerParent->Add(sizerChannels, 1, wxEXPAND, 0);

		// SID 1
		{
			wxStaticBoxSizer* sizerVoices1 = new wxStaticBoxSizer(wxVERTICAL, sizerChannels->GetStaticBox(), wxString::Format("%s %i", Strings::PlaybackMods::SID_TITLE, 1));
			sizerChannels->Add(sizerVoices1, 1, wxEXPAND | wxTOP | wxRIGHT, BOX_BORDER_SIZE);

			static constexpr unsigned int SID1 = 0;
			chkSid1_1 = NewVoiceCheckBox(*sizerVoices1, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 1), SID1, Toggle::VOICE1);
			chkSid1_2 = NewVoiceCheckBox(*sizerVoices1, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 2), SID1, Toggle::VOICE2);
			chkSid1_3 = NewVoiceCheckBox(*sizerVoices1, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 3), SID1, Toggle::VOICE3);

			NewHorizontalLine(*sizerVoices1);
			chkSid1_digi = NewVoiceCheckBox(*sizerVoices1, Strings::PlaybackMods::SID_DIGI, SID1, Toggle::DIGI);
			chkSid1_filter = NewVoiceCheckBox(*sizerVoices1, Strings::PlaybackMods::SID_FILTER, SID1, Toggle::FILTER);

			labelActiveSid1 = NewActiveSidLabel(*sizerVoices1);
		}

		// SID 2
		{
			wxStaticBoxSizer* sizerVoices2 = new wxStaticBoxSizer(wxVERTICAL, sizerChannels->GetStaticBox(), wxString::Format("%s %i", Strings::PlaybackMods::SID_TITLE, 2));
			sizerChannels->Add(sizerVoices2, 1, wxEXPAND | wxTOP | wxLEFT, BOX_BORDER_SIZE);

			static constexpr unsigned int SID2 = 1;
			chkSid2_1 = NewVoiceCheckBox(*sizerVoices2, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 1), SID2, Toggle::VOICE1);
			chkSid2_2 = NewVoiceCheckBox(*sizerVoices2, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 2), SID2, Toggle::VOICE2);
			chkSid2_3 = NewVoiceCheckBox(*sizerVoices2, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 3), SID2, Toggle::VOICE3);

			NewHorizontalLine(*sizerVoices2);
			chkSid2_digi = NewVoiceCheckBox(*sizerVoices2, Strings::PlaybackMods::SID_DIGI, SID2, Toggle::DIGI);
			chkSid2_filter = NewVoiceCheckBox(*sizerVoices2, Strings::PlaybackMods::SID_FILTER, SID2, Toggle::FILTER);

			labelActiveSid2 = NewActiveSidLabel(*sizerVoices2);
		}

		// SID 3
		{
			wxStaticBoxSizer* sizerVoices3 = new wxStaticBoxSizer(wxVERTICAL, sizerChannels->GetStaticBox(), wxString::Format("%s %i", Strings::PlaybackMods::SID_TITLE, 3));
			sizerChannels->Add(sizerVoices3, 1, wxEXPAND | wxTOP | wxLEFT, BOX_BORDER_SIZE);

			static constexpr unsigned int SID3 = 2;
			chkSid3_1 = NewVoiceCheckBox(*sizerVoices3, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 1), SID3, Toggle::VOICE1);
			chkSid3_2 = NewVoiceCheckBox(*sizerVoices3, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 2), SID3, Toggle::VOICE2);
			chkSid3_3 = NewVoiceCheckBox(*sizerVoices3, wxString::Format("%s %i", Strings::PlaybackMods::SID_VOICE, 3), SID3, Toggle::VOICE3);

			NewHorizontalLine(*sizerVoices3);
			chkSid3_digi = NewVoiceCheckBox(*sizerVoices3, Strings::PlaybackMods::SID_DIGI, SID3, Toggle::DIGI);
			chkSid3_filter = NewVoiceCheckBox(*sizerVoices3, Strings::PlaybackMods::SID_FILTER, SID3, Toggle::FILTER);

			labelActiveSid3 = NewActiveSidLabel(*sizerVoices3);
		}

		// Pre-render active label
		labelPreRenderActive = new wxStaticText(&_parentPanel, wxID_ANY, Strings::PlaybackMods::VOICES_UNAVAILABLE_PRERENDER, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END);

		{
			wxFont font = labelPreRenderActive->GetFont().MakeBold();
			font.SetPointSize(10);
			labelPreRenderActive->SetFont(font);
			sizerParent->Add(labelPreRenderActive, 0, wxALIGN_CENTER_HORIZONTAL);
			sizerParent->AddSpacer(10);
		}

		// Playback speed
	    wxStaticBoxSizer* sizer_6 = new wxStaticBoxSizer(wxVERTICAL, &_parentPanel, Strings::PlaybackMods::SPEED_SLIDER);
	    sizerParent->Add(sizer_6, 0, wxEXPAND | wxTOP, BOX_BORDER_SIZE);
	    sliderPlaybackSpeed = new wxSlider(sizer_6->GetStaticBox(), wxID_ANY, 100, 50, 200, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
		sliderPlaybackSpeed->SetPageSize(5);
	    sizer_6->Add(sliderPlaybackSpeed, 0, wxEXPAND, 0);

		// Setup
	    _parentPanel.SetSizer(sizerParent);
	    _parentPanel.Layout();

		// Simple UI bindings
		for (wxCheckBox* const chk : GetVoiceCheckBoxes())
		{
			chk->Bind(wxEVT_CONTEXT_MENU, &OnVoiceCheckboxContextMenuOpen, this);
		}

		sliderPlaybackSpeed->Bind(wxEVT_CONTEXT_MENU, &OnSliderPlaybackSpeedContextMenuOpen, this);
	}

	void ElementsPlaybackMods::SetAllCheckBoxesState(bool checked)
	{
		for (wxCheckBox* const chkBox : GetVoiceCheckBoxes())
		{
			chkBox->SetValue(checked); // This won't trigger an event.
		}
	}

	void ElementsPlaybackMods::SetActiveSidsIndicator(unsigned int count)
	{
		unsigned int index = 0;
		for (wxStaticText* label : {labelActiveSid1, labelActiveSid2, labelActiveSid3})
		{
			++index;
			label->Show(index <= count);
		}

		_parentPanel.Layout();
	}

	std::vector<wxCheckBox*> ElementsPlaybackMods::GetVoiceCheckBoxes()
	{
		return {chkSid1_1, chkSid1_2, chkSid1_3, chkSid2_1, chkSid2_2, chkSid2_3, chkSid3_1, chkSid3_2, chkSid3_3};
	}

	std::vector<wxCheckBox*> ElementsPlaybackMods::GetDigiCheckBoxes()
	{
		return {chkSid1_digi, chkSid2_digi, chkSid3_digi};
	}

	std::vector<wxCheckBox*> ElementsPlaybackMods::GetFilterCheckBoxes()
	{
		return {chkSid1_filter, chkSid2_filter, chkSid3_filter};
	}

	void ElementsPlaybackMods::OnVoiceCheckboxContextMenuOpen(wxContextMenuEvent& evt)
	{
		wxCheckBox* targetCheckBox = dynamic_cast<wxCheckBox*>(evt.GetEventObject());

		wxMenu* menu = new wxMenu();
		menu->SetRefData(targetCheckBox->GetRefData());

 		menu->Append(wxID_ANY, wxString::Format("%s %s", Strings::PlaybackMods::VOICE_MENU_ITEM_SOLO, targetCheckBox->GetLabelText()));

		menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &OnVoiceCheckboxPopupMenuSoloClick, this);
		targetCheckBox->PopupMenu(menu);
	}

	void ElementsPlaybackMods::OnVoiceCheckboxPopupMenuSoloClick(wxCommandEvent& evt)
	{
		const VoiceCheckBoxData* const activeData = dynamic_cast<VoiceCheckBoxData*>(evt.GetEventObject()->GetRefData());
		assert(activeData != nullptr);

		for (wxCheckBox* chkBox : GetVoiceCheckBoxes())
		{
			const VoiceCheckBoxData* const otherData = dynamic_cast<VoiceCheckBoxData*>(chkBox->GetRefData());
			assert(otherData != nullptr);

			if (activeData->sidNum == otherData->sidNum)
			{
				otherData->checkBoxObject.get().SetValue(activeData->voice == otherData->voice); // This won't trigger an event.

				wxCommandEvent evtNew(wxEVT_CHECKBOX, evt.GetId());
				evtNew.SetEventObject(chkBox);
				evtNew.SetInt(chkBox->IsChecked()); // evt.IsChecked() uses the same int.
				chkBox->GetEventHandler()->AddPendingEvent(evtNew);
			}
		}
	}

	void ElementsPlaybackMods::OnSliderPlaybackSpeedContextMenuOpen(wxContextMenuEvent& /*evt*/)
	{
		wxMenu menu;
		menu.Append(wxID_ANY, Strings::PlaybackMods::SPEED_SLIDER_MENU_ITEM_RESET);

		menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &OnSliderPlaybackSpeedResetClick, this);
		sliderPlaybackSpeed->PopupMenu(&menu);
	}

	void ElementsPlaybackMods::OnSliderPlaybackSpeedResetClick(wxCommandEvent& evt)
	{
		sliderPlaybackSpeed->SetValue(PLAYBACK_SPEED_NORMAL_PERCENTAGE); // This won't trigger an event.

		// Emit event
		wxCommandEvent evtNew(wxEVT_SCROLL_CHANGED, evt.GetId());
		evtNew.SetEventObject(sliderPlaybackSpeed);
		evtNew.SetInt(PLAYBACK_SPEED_NORMAL_PERCENTAGE);
		sliderPlaybackSpeed->GetEventHandler()->AddPendingEvent(evtNew);
	}
}
