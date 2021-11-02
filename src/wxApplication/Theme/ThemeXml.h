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

#pragma once

namespace ThemeXml
{
	const std::string FILENAME = "/theme.xml";

	namespace Sequence
	{
		const std::string Theme = "Theme";
		const std::string Images = "Images";
		const std::string GuiElements = "GuiElements";
		const std::string Colors = "Colors";
	}

	namespace Element
	{
		const std::string Image = "Image";
		const std::string GuiElement = "GuiElement";
		const std::string Color = "Color";
	}

	namespace Attr
	{
		namespace Theme
		{
			constexpr const char* const DisplayName = "displayName";
		}

		namespace Image
		{
			namespace TypeValue
			{
				const std::string Svg = "svg";
			}

			constexpr const char* const Type = "type";
			constexpr const char* const Offset = "offset";
			constexpr const char* const Scale = "scale";
			constexpr const char* const Name = "name";
			constexpr const char* const Path = "path";
		}

		namespace GuiElement
		{
			constexpr const char* const Name = "name";
		}

		namespace Color
		{
			namespace TypeValue
			{
				const std::string wxSYS = "wxSYS";
				const std::string Val = "Val";
			}

			constexpr const char* const Type = "type";
			constexpr const char* const Value = "value";
			constexpr const char* const Property = "property";
		}
	}
}
