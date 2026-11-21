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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "FilterWidgetBase.h"

class wxChoice;

namespace FrameElements
{
	class Dropdown : public FilterWidgetBase
	{
	public:
		Dropdown() = delete;
		Dropdown(const char* uid, const wxString& label, const wxString& tooltip, wxWindow* parent, const wxArrayString& choices, int initialValue = 0);

	public:
		void Enable(bool enabled = true) override;

	public:
		Settings::Option::Var GetValue() const override;

		void SetValue(Settings::Option::Var value) override;
		void SetValue(int value);

	private:
		wxChoice* _dropdown;
	};
}
