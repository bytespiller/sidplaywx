/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021 Jasmin Rutic (bytespiller@gmail.com)
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

#include "RepeatModeButton.h"

namespace UIElements
{
	RepeatModeButton::RepeatModeButton(const ButtonStates& buttonStates, const wxSize& size, wxPanel& panel, const std::optional<ExtraOptionsHandler>& extraOptionsHandler) :
		MultiImageButton(AsImageList(buttonStates), size, panel),
		_modeImagesCount(static_cast<int>(buttonStates.size())),
		_extraOptionsHandler(extraOptionsHandler)
	{
		_modeTexts.reserve(buttonStates.size());
		for (const auto& state : buttonStates)
		{
			_modeTexts.emplace_back(state.second.text);
		}
		UpdateTooltip();

		Bind(wxEVT_CONTEXT_MENU, &OnContextMenuOpen, this);
	}

	void RepeatModeButton::SetRepeatModeImage(RepeatMode forceMode)
	{
		SetActiveImage(static_cast<int>(forceMode));
		UpdateTooltip();
	}

	RepeatModeButton::RepeatMode RepeatModeButton::GetRepeatMode() const
	{
		return static_cast<RepeatMode>(GetActiveImage() + 1);
	}

	RepeatModeButton::RepeatMode RepeatModeButton::Cycle()
	{
		int newIndex = GetActiveImage() + 1;
		newIndex = (newIndex > _modeImagesCount - 1) ? 0 : newIndex;
		SetActiveImage(newIndex);
		UpdateTooltip();
		return GetRepeatMode();
	}

	void RepeatModeButton::SetModeTexts(const std::vector<wxString>& tooltips)
	{
		assert(tooltips.size() == _modeImagesCount);
		_modeTexts = tooltips;
		UpdateTooltip();
	}

	void RepeatModeButton::SetExtraOptionEnabled(ExtraOptionsHandler::ExtraOptionId optionId, bool enabled)
	{
		_extraOptionsHandler->extraOptions.at(optionId).enabled = enabled;
	}

	void RepeatModeButton::UpdateTooltip()
	{
		if (_modeTexts.empty())
		{
			UnsetToolTip();
		}
		else
		{
			const wxString& tooltipText = _modeTexts.at(GetActiveImage());
			SetToolTip(tooltipText);
		}
	}

	void RepeatModeButton::OnContextMenuOpen(wxContextMenuEvent& /*evt*/)
	{
		if (_extraOptionsHandler.has_value())
		{
			_extraOptionsHandler->menuOpenUpdateCallback(*this);
		}

		wxMenu* menu = new wxMenu();

		const int activeImageIndex = GetActiveImage();
		for (int i = 0; i < _modeTexts.size(); ++i)
		{
 			menu->AppendRadioItem(i, _modeTexts.at(i))->Check(activeImageIndex == i);
		}

		if (_extraOptionsHandler.has_value())
		{
			menu->AppendSeparator();
			const int idOffset = menu->GetMenuItemCount();
			for (const auto& extra : _extraOptionsHandler->extraOptions)
			{
				if (extra.second.separator)
				{
					menu->AppendSeparator();
				}

				const int extraId = static_cast<int>(extra.first);
				menu->AppendCheckItem(idOffset + extraId, extra.second.text)->Check(extra.second.enabled);
			}
		}

		menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &OnContextMenuItemSelected, this);
		PopupMenu(menu);
	}

	void RepeatModeButton::OnContextMenuItemSelected(wxCommandEvent& evt)
	{
		const bool isModeRadioButton = evt.GetId() < _modeImagesCount;
		if (isModeRadioButton)
		{
			SetRepeatModeImage(static_cast<RepeatMode>(evt.GetId()));

			wxCommandEvent* newEvt = new wxCommandEvent(wxEVT_BUTTON);
			newEvt->SetInt(evt.GetId() + 1);
			wxQueueEvent(this, newEvt);
		}
		else // ExtraOption
		{
			const int extraId = evt.GetId() - _modeImagesCount - 1;
			EmitSignal(SignalsRepeatModeButton::SIGNAL_EXTRA_OPTION_SELECTED, extraId);
		}
	}

	RepeatModeButton::ImageList RepeatModeButton::AsImageList(const ButtonStates& buttonStates)
	{
		ImageList il;
		il.reserve(buttonStates.size());
		for (const auto& state : buttonStates)
		{
			il.emplace_back(state.second.themeImage);
		}
		return il;
	}
}
