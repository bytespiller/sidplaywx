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

#include "ThemedElementsParser.h"
#include "ThemeColorsParser.h"
#include "../ThemeXml.h"
#include "../ThemeData/ThemedElementData.h"
#include <wx/xml/xml.h>

namespace ThemeLoader
{
	bool TryParseThemedElements(const wxXmlNode& node, ThemeData::ThemeData::ThemedElements& out)
	{
		if (node.GetName() != ThemeXml::Sequence::GuiElements)
		{
			return false;
		}

		wxXmlNode* guiElementNode = node.GetChildren();
		if (guiElementNode == nullptr)
		{
			return false;
		}

		do
		{
			// Sanity checks
			if (guiElementNode->GetName() != ThemeXml::Element::GuiElement)
			{
				continue;
			}

			wxXmlNode* guiElementChildProps = guiElementNode->GetChildren();
			if (guiElementChildProps == nullptr)
			{
				return false;
			}

			// Parse sub-properties
			ThemeData::ThemedElementData::ColoredProperties colors;
			do
			{
				TryParseColors(*guiElementChildProps, colors);

			} while (guiElementChildProps = guiElementChildProps->GetNext());

			// Name
			const std::string& elementName = guiElementNode->GetAttribute(ThemeXml::Attr::GuiElement::Name).ToStdString();

			// Finalize ThemedElementData
			out.emplace(elementName, ThemeData::ThemedElementData(colors));

		} while (guiElementNode = guiElementNode->GetNext());

		return true;
	}
}
