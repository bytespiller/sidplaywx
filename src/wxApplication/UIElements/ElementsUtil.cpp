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

#include "ElementsUtil.h"

#include "../Helpers/HelpersWx.h"
#include "../Theme/ThemeData/ThemeImage.h"

namespace UIElements
{
	namespace Util // Static functions.
	{
		std::shared_ptr<wxBitmapBundle> LoadColorizedSvg(const char* filename, const wxSize& size, const wxColor* const color)
		{
			const std::unique_ptr<BufferHolder>& data = Helpers::Wx::Files::GetFileContentFromDisk(filename);
			assert(data.get() != nullptr); // File not found.

			wxString svg(reinterpret_cast<const char*>(data->buffer), wxConvUTF8, data->size);

			// Colorize
			{
				wxColor col((color == nullptr) ? wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_CAPTIONTEXT) : *color);
				wxString hexCol(wxString::Format("#%02X%02X%02X", col.Red(), col.Green(),col.Blue()));
				svg.Replace("currentColor", hexCol);
				svg.Replace("#000", hexCol);
			}

			return std::make_shared<wxBitmapBundle>(wxBitmapBundle::FromSVG(svg.c_str(), size));
		}

		wxButton* NewSvgButton(const ThemeData::ThemeImage& themeImage, const wxSize& size, wxPanel& panel)
		{
			wxButton* button = new wxButton(&panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
			button->SetBitmap(*LoadColorizedSvg(themeImage.path.c_str(), size));
			return button;
		}
	}
}
