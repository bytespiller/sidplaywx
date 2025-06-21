/*
* This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
* Copyright (C) 2024-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "ScrollingLabel.h"

#include "../Theme/ThemeData/ThemedElementData.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

constexpr int INTERVAL_SCROLLING = 10; // ms
constexpr float FEATHERING = 20.0f; // px
constexpr int SCROLL_RESUME_DELAY = 1000; // ms -- delay of auto-resume after user dragged the scroll text manually.
constexpr float SCROLL_START_OFFSET = 0.75f; // Offset factor (higher value rewinds towards the window width).

namespace UIElements
{
	ScrollingLabel::ScrollingLabel(wxPanel* parent, const ThemeData::ThemedElementData& themedData, TextJustify justify) :
		wxWindow(parent, wxID_ANY),
		_themedData(themedData),
		_timer(this),
		_justify(justify),
		_bgColor(GetBackgroundColour()),
		_bgTransparentColor(_bgColor.Red(), _bgColor.Green(), _bgColor.Blue(), 0)
	{
		SetBackgroundStyle(wxBG_STYLE_PAINT);
		SetForegroundColour(_themedData.GetPropertyColor("textColor"));

		// Set fixed height similar to the regular StaticText control
		{
			int _, height;
			GetTrueTextExtent("A", _, height);
			SetMaxClientSize(wxSize(-1, std::max(1, height)));
		}

		// Set timer interval
		_timer.Start(INTERVAL_SCROLLING);
		_timer.Stop();

		// Bind event handlers
		Bind(wxEVT_PAINT, &OnPaintEvent, this);
		Bind(wxEVT_TIMER, &OnTimer, this, _timer.GetId());

		Bind(wxEVT_ENTER_WINDOW, &OnMouseEnter, this);
		Bind(wxEVT_LEAVE_WINDOW, &OnMouseLeave, this);
		Bind(wxEVT_LEFT_DOWN, &OnMouseDown, this);
		Bind(wxEVT_MOUSE_CAPTURE_LOST, &OnMouseCaptureLost, this);
	}

	ScrollingLabel::~ScrollingLabel()
	{
		_timer.Stop();
	}

	void ScrollingLabel::SetText(const wxString& text)
	{
		wxString normalizedText = text;
		normalizedText.Trim();

		if (_text.IsSameAs(normalizedText))
		{
			return;
		}

		_text = normalizedText;

		{
			int _;
			GetTrueTextExtent(_text, _textWidth, _);
		}

		if (IsScrollingNeeded())
		{
			_scrollStartDelay = SCROLL_RESUME_DELAY; // Don't delay initially.
			_posX = -(GetClientSize().GetWidth() * SCROLL_START_OFFSET);
		}

		Refresh(false);
	}

	const wxString& ScrollingLabel::GetText() const
	{
		return _text;
	}

	void ScrollingLabel::GetTrueTextExtent(const wxString& text, int& width, int& height)
	{
		wxGraphicsContext* const gc = wxGraphicsContext::Create(wxClientDC(this));
		gc->SetFont(GetFont(), GetForegroundColour());

		double w, h;
		gc->GetTextExtent(text, &w, &h); // Calling regular GetTextExtent comes short (on MSW) so we have to use the one from the wxGraphicsContext.

		width = static_cast<int>(std::ceil(w) + 1);
		height = static_cast<int>(std::ceil(h));

		delete gc;
	}

	void ScrollingLabel::Render(wxGraphicsContext& gc)
	{
		// Init scrolling state (because this callback happens on control resize too)
		const bool shouldScroll = IsScrollingNeeded();
		if (shouldScroll) // Auto-scrolling mode
		{
			if (!_timer.IsRunning())
			{
				_lastTimestamp = 0; // Ensure scrolling restarts from the beginning.
				_scrollStartDelay = SCROLL_RESUME_DELAY; // Don't delay initially.
				_timer.Start();
				SetCursor(wxCURSOR_SIZEWE);
			}
		}
		else // Static text mode
		{
			if (_timer.IsRunning())
			{
				_timer.Stop();
				_posX = 0; // Left-justify
				SetCursor(wxCURSOR_DEFAULT);
			}

			// Dynamically re-justify the now-static text
			if (_justify == TextJustify::Right)
			{
				_posX = _textWidth - GetClientSize().GetWidth();
			}
		}

		// Draw the text
		gc.SetFont(GetFont(), GetForegroundColour());
		gc.DrawText(_text, -_posX, 0);

		if (!shouldScroll)
		{
			return;
		}

		// Create a gradient brush (for feathered overlay)
		// Draw a rectangle with the gradient brush
		const wxSize& size = GetClientSize();
		const float startPos = std::min(0.5f, FEATHERING / std::max(1, size.GetWidth()));
		const float endPos = 1.0f - startPos;

		wxGraphicsGradientStops stops(_bgColor, _bgTransparentColor);
		stops.Add(_bgColor, 0.0f);
		stops.Add(_bgTransparentColor, startPos);
		stops.Add(_bgTransparentColor, endPos);
		stops.Add(_bgColor, 1.0f);

		const wxGraphicsBrush& brush = gc.CreateLinearGradientBrush(0, 0, size.GetWidth(), 0, stops);
		gc.SetBrush(brush);
		gc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
	}

	// Called by the system of wxWidgets when the panel needs to be redrawn. You can also trigger this call by calling Refresh()/Update().
	void ScrollingLabel::OnPaintEvent(wxPaintEvent& /*evt*/)
	{
		wxAutoBufferedPaintDC dc(this);
#ifndef __WXGTK__
		dc.Clear();
#endif
		wxGraphicsContext* const gc = wxGraphicsContext::Create(dc);
		Render(*gc);
		delete gc;
	}

	void ScrollingLabel::OnTimer(wxTimerEvent& /*evt*/)
	{
		// Determine time-based scroll amount
		const double now = wxGetUTCTimeMillis().ToDouble();
		if (_lastTimestamp == 0)
		{
			_lastTimestamp = now;
		}

		const double elapsed = now - _lastTimestamp;
		_lastTimestamp = now;

		// Update scroll position
		if (_mouseCaptured)
		{
			_posX = _mouseCapturedDownX - wxGetMousePosition().x;
			if (!wxGetMouseState().LeftIsDown())
			{
				ReleaseMouse();
				_mouseCaptured = false;
				_scrollStartDelay = 0;
			}
		}
		else if (!_autoScrollingPaused)
		{
			if (_scrollStartDelay >= SCROLL_RESUME_DELAY)
			{
				_posX += elapsed / 30;
			}
			else
			{
				_scrollStartDelay += elapsed;
			}
		}

		if (_posX < -GetClientSize().GetWidth())
		{
			// Window width reduced, catch up so we're not trailing behind
			_posX = -GetClientSize().GetWidth();
		}
		else if (_textWidth > 0 && _posX > _textWidth)
		{
			// Wrap the scroll position
			_posX = -GetClientSize().GetWidth();
		}

		// Force repaint
		Refresh(false);
	}

	void ScrollingLabel::OnMouseEnter(wxMouseEvent& /*evt*/)
	{
		_autoScrollingPaused = true;
	}

	void ScrollingLabel::OnMouseLeave(wxMouseEvent& /*evt*/)
	{
		_autoScrollingPaused = false;
	}

	void ScrollingLabel::OnMouseDown(wxMouseEvent& /*evt*/)
	{
		if (IsScrollingNeeded())
		{
			if (!_mouseCaptured)
			{
				_mouseCaptured = true;
				CaptureMouse();
				_mouseCapturedDownX = wxGetMousePosition().x + std::ceil(_posX);
			}
		}
	}

	void ScrollingLabel::OnMouseCaptureLost(wxMouseCaptureLostEvent& /*evt*/)
	{
		_mouseCaptured = false;
	}

	bool ScrollingLabel::IsScrollingNeeded() const
	{
		return _textWidth > GetClientSize().GetWidth();
	}
}
