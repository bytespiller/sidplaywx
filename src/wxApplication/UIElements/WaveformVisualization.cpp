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

#include "WaveformVisualization.h"
#include "../Theme/ThemeData/ThemedElementData.h"

namespace UIElements
{
	static wxPen wavePen;
	static wxColor gradientTo;

	constexpr float MAX_ABS_SHORT = std::numeric_limits<short>::max() + 1.0f; // Float because it's used for calculating a scale factor.

	WaveformVisualization::WaveformVisualization(wxPanel* parent, const ThemeData::ThemedElementData& themedData) :
		wxWindow(parent, wxID_ANY),
		_themedData(themedData)
	{
		// Reminder: parent panel MUST be double buffered.

		// Colors
		wavePen = wxPen(wxColor(_themedData.GetPropertyColor("waveColor")), 2);
		gradientTo = _themedData.GetPropertyColor("gradientTo");

		// Bindings
		Bind(wxEVT_PAINT, &WaveformVisualization::OnPaintEvent, this);
	}

	void WaveformVisualization::SetBufferWindow(size_t size)
	{
		if (size == 0)
		{
			_bufferSize = 0;
			_buffer = nullptr;
		}
		else
		{
			_bufferSize = size;
			_buffer = std::make_unique<short[]>(_bufferSize);
		}

		Clear();
	}

	short* WaveformVisualization::GetBuffer()
	{
		return _buffer.get();
	}

	void WaveformVisualization::Clear()
	{
		if (_buffer != nullptr)
		{
			std::fill_n(_buffer.get(), _bufferSize, 0);
		}

		Refresh();
	}

	void WaveformVisualization::Render(wxDC& dc)
	{
		if (_buffer == nullptr)
		{
			dc.Clear();
			return;
		}

		const float width = std::max(1, GetClientSize().GetWidth()) + 1.0f;
		const int height = std::max(1, GetClientSize().GetHeight());
		const int vertCenter = height / 2;
		const float amplitudeRange = vertCenter / MAX_ABS_SHORT;

		const size_t compressX = (_bufferSize < width) ? (width / _bufferSize) + 1.0f : 1; // Stretch horizontally if the client width exceeds the buffer size.
		const float numSamplesPerPx = std::max(1.0f, _bufferSize / width); // Constrain the waveform width within the available client width.

		// Background
		dc.GradientFillLinear(GetClientSize(), GetBackgroundColour(), gradientTo, wxDirection::wxDOWN);

		// Waveform
		wxPoint points[static_cast<size_t>(width)];

		for (int x = 0; x < width; ++x)
		{
			int sum = 0;
			for (int n = 0; n < numSamplesPerPx; ++n)
			{
				sum -= _buffer[((x / compressX) * numSamplesPerPx) + n];
			}

			points[x] = wxPoint(x, (sum / std::ceil(numSamplesPerPx)) * amplitudeRange);
		}

		dc.SetPen(wavePen);
		dc.DrawLines(width, points, 0, vertCenter);
	}

	// Called by the system of wxWidgets when the panel needs to be redrawn. You can also trigger this call by calling Refresh()/Update().
	void WaveformVisualization::OnPaintEvent(wxPaintEvent& /*evt*/)
	{
		wxPaintDC dc(this);
		Render(dc);
	}
}
