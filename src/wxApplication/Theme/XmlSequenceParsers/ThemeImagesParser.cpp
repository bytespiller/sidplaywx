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

#include "ThemeImagesParser.h"
#include "../ThemeXml.h"
#include "../ThemeData/ThemeImage.h"
#include "../../../Util/HelpersGeneral.h"
#include <wx/xml/xml.h>

namespace ThemeLoader
{
	bool TryParseThemeImages(const wxXmlNode& node, const std::string& themePath, ThemeData::ThemeData::ThemeImages& out)
	{
		if (node.GetName() != ThemeXml::Sequence::Images)
		{
			return false;
		}

		wxXmlNode* child = node.GetChildren();
		if (child == nullptr)
		{
			return false;
		}

		do
		{
			// Sanity checks
			if (child->GetName() != ThemeXml::Element::Image)
			{
				continue;
			}

			const std::string& strType = child->GetAttribute(ThemeXml::Attr::Image::Type).ToStdString();
			if (strType != ThemeXml::Attr::Image::TypeValue::Svg)
			{
				throw std::runtime_error("Image type not supported!"); // Technically we support png etc. but since only the svg is used, let's keep things simple for now...
			}

			// Name
			const std::string& imageName = child->GetAttribute(ThemeXml::Attr::Image::Name).ToStdString();

			// Path
			const std::string& imagePath = themePath + "/" + child->GetAttribute(ThemeXml::Attr::Image::Path).ToStdString();

			// Finalize ThemeImage
			out.emplace_back(ThemeData::ThemeImage(imageName, imagePath));

		} while (child = child->GetNext());

		return true;
	}
}
