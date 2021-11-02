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

#include "CompositeSeekBar.h"
#include "../Helpers/DpiSize.h"
#include "../Theme/ThemeData/ThemedElementData.h"
#include <unordered_map>

namespace
{
	static constexpr int SEEKBAR_BORDER_SIZE = 1;
	static constexpr int SEEKBAR_BORDER_SIZE_DOUBLE = SEEKBAR_BORDER_SIZE * 2;
	static constexpr float FILL_BORDER_HEIGHT_FACTOR = 0.4f;

	namespace ThemedColors
	{
		static const std::string thumbColor("thumbColor");
		static const std::string thumbPressedColor("thumbPressedColor");
		static const std::string fillColorBackground("fillColorBackground");
		static const std::string fillColorBarDefault("fillColorBarDefault");
		static const std::string fillColorBarSeeking("fillColorBarSeeking");
		static const std::string fillColorBarPreviewDiscard("fillColorBarPreviewDiscard");
		static const std::string fillColorBarSeekingRemaining("fillColorBarSeekingRemaining");
		static const std::string thumbDisabledColor("thumbDisabledColor");

		std::unordered_map<std::string, wxColor> color =
		{
			{thumbColor, wxColor()},
			{thumbPressedColor, wxColor()},
			{fillColorBackground, wxColor()},
			{fillColorBarDefault, wxColor()},
			{fillColorBarSeeking, wxColor()},
			{fillColorBarPreviewDiscard, wxColor()},
			{fillColorBarSeekingRemaining, wxColor()},
			{thumbDisabledColor, wxColor()}
		};
	}
}

namespace UIElements
{
	static constexpr double NON_ZERO_DOUBLE = 0.001; // Can't use legit stuff like the epsilon or defined limits since all of them are way too small.
	static constexpr int TASKBAR_PROGRESS_MAX_VALUE = 100;

	wxDEFINE_EVENT(EVT_CSB_SeekForward, wxCommandEvent);
	wxDEFINE_EVENT(EVT_CSB_SeekBackward, wxCommandEvent);

	CompositeSeekBar::CompositeSeekBar(wxPanel* parent, const ThemeData::ThemedElementData& themedData) :
		wxWindow(parent, wxID_ANY),
		wxAppProgressIndicator(wxTheApp->GetTopWindow(), TASKBAR_PROGRESS_MAX_VALUE),
		_themedData(themedData)
	{
		// Reminder: parent panel MUST be double buffered.
		Disable();

		// Fill color properties with loaded wxColor (or black if not found in theme.xml)
		for (auto& nameAndColor : ThemedColors::color)
		{
			nameAndColor.second = _themedData.GetPropertyColor(nameAndColor.first);
		}

		// Bindings
		Bind(wxEVT_LEFT_DOWN, &OnMouseLeftDown, this);
		Bind(wxEVT_LEFT_UP, &OnMouseLeftUp, this);
		Bind(wxEVT_MOTION, &OnMouseMoved, this);
		Bind(wxEVT_MOUSE_CAPTURE_LOST, &OnMouseCaptureLost, this);
		Bind(wxEVT_RIGHT_DOWN, &OnMouseRightDown, this);

		Bind(wxEVT_PAINT, &CompositeSeekBar::OnPaintEvent, this);

		// Final
		CallAfter([this]()
		{
			_thumbSize = DpiSize(_thumbSize);
			_thumbHalfWidth = _thumbSize.GetWidth() / 2;
		});
	}

	void CompositeSeekBar::UpdatePlaybackPosition(long time)
	{
		_progressFillFactor = (_duration > 1.0) ? std::min(1.0, time / _duration) : 0.0;
		const bool seeking = !IsSeekTargetReached();
		if (!_pressedDown && !seeking)
		{
			_targetFillFactor = _progressFillFactor;
		}

		UpdateTaskbarIndicator();
		Refresh();
	}

	void CompositeSeekBar::ResetPlaybackPosition(long duration)
	{
		CancelSeekPreviewIfActive();
		CancelMouseLeftDown();
		_cancelableSeekPreviewFillFactor = CLEAR_SEEK_PREVIEW;

		_progressFillFactor = 0.0;
		_targetFillFactor = 0.0;
		_duration = std::max(1.0, static_cast<double>(duration));
		wxAppProgressIndicator::SetValue(0);

		SetEnabledAuto();
		Refresh();
	}

