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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

namespace FrameElements
{
	class ElementsPlaybackMods
	{
	public:
		struct VoiceCheckBoxData : wxObjectRefData
		{
			VoiceCheckBoxData() = delete;
			VoiceCheckBoxData(const VoiceCheckBoxData&) = delete;

			VoiceCheckBoxData(wxCheckBox& aCheckBox, unsigned int aSidNum, unsigned int aVoice) :
				checkBoxObject(aCheckBox),
				sidNum(aSidNum),
				voice(aVoice)
			{

			}

			std::reference_wrapper<wxCheckBox> checkBoxObject;
			const unsigned int sidNum = 0;
			const unsigned int voice = 0;
		};

	public:
		ElementsPlaybackMods() = delete;
		ElementsPlaybackMods(const ElementsPlaybackMods&) = delete;
		ElementsPlaybackMods& operator=(const ElementsPlaybackMods&) = delete;

	    explicit ElementsPlaybackMods(wxPanel& panel);

	public:
		std::vector<wxCheckBox*> GetVoiceCheckBoxes();
		std::vector<wxCheckBox*> GetDigiCheckBoxes();
		std::vector<wxCheckBox*> GetFilterCheckBoxes();
		void SetAllCheckBoxesState(bool checked);
		void SetActiveSidsIndicator(unsigned int count);

	private: // Event handlers
		void OnVoiceCheckboxContextMenuOpen(wxContextMenuEvent& evt);
		void OnVoiceCheckboxPopupMenuSoloClick(wxCommandEvent& evt);

		void OnSliderPlaybackSpeedContextMenuOpen(wxContextMenuEvent& evt);
		void OnSliderPlaybackSpeedResetClick(wxCommandEvent& evt);

	public:
	    wxCheckBox* chkSid1_1;
	    wxCheckBox* chkSid1_2;
	    wxCheckBox* chkSid1_3;
		wxCheckBox* chkSid1_digi;
		wxCheckBox* chkSid1_filter;
		wxStaticText* labelActiveSid1;

	    wxCheckBox* chkSid2_1;
	    wxCheckBox* chkSid2_2;
	    wxCheckBox* chkSid2_3;
		wxCheckBox* chkSid2_digi;
		wxCheckBox* chkSid2_filter;
		wxStaticText* labelActiveSid2;

	    wxCheckBox* chkSid3_1;
	    wxCheckBox* chkSid3_2;
	    wxCheckBox* chkSid3_3;
		wxCheckBox* chkSid3_digi;
		wxCheckBox* chkSid3_filter;
		wxStaticText* labelActiveSid3;

		wxStaticText* labelPreRenderActive;

	    wxSlider* sliderPlaybackSpeed;

	private:
		wxPanel& _parentPanel;
	};
}
