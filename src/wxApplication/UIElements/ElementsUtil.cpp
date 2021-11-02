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

#include "ElementsUtil.h"

#include <wx/rawbmp.h>

#define NANOSVG_ALL_COLOR_KEYWORDS  // Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION      // Expands implementation
#include <nanosvg.h>

#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>

#include "../Theme/ThemeData/ThemeImage.h"

namespace UIElements
{
	namespace Util // Static functions.
	{
		/*
		wxButton* NewBitmapButton(const char* image, const wxSize& size, wxPanel& panel, const wxPoint& artOffset)
		{
			wxButton* button = new wxButton(&panel, wxID_ANY, wxEmptyString);
			button->SetMinSize(size);
			wxImage playImageScaled = wxBitmap(image, wxBITMAP_TYPE_PNG).ConvertToImage().Scale(button->GetMinClientSize().x - artOffset.x, button->GetMinClientSize().y - artOffset.y);
			button->SetBitmap(playImageScaled);
			return button;
		}
		*/

		std::shared_ptr<wxBitmap> LoadRasterizedSvg(const char* image, const wxSize& size, const wxPoint& artOffset, double scale)
		{
			// Portions from https://forums.wxwidgets.org/viewtopic.php?t=44708#p184852 by doublemax
			const int dpi = wxGetDisplayPPI().GetY();
			NSVGimage* svgImage = nsvgParseFromFile(image, "px", static_cast<float>(dpi));
			assert(svgImage != nullptr); // File not found!

			const int colorChannels = 4;
			unsigned char* imageBuffer = new unsigned char[size.GetWidth() * size.GetHeight() * colorChannels];
			const float calculatedScale = (size.GetWidth() / svgImage->width) * scale;
			NSVGrasterizer* rast = nsvgCreateRasterizer();
			nsvgRasterize(rast, svgImage, artOffset.x * calculatedScale, artOffset.y * calculatedScale, calculatedScale, imageBuffer, size.GetWidth(), size.GetHeight(), size.GetHeight() * colorChannels);

			std::shared_ptr<wxBitmap> destBitmap = std::make_shared<wxBitmap>(size); // Monochrome mode.

			wxAlphaPixelData bmdata(*destBitmap.get());
			wxAlphaPixelData::Iterator dst(bmdata);

			unsigned char* sourceData = imageBuffer;
			for(int y = 0; y < destBitmap->GetHeight(); ++y)
			{
				dst.MoveTo(bmdata, 0, y);
				for(int x = 0; x < destBitmap->GetWidth(); ++x)
				{
					const unsigned char alpha = sourceData[3];
					dst.Blue() = sourceData[2] * alpha / 255;
					dst.Green() = sourceData[1] * alpha / 255;
					dst.Red() = sourceData[0] * alpha / 255;
					dst.Alpha() = alpha;
					++dst;
					sourceData += 4;
				}
			}

			nsvgDeleteRasterizer(rast);
			delete[] imageBuffer;

			free(svgImage->shapes);
			free(svgImage);

			return destBitmap;
		}

		wxButton* NewSvgButton(const ThemeData::ThemeImage& themeImage, const wxSize& size, wxPanel& panel)
		{
			wxButton* button = new wxButton(&panel, wxID_ANY, wxEmptyString);
			button->SetMinSize(size);
			button->SetBitmap(*LoadRasterizedSvg(themeImage.path.c_str(), size, themeImage.offset, themeImage.scale));
			return button;
		}
	}
}