	void CompositeSeekBar::SetEnabledAuto()
	{
		Enable(_duration > 1.0);
	}

	double CompositeSeekBar::GetNormalizedFillTarget() const
	{
		return _targetFillFactor;
	}

	bool CompositeSeekBar::IsSeekPreviewing() const
	{
		return _pressedDown;
	}

	long CompositeSeekBar::GetDurationValue() const
	{
		return _duration;
	}

	void CompositeSeekBar::SetTaskbarProgressOption(TaskbarProgressOption option)
	{
		_taskbarProgressOption = option;
		UpdateTaskbarIndicator();

		if (_taskbarProgressOption == TaskbarProgressOption::Disabled)
		{
			wxAppProgressIndicator::SetValue(0);
			SetTaskbarProgressState(wxTASKBAR_BUTTON_NO_PROGRESS);
		}
	}

	void CompositeSeekBar::SetTaskbarProgressState(wxTaskBarButtonState state)
	{
		if (wxTaskBarButton* tb = dynamic_cast<wxFrame*>(wxTheApp->GetTopWindow())->MSWGetTaskBarButton())
		{
			const wxTaskBarButtonState applyState = (_taskbarProgressOption == TaskbarProgressOption::Disabled) ? wxTASKBAR_BUTTON_NO_PROGRESS : state;
			tb->SetProgressState(applyState);
		}
	}

	// Called by the system of by wxWidgets when the panel needs to be redrawn. You can also trigger this call by calling Refresh()/Update().
	void CompositeSeekBar::OnPaintEvent(wxPaintEvent& /*evt*/)
	{
		wxPaintDC dc(this);
		Render(dc);
	}

	void CompositeSeekBar::Render(wxDC& dc)
	{
		const int seekAreaWidth = GetSeekAreaWidth();
		const int seekAreaHeight = GetClientSize().GetHeight();
		const int barHeight = static_cast<int>(seekAreaHeight * FILL_BORDER_HEIGHT_FACTOR);
		const int barY = (seekAreaHeight - barHeight) / 2;
		const bool seekingForward = !IsSeekTargetReached();
		const bool seekingBackward = _pressedDown && _targetFillFactor < _progressFillFactor;
		const int borderSize = SEEKBAR_BORDER_SIZE; // Otherwise std::max can't find it.

		// Fill background
		dc.SetBrush(ThemedColors::color.at(ThemedColors::fillColorBackground));
		dc.DrawRectangle(0, barY, seekAreaWidth, barHeight);

		// Disable border
		dc.SetPen(*wxTRANSPARENT_PEN);

		// Seekbar current-progress
		wxColor* brushSeekbar = &ThemedColors::color.at(ThemedColors::fillColorBarDefault);
		if (seekingBackward)
		{
			brushSeekbar = &ThemedColors::color.at(ThemedColors::fillColorBarPreviewDiscard);
		}
		else if (seekingForward)
		{
			brushSeekbar = &ThemedColors::color.at(ThemedColors::fillColorBarSeeking);
		}
		dc.SetBrush(*brushSeekbar);

		const int seekFillWidth = static_cast<int>(seekAreaWidth * _progressFillFactor);
		dc.DrawRectangle(SEEKBAR_BORDER_SIZE, barY + SEEKBAR_BORDER_SIZE, seekFillWidth, barHeight - SEEKBAR_BORDER_SIZE_DOUBLE);

		// Seekbar remaining-progress (a.k.a. preview)
		const int tmx = std::max(0, static_cast<int>(seekAreaWidth * _targetFillFactor)); // Limit minimum.
		const int thumbTargetX = std::min(seekAreaWidth, tmx); // Limit maximum.

		if (seekingForward)
		{
			dc.SetBrush(ThemedColors::color.at(ThemedColors::fillColorBarSeekingRemaining));
			const int startX = std::max(borderSize, seekFillWidth);
			dc.DrawRectangle(startX, barY + SEEKBAR_BORDER_SIZE, thumbTargetX - startX, barHeight - SEEKBAR_BORDER_SIZE_DOUBLE);
		}

		// Re-enable the border
		dc.SetPen(wxNullPen);

		// Thumb
		if (IsEnabled())
		{
			const wxColor& thumbColor = (_pressedDown) ? ThemedColors::color.at(ThemedColors::thumbPressedColor) : ThemedColors::color.at(ThemedColors::thumbColor);
			dc.SetBrush(thumbColor);
		}
		else
		{
			dc.SetBrush(ThemedColors::color.at(ThemedColors::thumbDisabledColor));
		}

		dc.DrawRectangle(thumbTargetX, (seekAreaHeight - _thumbSize.GetHeight()) / 2, _thumbSize.GetWidth(), _thumbSize.GetHeight());
	}

