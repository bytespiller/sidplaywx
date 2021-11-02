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

#include "MenuBar.h"

namespace UIElements
{
    int MenuBar::GetWrappedMenuId(int generatedId) const
    {
		if (_menuIds.count(generatedId) == 0)
		{
			return MENU_ID_UNDEFINED;
		}

		return _menuIds.at(generatedId);
    }

    wxMenuItem* MenuBar::AppendItem(int menuId, const wxString& text, wxMenu* parent, const wxString& helpText, wxItemKind kind)
    {
        wxMenuItem* menuItem = parent->Append(wxID_ANY, text, helpText, kind);
		_menuIds[menuItem->GetId()] = menuId;
        return menuItem;
    }

    wxMenuBar* MenuBar::GetWxMenuBarPtr()
    {
        return static_cast<wxMenuBar*>(this);
    }

    bool MenuBar::AppendMenu(wxMenu* menu, const wxString& caption)
    {
        return wxMenuBar::Append(menu, caption);
    }

    bool MenuBar::Enable(bool enable)
    {
        return wxMenuBar::Enable(enable);
    }
}
