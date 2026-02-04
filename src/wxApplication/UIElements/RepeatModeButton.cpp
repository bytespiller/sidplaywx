/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2026 Jasmin Rutic (bytespiller@gmail.com)
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

		// Prepend top extra options
		if (AddExtraOptions(menu, false))
		{
			menu->AppendSeparator();
		}

		// Repeat mode radio/check items
		const RepeatMode activeRepeatMode = GetRepeatMode();
		for (const auto& [mode, state] : _buttonStates)
		{
			wxMenuItem* const item = menu->AppendRadioItem(static_cast<int>(mode), state.text);
			item->Check(activeRepeatMode == mode);
			item->Enable(state.enabled);
		}

		// Append bottom extra options
		menu->AppendSeparator();
		AddExtraOptions(menu, true);

		// Bind & open
		menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &OnContextMenuItemSelected, this);
		PopupMenu(menu);
	}

	bool RepeatModeButton::AddExtraOptions(wxMenu* menu, bool bottom)
	{
		bool addedSomething = false;

		if (_extraOptionsHandler.has_value())
		{
			for (const auto& pair : _extraOptionsHandler->extraOptions)
			{
				if (pair.second.bottom != bottom)
				{
					continue;
				}

				if (pair.second.separator)
				{
					menu->AppendSeparator();
				}

				const int extraId = static_cast<int>(pair.first);
				if (pair.second.type == ExtraOptionsHandler::Type::Toggle)
				{
					menu->AppendCheckItem(extraId, pair.second.text)->Check(pair.second.enabled);
					addedSomething = true;
				}
				else if (pair.second.type == ExtraOptionsHandler::Type::Action)
				{
					menu->Append(extraId, pair.second.text)->Enable(pair.second.enabled);
					addedSomething = true;
				}
			}
		}

		return addedSomething;
	}

	void RepeatModeButton::OnContextMenuItemSelected(wxCommandEvent& evt)
	{
		const int id = evt.GetId();
		switch (static_cast<RepeatMode>(id))
		{
			case RepeatMode::InfiniteDuration:
			case RepeatMode::Normal:
			case RepeatMode::PlayOnce:
			case RepeatMode::RepeatAll:
			case RepeatMode::RepeatOne:
			{
				SetRepeatMode(static_cast<RepeatMode>(id));

				wxCommandEvent* newEvt = new wxCommandEvent(wxEVT_BUTTON);
				newEvt->SetInt(evt.GetId());
				wxQueueEvent(this, newEvt);
				break;
			}
			default: // ExtraOption
			{
				if (_extraOptionsHandler.has_value())
				{
					const bool valid = std::any_of(_extraOptionsHandler->extraOptions.cbegin(), _extraOptionsHandler->extraOptions.cend(), [id](const std::pair<ExtraOptionsHandler::ExtraOptionId, ExtraOptionsHandler::ExtraOption>& pair)
					{
						return static_cast<int>(pair.first) == id;
					});

					if (valid)
					{
						EmitSignal(SignalsRepeatModeButton::SIGNAL_EXTRA_OPTION_SELECTED, id);
					}
					else
					{
						__builtin_trap(); // Invalid ExtraOption, perhaps a new RepeatMode was added and isn't handled above?
					}
				}
			}
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
