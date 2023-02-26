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

#include "FramePlaybackMods.h"
#include "../../MyApp.h"
#include "../../../Util/HelpersGeneral.h"
#include <wx/filedlg.h>

using VoiceCheckBoxData = FrameElements::ElementsPlaybackMods::VoiceCheckBoxData;

FramePlaybackMods::FramePlaybackMods(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app)
    : wxDialog(parent, wxID_ANY, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
    _app(app)
{
    _panel = new wxPanel(this, wxID_ANY);
    _panel->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    _panel->SetDoubleBuffered(true);
    _ui = std::make_unique<FrameElements::ElementsPlaybackMods>(*_panel);

    // Configure UI
    UpdateUiState();

    // Bindings
    for (wxCheckBox* const chkBox : _ui->GetVoiceCheckBoxes())
    {
        chkBox->Bind(wxEVT_CHECKBOX, &OnVoiceCheckBox, this);
    }

    _ui->sliderPlaybackSpeed->Bind(wxEVT_SCROLL_CHANGED, &OnSpeedSlider, this);
    _ui->sliderPlaybackSpeed->Bind(wxEVT_SCROLL_THUMBRELEASE , &OnSpeedSlider, this);

    Bind(wxEVT_CHAR_HOOK, &OnCharHook, this);

    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_PLAYBACK_SPEED_CHANGED, std::bind(&UpdateUiState, this));
    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_PLAYBACK_STATE_CHANGED, std::bind(&UpdateUiState, this));
}

bool FramePlaybackMods::Show(bool show)
{
    const bool retval = wxDialog::Show(show);
    UpdateUiState();
    return retval;
}

int FramePlaybackMods::ShowModal()
{
    const int retval = wxDialog::ShowModal();
    UpdateUiState();
    return retval;
}

void FramePlaybackMods::UpdateUiState()
{
    // Reminder: don't call this from thread context (e.g., EmitSignal initiated from the seek callback).
    if (!wxIsMainThread())
    {
        return;
    }

    const bool changeable = !(_app.GetPlaybackInfo().GetState() != PlaybackController::State::Stopped && _app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool());
    _ui->labelPreRenderActive->Show(!changeable);

    const PlaybackController& playback = _app.GetPlaybackInfo();
    const int sidChipsRequired = playback.GetCurrentTuneSidChipsRequired();

    for (wxCheckBox* chkBox : _ui->GetVoiceCheckBoxes())
    {
        const VoiceCheckBoxData* const data = dynamic_cast<VoiceCheckBoxData*>(chkBox->GetRefData());
        assert(data != nullptr);
        chkBox->SetValue(playback.IsVoiceEnabled(data->sidNum, data->voice));
        chkBox->Enable(changeable && sidChipsRequired >= data->sidNum + 1);
    }

    _ui->SetActiveSidsIndicator(sidChipsRequired);
    _ui->sliderPlaybackSpeed->SetValue(playback.GetPlaybackSpeedFactor() * 100);
}

void FramePlaybackMods::OnSpeedSlider(wxCommandEvent& evt)
{
    const double wantedSpeed = evt.GetInt() / 100.0;
    const double cSpeed = _app.GetPlaybackInfo().GetPlaybackSpeedFactor();
    if (Helpers::General::AreFloatsEqual(cSpeed, wantedSpeed))
    {
        return;
    }

    _app.SetPlaybackSpeed(wantedSpeed);

    const double actualSpeed = _app.GetPlaybackInfo().GetPlaybackSpeedFactor();
    if (!Helpers::General::AreFloatsEqual(actualSpeed, wantedSpeed))
    {
        _ui->sliderPlaybackSpeed->SetValue(actualSpeed * 100);
    }
}

void FramePlaybackMods::OnVoiceCheckBox(wxCommandEvent& evt)
{
    const VoiceCheckBoxData* const data = dynamic_cast<VoiceCheckBoxData*>(evt.GetEventObject()->GetRefData());
    assert(data != nullptr);

    const std::vector<wxCheckBox*>& allCheckboxes = _ui->GetVoiceCheckBoxes();
    const bool modifierSolo = wxGetKeyState(WXK_CONTROL);
    if (modifierSolo)
    {
        const bool modifierSoloAll = wxGetKeyState(WXK_SHIFT);
        const bool shouldUnmuteAll = std::all_of(allCheckboxes.begin(), allCheckboxes.end(), [data, modifierSoloAll](const wxCheckBox* chkBox)
        {
            const VoiceCheckBoxData* const dataOther = dynamic_cast<VoiceCheckBoxData*>(chkBox->GetRefData());
            if (!modifierSoloAll && dataOther->sidNum != data->sidNum)
            {
                return true;
            }

            return !chkBox->IsChecked() || &data->checkBoxObject.get() == chkBox;
        });

        for (wxCheckBox* const chkBox : allCheckboxes)
        {
            const VoiceCheckBoxData* const dataOther = dynamic_cast<VoiceCheckBoxData*>(chkBox->GetRefData());
            assert(dataOther != nullptr);

            const bool sameSid = dataOther->sidNum == data->sidNum;
            const bool sameVoice = dataOther->voice == data->voice;
            if (sameSid && sameVoice)
            {
                continue;
            }

            if (!modifierSoloAll && !sameSid)
            {
                continue;
            }

            _app.ToggleVoice(dataOther->sidNum, dataOther->voice, shouldUnmuteAll);
        }
    }

    _app.ToggleVoice(data->sidNum, data->voice, modifierSolo || evt.IsChecked());
    UpdateUiState();
}

void FramePlaybackMods::OnCharHook(wxKeyEvent& evt)
{
    if (evt.GetKeyCode() == WXK_ESCAPE)
    {
        Close();
    }
}
