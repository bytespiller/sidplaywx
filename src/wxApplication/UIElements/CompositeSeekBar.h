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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#ifdef WIN32
	#include <wx/appprogress.h>
	#include <wx/taskbarbutton.h>
#else
	enum wxTaskBarButtonState // Placeholder with invalid values. Use static_assert to ensure this is never actually used.
	{
		wxTASKBAR_BUTTON_NO_PROGRESS = -1,
		wxTASKBAR_BUTTON_NORMAL = -1,
		wxTASKBAR_BUTTON_PAUSED = -1,
		wxTASKBAR_BUTTON_INDETERMINATE = -1
	};
#endif

namespace ThemeData
{
	class ThemedElementData;
}

namespace UIElements
{
	wxDECLARE_EVENT(EVT_CSB_SeekForward, wxCommandEvent);
	wxDECLARE_EVENT(EVT_CSB_SeekBackward, wxCommandEvent);

	class CompositeSeekBar : public wxWindow
#ifdef WIN32
							, public wxAppProgressIndicator::wxAppProgressIndicator
#endif
	{
	public:
		enum class TaskbarProgressOption : int
		{
			Enabled = 0,
			Color,
			Disabled
		};

	public:
		CompositeSeekBar(wxPanel* parent, const ThemeData::ThemedElementData& themedData);
		~CompositeSeekBar() override = default;

	public:
		void UpdatePlaybackPosition(long time, double preRenderProgressFactor = 0.0);
		void ResetPlaybackPosition(long duration);
		void SetEnabledAuto();
		double GetNormalizedFillTarget() const;
		bool IsSeekPreviewing() const;
		long GetDurationValue() const;

		void SetTaskbarProgressOption(TaskbarProgressOption option);
		void SetTaskbarProgressState(wxTaskBarButtonState state);

	private:
		void Render(wxDC& dc);

		bool IsSeekTargetReached() const;
		void SetTargetFactor(int clientPointX);
		bool TriggerSeekEventIfRelevant();
		void CancelSeekPreviewIfActive();
		void CancelMouseLeftDown();

		void UpdateTaskbarIndicator();

		inline int GetSeekAreaWidth() const
		{
			return GetClientSize().GetWidth() - _thumbSize.GetWidth();
		}

	private: // Event handlers
		void OnPaintEvent(wxPaintEvent& evt);

		void OnMouseLeftDown(wxMouseEvent& evt);
		void OnMouseLeftUp(wxMouseEvent& evt);
		void OnMouseMoved(wxMouseEvent& evt);
		void OnMouseCaptureLost(wxMouseCaptureLostEvent& evt);
		void OnMouseRightDown(wxMouseEvent& evt);

	private:
		static constexpr double CLEAR_SEEK_PREVIEW = -1.0; // Reminder: this is a double, never check it for == equality!

	private:
		bool _pressedDown = false;
		double _progressFillFactor = 0.0;
		double _preRenderFillFactor = 0.0;
		double _targetFillFactor = 0.0;
		double _cancelableSeekPreviewFillFactor = CLEAR_SEEK_PREVIEW;
		double _duration = 1.0;
		const ThemeData::ThemedElementData& _themedData;
		TaskbarProgressOption _taskbarProgressOption{};

	private:
		wxSize _thumbSize = wxSize(10, 20);
		int _thumbHalfWidth = _thumbSize.GetWidth() / 2;
	};
}
