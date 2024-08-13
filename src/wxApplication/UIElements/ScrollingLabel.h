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

namespace ThemeData
{
	class ThemedElementData;
}

namespace UIElements
{
	class ScrollingLabel : public wxWindow
	{
	public:
		/// @brief Text justification when not scrolling.
		enum class TextJustify
		{
			Left,
			Right
		};

	public:
		ScrollingLabel() = delete;
		ScrollingLabel(wxPanel* parent, const ThemeData::ThemedElementData& themedData, TextJustify justify = TextJustify::Left);

		~ScrollingLabel() override;

	public:
		void SetText(const wxString& text);
		const wxString& GetText() const;

	private:
		void GetTrueTextExtent(const wxString& text, int& width, int& height);
		void Render(wxGraphicsContext& gc);
		bool IsScrollingNeeded() const;

	private: // Event handlers
		void OnPaintEvent(wxPaintEvent& evt);
		void OnTimer(wxTimerEvent& evt);

		void OnMouseEnter(wxMouseEvent& evt);
		void OnMouseLeave(wxMouseEvent& evt);
		void OnMouseDown(wxMouseEvent& evt);
		void OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);

	private:
		const ThemeData::ThemedElementData& _themedData;
		wxTimer _timer;
		TextJustify _justify;
		wxString _text;
		int _textWidth = 0;
		double _posX = 0.0;
		double _lastTimestamp = 0;
		int _scrollStartDelay = 0;

		bool _autoScrollingPaused = false;
		bool _mouseCaptured = false;
		int _mouseCapturedDownX = 0;
	};
}
