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

#include "ThemeColor.h"
#include "../Config/UIStrings.h"
#include <unordered_map>

namespace
{
	const std::unordered_map<std::string, wxSystemColour> wxSysColourMap
	{
		{"wxSYS_COLOUR_SCROLLBAR", wxSYS_COLOUR_SCROLLBAR},
		{"wxSYS_COLOUR_DESKTOP", wxSYS_COLOUR_DESKTOP},
		{"wxSYS_COLOUR_ACTIVECAPTION", wxSYS_COLOUR_ACTIVECAPTION},
		{"wxSYS_COLOUR_INACTIVECAPTION", wxSYS_COLOUR_INACTIVECAPTION},
		{"wxSYS_COLOUR_MENU", wxSYS_COLOUR_MENU},
		{"wxSYS_COLOUR_WINDOW", wxSYS_COLOUR_WINDOW},
		{"wxSYS_COLOUR_WINDOWFRAME", wxSYS_COLOUR_WINDOWFRAME},
		{"wxSYS_COLOUR_MENUTEXT", wxSYS_COLOUR_MENUTEXT},
		{"wxSYS_COLOUR_WINDOWTEXT", wxSYS_COLOUR_WINDOWTEXT},
		{"wxSYS_COLOUR_CAPTIONTEXT", wxSYS_COLOUR_CAPTIONTEXT},
		{"wxSYS_COLOUR_ACTIVEBORDER", wxSYS_COLOUR_ACTIVEBORDER},
		{"wxSYS_COLOUR_INACTIVEBORDER", wxSYS_COLOUR_INACTIVEBORDER},
		{"wxSYS_COLOUR_APPWORKSPACE", wxSYS_COLOUR_APPWORKSPACE},
		{"wxSYS_COLOUR_HIGHLIGHT", wxSYS_COLOUR_HIGHLIGHT},
		{"wxSYS_COLOUR_HIGHLIGHTTEXT", wxSYS_COLOUR_HIGHLIGHTTEXT},
		{"wxSYS_COLOUR_BTNFACE", wxSYS_COLOUR_BTNFACE},
		{"wxSYS_COLOUR_BTNSHADOW", wxSYS_COLOUR_BTNSHADOW},
		{"wxSYS_COLOUR_GRAYTEXT", wxSYS_COLOUR_GRAYTEXT},
		{"wxSYS_COLOUR_BTNTEXT", wxSYS_COLOUR_BTNTEXT},
		{"wxSYS_COLOUR_INACTIVECAPTIONTEXT", wxSYS_COLOUR_INACTIVECAPTIONTEXT},
		{"wxSYS_COLOUR_BTNHIGHLIGHT", wxSYS_COLOUR_BTNHIGHLIGHT},
		{"wxSYS_COLOUR_3DDKSHADOW", wxSYS_COLOUR_3DDKSHADOW},
		{"wxSYS_COLOUR_3DLIGHT", wxSYS_COLOUR_3DLIGHT},
		{"wxSYS_COLOUR_INFOTEXT", wxSYS_COLOUR_INFOTEXT},
		{"wxSYS_COLOUR_INFOBK", wxSYS_COLOUR_INFOBK},
		{"wxSYS_COLOUR_LISTBOX", wxSYS_COLOUR_LISTBOX},
		{"wxSYS_COLOUR_HOTLIGHT", wxSYS_COLOUR_HOTLIGHT},
		{"wxSYS_COLOUR_GRADIENTACTIVECAPTION", wxSYS_COLOUR_GRADIENTACTIVECAPTION},
		{"wxSYS_COLOUR_GRADIENTINACTIVECAPTION", wxSYS_COLOUR_GRADIENTINACTIVECAPTION},
		{"wxSYS_COLOUR_MENUHILIGHT", wxSYS_COLOUR_MENUHILIGHT},
		{"wxSYS_COLOUR_MENUBAR", wxSYS_COLOUR_MENUBAR},
		{"wxSYS_COLOUR_LISTBOXTEXT", wxSYS_COLOUR_LISTBOXTEXT},
		{"wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT", wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT},
		{"wxSYS_COLOUR_MAX", wxSYS_COLOUR_MAX},
		// synonyms
		{"wxSYS_COLOUR_BACKGROUND", wxSYS_COLOUR_DESKTOP},
		{"wxSYS_COLOUR_3DFACE", wxSYS_COLOUR_BTNFACE},
		{"wxSYS_COLOUR_3DSHADOW", wxSYS_COLOUR_BTNSHADOW},
		{"wxSYS_COLOUR_BTNHILIGHT", wxSYS_COLOUR_BTNHIGHLIGHT},
		{"wxSYS_COLOUR_3DHIGHLIGHT", wxSYS_COLOUR_BTNHIGHLIGHT},
		{"wxSYS_COLOUR_3DHILIGHT", wxSYS_COLOUR_BTNHIGHLIGHT},
		{"wxSYS_COLOUR_FRAMEBK", wxSYS_COLOUR_BTNFACE}
	};

	wxSystemColour GetWxSysColor(const std::string& value)
	{
		assert(wxSysColourMap.count(value) == 1);
		return wxSysColourMap.at(value);
	}
}

namespace ThemeColor
{
	wxColor AsWxColor(Type type, const std::string& value)
	{
		switch (type)
		{
			case Type::wxSYS:
				return wxSystemSettings::GetColour(GetWxSysColor(value));
				break;

			case Type::Val:
				return wxColour(value);
				break;

			default:
				throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
				break;
		}
	}
}
