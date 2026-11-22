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
#include "../../Config/UIStrings.h"
#include "../../../Util/HelpersGeneral.h"
#include "../../../wxApplication/Helpers/HelpersWx.h"
#include <wx/dir.h>
#include <wx/filedlg.h>

namespace
{
    const wxString DIR_USER_PRESETS("UserPresets");

    wxFileName GetRootSavePath()
    {
        return Helpers::Wx::Files::GetConfigFilePath(wxEmptyString);
    }
}

FrameEmuPresets::FrameEmuPresets(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app)
    : wxDialog(parent, wxID_ANY, title, pos, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP),
    _app(app)
{
    SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    _panel = new wxPanel(this, wxID_ANY);
    _ui = std::make_unique<FrameElements::ElementsEmuPresets>(*_panel);

    // Configure UI
    UpdateUiState(); // TODO: get rid of this if unused

    // *** Bindings ***
    _ui->btnPresetAdd->Bind(wxEVT_BUTTON, [this](wxCommandEvent& /*evt*/) { SaveAs(); });
    _ui->btnPresetRemove->Bind(wxEVT_BUTTON, [this](wxCommandEvent& /*evt*/) { RemovePreset(); });

    _ui->presetsList->Bind(wxEVT_LISTBOX_DCLICK, [this](wxCommandEvent& evt) { LoadOrResetPreset(evt.GetString()); });
    _ui->presetsList->Bind(wxEVT_KEY_DOWN, [this](wxKeyEvent& evt)
    {
        const int key = evt.GetKeyCode();
        if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER)
        {
            LoadOrResetPreset(_ui->presetsList->GetStringSelection());
            return;
        }
        evt.Skip();
    });
    _ui->presetsList->Bind(wxEVT_LISTBOX, [this](wxCommandEvent& evt) { _ui->btnPresetRemove->Enable(!evt.GetString().IsEmpty()); });

    Bind(wxEVT_CHAR_HOOK, &OnCharHook, this);
}

bool FrameEmuPresets::Show(bool show)
{
    const bool retval = wxDialog::Show(show);
    UpdateUiState();
    RefreshPresetsList();
    return retval;
}

void FrameEmuPresets::UpdateUiState() // TODO: remove me?
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

void FrameEmuPresets::RefreshPresetsList()
{
    _ui->presetsList->Clear();

    wxFileName filepath(GetRootSavePath());
    filepath.AppendDir(DIR_USER_PRESETS);

    wxArrayString filenames;
    wxDir dir(filepath.GetFullPath());

    if (dir.IsOpened())
    {
        {
            wxString filename;

            bool cont = dir.GetFirst(&filename, wxString::Format("*%s", Helpers::Wx::Files::FILE_EXTENSION_XML), wxDIR_FILES);
            while (cont)
            {
                filenames.Add(wxFileName(filename).GetName());
                cont = dir.GetNext(&filename);
            }
        }

        if (!filenames.IsEmpty())
        {
            _ui->presetsList->InsertItems(filenames, 0);
        }
    }
}

void FrameEmuPresets::SaveAs()
{
    wxTextEntryDialog dlg
    (
        this,
        "Preset name", // TODO: UIStrings
        wxGetStockLabel(wxID_SAVEAS, wxSTOCK_NOFLAGS),
        "Custom" // TODO: UIStrings
    );

    if (dlg.ShowModal() != wxID_OK)
    {
        return;
    }

    wxString filename = Helpers::Wx::Files::SanitizedFileName(dlg.GetValue());
    if (filename.IsEmpty()) // Completely invalid filename.
    {
        CallAfter([this](){ SaveAs(); });
        return;
    }

    if (!filename.EndsWith(Helpers::Wx::Files::FILE_EXTENSION_XML))
    {
        filename += Helpers::Wx::Files::FILE_EXTENSION_XML;
    }

    wxFileName filepath(GetRootSavePath());
    if (!filepath.DirExists(DIR_USER_PRESETS))
    {
        filepath.Mkdir(DIR_USER_PRESETS);
    }
    filepath.AppendDir(DIR_USER_PRESETS);
    filepath.SetFullName(filename);

    if (filepath.Exists())
    {
        const wxString prompt = wxString::Format("Overwrite: %s?", filename.Mid(0, filename.Length() - Helpers::Wx::Files::FILE_EXTENSION_XML.length())); // TODO: UIStrings
        const int answer = wxMessageBox(prompt, wxGetStockLabel(wxID_SAVEAS, wxSTOCK_NOFLAGS), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this);
        if (answer != wxYES)
        {
            CallAfter([this](){ SaveAs(); });
            return;
        }
    }

    // Save to disk
    Settings::CustomPresetSettings settings(filepath.GetFullPath());
    for (const FrameElements::FilterWidgetBase* const widget : _ui->GetAllWidgets())
    {
        if (widget->IsEnabled())
        {
            settings.AddOption(Settings::Option(widget->GetUid(), widget->GetValue()));
        }
    }

    settings.TrySave();
    RefreshPresetsList();
}

void FrameEmuPresets::LoadOrResetPreset(const wxString& name)
{
    const bool diskPreset = !name.IsEmpty();
    _ui->btnPresetRemove->Enable(diskPreset);

    wxFileName filepath;
    if (diskPreset)
    {
        filepath = GetRootSavePath();
        filepath.AppendDir(DIR_USER_PRESETS);
        filepath.SetFullName(wxString::Format("%s%s", name, Helpers::Wx::Files::FILE_EXTENSION_XML));

        if (!filepath.Exists())
        {
            wxMessageBox(Strings::Common::GENERIC_NOT_FOUND, Strings::EmuPresets::WINDOW_TITLE, wxICON_ERROR);
            return;
        }
    }

    Settings::CustomPresetSettings settings(filepath.GetFullPath());
    if (diskPreset)
    {
        SetTitle(wxString::Format("%s - %s", Strings::EmuPresets::WINDOW_TITLE, name));
        settings.TryLoad(settings.GetDefaultSettings());
    }
    else
    {
        SetTitle(Strings::EmuPresets::WINDOW_TITLE);
    }

    for (FrameElements::FilterWidgetBase* const widget : _ui->GetAllWidgets())
    {
        Settings::Option* option = settings.GetOption(widget->GetUid());
        widget->Enable(option != nullptr && !option->IsUnchangedDefault());
        if (option != nullptr)
        {
            widget->SetValue(option->GetValue());
        }
    }
}

void FrameEmuPresets::RemovePreset()
{
    wxString name = _ui->presetsList->GetStringSelection();
    if (name.IsEmpty())
    {
        return;
    }

    const wxString prompt = wxString::Format("Delete preset: %s?", name); // TODO: UIStrings
    const int answer = wxMessageBox(prompt, wxGetStockLabel(wxID_DELETE, wxSTOCK_NOFLAGS), wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this);
    if (answer != wxYES)
    {
        return;
    }

    wxFileName filepath(GetRootSavePath());
    filepath.AppendDir(DIR_USER_PRESETS);
    filepath.SetFullName(wxString::Format("%s%s", name, Helpers::Wx::Files::FILE_EXTENSION_XML));

    if (!filepath.Exists())
    {
        wxMessageBox(Strings::Common::GENERIC_NOT_FOUND, Strings::EmuPresets::WINDOW_TITLE, wxICON_ERROR);
        return;
    }

    if (wxRemoveFile(filepath.GetFullPath()))
    {
        _ui->btnPresetRemove->Enable(false);
        RefreshPresetsList();
    }
}
