/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "SearchBar.h"
#include "../Config/UIStrings.h"

namespace UIElements
{
	static const int INPUT_BOX_FLASH_DURATION_MS = 250;

	SearchBar::SearchBar(wxWindow* parent, bool visible)
	{
		_panel = new wxPanel(parent);
		_sizerFind = new wxBoxSizer(wxOrientation::wxHORIZONTAL);

		_txtInput = new wxTextCtrl(_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
		_originalTxtInputColour = _txtInput->GetBackgroundColour();

		_btnFindPrev = new wxButton(_panel, wxID_ANY, Strings::FramePlayer::MENU_ITEM_FIND_PREV);
		_btnFindNext = new wxButton(_panel, wxID_ANY, Strings::FramePlayer::MENU_ITEM_FIND_NEXT);

		_sizerFind->Add(_txtInput, 1, wxEXPAND, 0);
		_sizerFind->Add(_btnFindPrev);
		_sizerFind->Add(_btnFindNext);

		_panel->SetSizerAndFit(_sizerFind);

		if (!visible)
		{
			_panel->Hide();
		}

		// Bindings
		_txtInput->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent& /*evt*/)
		{
			const bool shiftPressed = wxGetKeyState(wxKeyCode::WXK_SHIFT);
			EmitSignal((shiftPressed) ? SignalsSearchBar::SIGNAL_FIND_PREV : SignalsSearchBar::SIGNAL_FIND_NEXT);
		});

		_txtInput->Bind(wxEVT_KEY_DOWN, [this](wxKeyEvent& evt)
		{
			if (evt.GetKeyCode() == wxKeyCode::WXK_ESCAPE)
			{
				Hide();
			}

			evt.Skip();
		});

		_btnFindPrev->Bind(wxEVT_BUTTON, [this](wxCommandEvent& /*evt*/)
		{
			EmitSignal(SignalsSearchBar::SIGNAL_FIND_PREV);
		});

		_btnFindNext->Bind(wxEVT_BUTTON, [this](wxCommandEvent& /*evt*/)
		{
			EmitSignal(SignalsSearchBar::SIGNAL_FIND_NEXT);
		});
	}

	wxPanel* const SearchBar::GetPanel() const
	{
		return _panel;
	}

	bool SearchBar::IsShown() const
	{
		return _panel->IsShown();
	}

	void SearchBar::Show(bool show)
	{
		if (show)
		{
			_panel->Show();
			_txtInput->SelectAll();
			_txtInput->SetFocus();
		}
		else
		{
			_panel->Hide();
			_panel->Navigate(wxNavigationKeyEvent::IsBackward); // Restore focus to whatever comes before us (typically it's the Playlist).
		}

		_panel->GetParent()->Layout();
	}

	void SearchBar::Hide()
	{
		Show(false);
	}

	bool SearchBar::ToggleVisibility()
	{
		if (IsShown())
		{
			Hide();
			return false;
		}
		else
		{
			Show();
			return true;
		}
	}

	void SearchBar::SelectAll() const
	{
		_txtInput->SelectAll();
	}

	void SearchBar::SelectNone() const
	{
		_txtInput->SelectNone();
	}

	void SearchBar::Clear() const
	{
		_txtInput->Clear();
	}

	void SearchBar::FlashInputBox(const wxColour& colour)
	{
		_flashInputTimer = nullptr; // Kill timer.

		_txtInput->SetBackgroundColour(colour);
		_txtInput->Refresh();

		_flashInputTimer = std::make_unique<SimpleTimer>(SimpleTimer::Type::OneShot, INPUT_BOX_FLASH_DURATION_MS, [&]()
		{
			if (_txtInput != nullptr)
			{
				_txtInput->SetBackgroundColour(_originalTxtInputColour);
				_txtInput->Refresh();
			}
		});
	}

	wxString SearchBar::GetQuery() const
	{
		return _txtInput->GetValue().Lower().Trim(); // Reminder: Lower() returns a copy, Trim() returns a reference, so this call order is on purpose.
	}
}
