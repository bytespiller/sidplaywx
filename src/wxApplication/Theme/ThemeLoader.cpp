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

#include "ThemeLoader.h"
#include "ThemeXml.h"
#include "XmlSequenceParsers/ThemeImagesParser.h"
#include "XmlSequenceParsers/ThemedElementsParser.h"
#include "ThemeData/ThemeData.h"
#include <wx/xml/xml.h>

namespace ThemeLoader
{
	ThemeData::ThemeData LoadTheme(const std::string& path)
	{
		wxXmlDocument doc;
		if (!doc.Load(path + ThemeXml::FILENAME))
		{
			throw std::runtime_error("Failed to load the theme file!");
		}

		assert(doc.GetRoot()->GetName() == ThemeXml::Sequence::Theme);
		const std::string& themeDisplayName = doc.GetRoot()->GetAttribute(ThemeXml::Attr::Theme::DisplayName).ToStdString();

		// Theme DisplayName
		wxXmlNode* child = doc.GetRoot()->GetChildren();
		assert(child->GetName() == ThemeXml::Sequence::Images); // Temp until feature expansion

		// Load
		std::unique_ptr<ThemeData::ThemeData::ThemeImages> themeImages = std::make_unique<ThemeData::ThemeData::ThemeImages>();
		std::unique_ptr<ThemeData::ThemeData::ThemedElements> themedElements = std::make_unique<ThemeData::ThemeData::ThemedElements>();
		do
		{
			TryParseThemeImages(*child, path, *themeImages);
			TryParseThemedElements(*child, *themedElements);

		} while (child = child->GetNext());

		return ThemeData::ThemeData(themeDisplayName, themeImages, themedElements);
	}
}
