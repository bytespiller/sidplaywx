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

#include "FrameEmuPresets.h"
#include "../../MyApp.h"
#include "../../../Util/HelpersGeneral.h"
#include <wx/filedlg.h>

FrameEmuPresets::FrameEmuPresets(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app)
    : wxDialog(parent, wxID_ANY, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
    _app(app)
{
    SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    _panel = new wxPanel(this, wxID_ANY);
    _ui = std::make_unique<FrameElements::ElementsEmuPresets>(*_panel);

    // Configure UI
    UpdateUiState();

    // *** Bindings ***
    //_ui->sliderPlaybackSpeed->Bind(wxEVT_SCROLL_CHANGED, &OnSpeedSlider, this);
    //_ui->sliderPlaybackSpeed->Bind(wxEVT_SCROLL_THUMBRELEASE , &OnSpeedSlider, this);

    Bind(wxEVT_CHAR_HOOK, &OnCharHook, this);
}

bool FrameEmuPresets::Show(bool show)
{
    const bool retval = wxDialog::Show(show);
    UpdateUiState();
    return retval;
}

void FrameEmuPresets::UpdateUiState()
{
    // Reminder: don't call this from thread context (e.g., EmitSignal initiated from the seek callback).
    if (!wxIsMainThread())
    {
        return;
    }

    //const PlaybackController& playback = _app.GetPlaybackInfo();
}

void FrameEmuPresets::OnCharHook(wxKeyEvent& evt)
{
    if (evt.GetKeyCode() == WXK_ESCAPE)
    {
        Close();
        return;
    }

    evt.Skip();
}
