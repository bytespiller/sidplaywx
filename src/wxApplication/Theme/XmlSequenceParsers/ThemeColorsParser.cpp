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

#include "ThemeColorsParser.h"
#include "../ThemeColor.h"
#include "../ThemeXml.h"
#include "../../Config/UIStrings.h"
#include <wx/xml/xml.h>

namespace ThemeLoader
{
	bool TryParseColors(const wxXmlNode& node, ThemeData::ThemedElementData::ColoredProperties& out)
	{
		if (node.GetName() != ThemeXml::Sequence::Colors)
		{
			return false;
		}

		wxXmlNode* child = node.GetChildren();
		do
		{
			if (child->GetName() != ThemeXml::Element::Color)
			{
				continue;
			}

			const wxString& strType = child->GetAttribute(ThemeXml::Attr::Color::Type);
			ThemeColor::Type type;
			if (strType == ThemeXml::Attr::Color::TypeValue::Val)
			{
				type = ThemeColor::Type::Val;
			}
			else if (strType == ThemeXml::Attr::Color::TypeValue::wxSYS)
			{
				type = ThemeColor::Type::wxSYS;
			}
			else
			{
				throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
			}

			out.emplace(child->GetAttribute(ThemeXml::Attr::Color::Property).ToStdString(), ThemeColor::AsWxColor(type, child->GetAttribute(ThemeXml::Attr::Color::Value).ToStdString()));

		} while (child = child->GetNext());

		return true;
	}
}
