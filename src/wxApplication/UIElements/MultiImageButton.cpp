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

#include "MultiImageButton.h"
#include "ElementsUtil.h"
#include "../Theme/ThemeData/ThemeImage.h"

namespace UIElements
{
	MultiImageButton::MultiImageButton(const MultiImageButton::ImageList& images, const wxSize& size, wxPanel& panel) :
		wxButton(&panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)
	{
		for (const ThemeData::ThemeImage& themeImage : images)
		{
			_indexedBitmaps.emplace_back(UIElements::Util::LoadColorizedSvg(themeImage.path.c_str(), size));
		}

		SetActiveImage(0);
	}

	void MultiImageButton::SetActiveImage(int index)
	{
		_activeBitmapIndex = index;
		SetBitmap(*_indexedBitmaps.at(_activeBitmapIndex).get());
	}

	int MultiImageButton::GetActiveImage() const
	{
		return _activeBitmapIndex;
	}
}
