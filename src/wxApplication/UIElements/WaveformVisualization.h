/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2024 Jasmin Rutic (bytespiller@gmail.com)
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

namespace ThemeData
{
	class ThemedElementData;
}

namespace UIElements
{
	class WaveformVisualization : public wxWindow
	{
	public:
		WaveformVisualization(wxPanel* parent, const ThemeData::ThemedElementData& themedData);
		~WaveformVisualization() override = default;

	public:
		/// @brief Set to 0 to disable. Sets the buffer size in bytes (i.e., span of the displayed waveform).
		void SetBufferWindow(size_t size);

		/// @brief Gets the pointer to a previously initialized buffer. Can be nullptr if disabled or not initialized.
		short* GetBuffer();

		/// @brief Clears the visualization buffer.
		void Clear();

	private:
		void Render(wxDC& dc);

	private: // Event handlers
		void OnPaintEvent(wxPaintEvent& evt);

	private:
		const ThemeData::ThemedElementData& _themedData;
		size_t _bufferSize = 0;
		std::unique_ptr<short[]> _buffer = nullptr;
	};
}