	bool CompositeSeekBar::IsSeekTargetReached() const
	{
		return _progressFillFactor >= _targetFillFactor;
	}

	void CompositeSeekBar::SetTargetFactor(int clientPointX)
	{
		_targetFillFactor = std::max(NON_ZERO_DOUBLE, static_cast<double>(clientPointX) / GetSeekAreaWidth()); // Must not be possible to set to outright zero, otherwise some weird clamping to like 5 seconds happens, idk...
	}

	bool CompositeSeekBar::TriggerSeekEventIfRelevant()
	{
		wxCommandEvent* eventSeeking = nullptr;
		if (_targetFillFactor > _progressFillFactor)
		{
			eventSeeking = new wxCommandEvent(EVT_CSB_SeekForward);
		}
		else if (_targetFillFactor < _progressFillFactor)
		{
			eventSeeking = new wxCommandEvent(EVT_CSB_SeekBackward);
		}

		const bool seekPositionLocked = eventSeeking != nullptr;
		if (seekPositionLocked)
		{
			eventSeeking->SetExtraLong(_duration * _targetFillFactor);
			wxQueueEvent(this, eventSeeking);
		}

		return seekPositionLocked;
	}

	void CompositeSeekBar::CancelSeekPreviewIfActive()
	{
		if (_pressedDown)
		{
			const bool seekingActive = _cancelableSeekPreviewFillFactor > CLEAR_SEEK_PREVIEW && _progressFillFactor < _cancelableSeekPreviewFillFactor;
			_targetFillFactor = (seekingActive) ? _cancelableSeekPreviewFillFactor : _progressFillFactor;
			CancelMouseLeftDown();
		}
	}

	void CompositeSeekBar::CancelMouseLeftDown()
	{
		if (HasCapture())
		{
			ReleaseMouse();
		}
		_pressedDown = false;
		Refresh();
	}

	void CompositeSeekBar::UpdateTaskbarIndicator()
	{
		if (_taskbarProgressOption != TaskbarProgressOption::Disabled)
		{
			if(IsSeekTargetReached())
			{
				if (_taskbarProgressOption == TaskbarProgressOption::Enabled)
				{
					wxAppProgressIndicator::SetValue(std::max(1.0, _progressFillFactor * TASKBAR_PROGRESS_MAX_VALUE));
				}
				else
				{
					wxAppProgressIndicator::SetValue(TASKBAR_PROGRESS_MAX_VALUE);
				}
			}
		}
	}

	void CompositeSeekBar::OnMouseLeftDown(wxMouseEvent& evt)
	{
		if (!HasCapture())
		{
			CaptureMouse();
			_pressedDown = true;
			OnMouseMoved(evt);
		}
	}

	void CompositeSeekBar::OnMouseLeftUp(wxMouseEvent& /*evt*/)
	{
		if (_pressedDown)
		{
			CancelMouseLeftDown();
			const bool seekingTriggered = TriggerSeekEventIfRelevant();
			_cancelableSeekPreviewFillFactor = (seekingTriggered) ? _targetFillFactor : CLEAR_SEEK_PREVIEW;  // Allows us (while this seeking is underway) to cancel any new seek-previews (re-parking the seek-bar back to this target factor).
		}
	}

	void CompositeSeekBar::OnMouseMoved(wxMouseEvent& evt)
	{
		if (_pressedDown)
		{
			const int safeX = std::min(GetSeekAreaWidth(), std::max(0, evt.GetPosition().x - _thumbHalfWidth));
			SetTargetFactor(safeX);
			Refresh();
		}
	}

	void CompositeSeekBar::OnMouseCaptureLost(wxMouseCaptureLostEvent& /*evt*/)
	{
		CancelSeekPreviewIfActive(); // Also cancels the mouse left down.
	}

	void CompositeSeekBar::OnMouseRightDown(wxMouseEvent& /*evt*/)
	{
		CancelSeekPreviewIfActive();
	}
}
