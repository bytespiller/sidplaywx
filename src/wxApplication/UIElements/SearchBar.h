/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023 Jasmin Rutic (bytespiller@gmail.com)
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

#include "../../Util/SimpleTimer.h"
#include "../../Util/SimpleSignal/SimpleSignalProvider.h"

namespace UIElements
{
	enum class SignalsSearchBar
	{
		SIGNAL_FIND_PREV,
		SIGNAL_FIND_NEXT
	};

	class SearchBar: public SimpleSignalProvider<SignalsSearchBar>
	{
	public:
		explicit SearchBar(wxWindow* parent, bool visible = true);

		wxPanel* const GetPanel() const;

		bool IsShown() const;
		void Show(bool show = true);
		void Hide();

		/// @brief Toggles the SearchBar visibility.
		/// @return New visibility state.
		bool ToggleVisibility();

		void SelectAll() const;
		void SelectNone() const;
		void Clear() const;

		/// @brief Returns trimmed lowercase user query string.
		wxString GetQuery() const;

		void FlashInputBox(const wxColour& colour);

	private:
		wxPanel* _panel = nullptr;
		wxBoxSizer* _sizerFind = nullptr;
		wxTextCtrl* _txtInput = nullptr;
		wxButton* _btnFindPrev = nullptr;
		wxButton* _btnFindNext = nullptr;

		wxColour _originalTxtInputColour;
		std::unique_ptr<SimpleTimer> _flashInputTimer;
	};
}
