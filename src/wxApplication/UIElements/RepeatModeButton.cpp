/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2025 Jasmin Rutic (bytespiller@gmail.com)
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
		_buttonStates(buttonStates),
		_extraOptionsHandler(extraOptionsHandler)
	{
		UpdateTooltip();

		Bind(wxEVT_CONTEXT_MENU, &OnContextMenuOpen, this);
	}

	void RepeatModeButton::SetRepeatMode(RepeatMode forceMode)
	{
		SetActiveImage(static_cast<int>(forceMode) - 1);
		UpdateTooltip();
	}

	RepeatModeButton::RepeatMode RepeatModeButton::GetRepeatMode() const
	{
		return static_cast<RepeatMode>(GetActiveImage() + 1);
	}

	RepeatModeButton::RepeatMode RepeatModeButton::Cycle()
	{
		int newIndex = GetActiveImage() + 1;
		newIndex = (newIndex > _buttonStates.size() - 1) ? 0 : newIndex;

		SetActiveImage(newIndex);
		UpdateTooltip();

		const RepeatMode newMode = GetRepeatMode();
		if (newMode != RepeatMode::Undefined && !_buttonStates.at(newMode).enabled)
		{
			if (std::any_of(_buttonStates.begin(), _buttonStates.end(), [](const auto& kvp) { return kvp.second.enabled; }))
			{
				return Cycle();
			}
		}

		return GetRepeatMode();
	}

	void RepeatModeButton::SetRepeatModeOptionEnabled(RepeatMode mode, bool enabled)
	{
		_buttonStates.at(mode).enabled = enabled;
	}

	void RepeatModeButton::SetExtraOptionEnabled(ExtraOptionsHandler::ExtraOptionId optionId, bool enabled)
	{
		_extraOptionsHandler->extraOptions.at(optionId).enabled = enabled;
	}

	void RepeatModeButton::UpdateTooltip()
	{
		const wxString& tooltipText = _buttonStates.at(GetRepeatMode()).text;
		if (tooltipText.IsEmpty())
		{
			UnsetToolTip();
		}
		else
		{
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

		const RepeatMode activeRepeatMode = GetRepeatMode();
		for (const auto& [mode, state] : _buttonStates)
		{
			wxMenuItem* const item = menu->AppendRadioItem(static_cast<int>(mode), state.text);
			item->Check(activeRepeatMode == mode);
			item->Enable(state.enabled);
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
		const bool isModeRadioButton = evt.GetId() <= _buttonStates.size();
		if (isModeRadioButton)
		{
			SetRepeatMode(static_cast<RepeatMode>(evt.GetId()));

			wxCommandEvent* newEvt = new wxCommandEvent(wxEVT_BUTTON);
			newEvt->SetInt(evt.GetId());
			wxQueueEvent(this, newEvt);
		}
		else // ExtraOption
		{
			const int extraId = evt.GetId() - _buttonStates.size() - 1;
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
