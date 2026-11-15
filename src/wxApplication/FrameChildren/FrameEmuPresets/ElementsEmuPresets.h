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

#include "Widgets/FilterWidgetBase.h"
#include <vector>

namespace FrameElements
{
	class CollapsiblePane;

	class ElementsEmuPresets
	{
	public:
		ElementsEmuPresets() = delete;
		ElementsEmuPresets(const ElementsEmuPresets&) = delete;
		ElementsEmuPresets& operator=(const ElementsEmuPresets&) = delete;

	    explicit ElementsEmuPresets(wxPanel& panel);

	public:
		FilterWidgetBase* TryGetWidget(const char* uid);
		std::vector<FrameElements::FilterWidgetBase*> GetAllWidgets();

	public:
		wxListBox* presetsList = nullptr;
		wxButton* btnPresetAdd = nullptr;
		wxButton* btnPresetRemove = nullptr;

	private:
		/// @brief Helper to remember the pane pointer & return it in one line.
		CollapsiblePane* RememberCollapsiblePane(CollapsiblePane* newPanePtr);

	private:
		wxPanel& _parentPanel;
		std::vector<CollapsiblePane*> _widgetHolders;
	};
}
