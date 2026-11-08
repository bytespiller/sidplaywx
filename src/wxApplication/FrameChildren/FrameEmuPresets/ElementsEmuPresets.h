/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <memory>

#include "Widgets/DoubleSlider.h"

namespace FrameElements
{
	class ElementsEmuPresets
	{
	public:
		ElementsEmuPresets() = delete;
		ElementsEmuPresets(const ElementsEmuPresets&) = delete;
		ElementsEmuPresets& operator=(const ElementsEmuPresets&) = delete;

	    explicit ElementsEmuPresets(wxPanel& panel);

	public:
		wxListBox* presetsList = nullptr;

		// TODO: C64 model (PAL/NTSC/etc.)
		// TODO: SID model
		// TODO: Combined waveforms strength (W/A*/S)
		std::unique_ptr<DoubleSlider> sliderDacLeakage;

		std::unique_ptr<DoubleSlider> sliderFilterCurve6581; // 0.5
		std::unique_ptr<DoubleSlider> sliderFilterRange6581; // 0.5
		std::unique_ptr<DoubleSlider> sliderWaveOffset6581; // 0
		// TODO: Early model caps 6581 (true/false)

		std::unique_ptr<DoubleSlider> sliderFilterCurve8580; // 0.5

		// TODO: DigiBoost 8580 (true/false)

	private:
		wxPanel& _parentPanel;
	};
}
