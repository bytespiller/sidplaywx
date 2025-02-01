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

#include "ElementsTuneInfo.h"

#include <memory>

class PlaybackController;
class PlaylistTreeModelNode;
class Stil;

class FrameTuneInfo: public wxDialog
{
public:
	FrameTuneInfo() = delete;
	FrameTuneInfo(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, const PlaybackController& playbackInfo, Stil& stil);

public:
	bool ShowAndUpdate(const PlaylistTreeModelNode* const node);

	bool Show(bool show, const PlaylistTreeModelNode* const node);
	void Hide();

	bool IsVisible() const;
	bool ShouldFollowPlayback() const;

private:
	void UpdateInfo(const PlaylistTreeModelNode* const node);
	void SetPropertyValue(const char* name, const wxString& value);

private:
	const PlaybackController& _playbackInfo;
	Stil& _stil;
	wxPanel* _panel = nullptr;
	std::unique_ptr<FrameElements::ElementsTuneInfo> _ui;
};
