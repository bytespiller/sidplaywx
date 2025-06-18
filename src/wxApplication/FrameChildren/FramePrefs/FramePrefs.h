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

#include "ElementsPrefs.h"
#include "../../Config/AppSettings.h"
#include "../../FramePlayer/FramePlayer.h"

#include <map>
#include <memory>

class MyApp;

class FramePrefs : public wxDialog
{
public:
    FramePrefs() = delete;
    FramePrefs(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app, FramePlayer& framePlayer);

public:
    bool Destroy() override;

private:
    using SettingId = const char*;
    using TypeSerialized = Settings::Option::TypeSerialized;

    struct WrappedProp
    {
        WrappedProp() = delete;
        WrappedProp(SettingId aSettingId, TypeSerialized aType, wxPGProperty& aProperty, bool aRequiresRestart) :
            settingId(aSettingId),
            type(aType),
            property(aProperty),
            requiresRestart(aRequiresRestart)
        {
        }

        SettingId settingId;
        TypeSerialized type;
        wxPGProperty& property;
        bool requiresRestart = false;
        bool changed = false;
    };

    using WrappedProps = std::map<SettingId, WrappedProp>;

private:
    void AddWrappedProp(SettingId settingId, TypeSerialized type, wxPGProperty* property, wxPropertyGridPage& page, bool requiresRestart, wxString helpString = "");
    void AddWrappedProp(SettingId settingId, TypeSerialized type, wxPGProperty* property, wxPropertyGridPage& page, bool requiresRestart, wxString helpString, wxVariant minValue, wxVariant maxValue);
    void FillPropertyGrid();
    FramePrefs::WrappedProp& GetWrappedProp(const wxPGProperty& property);

private:
    void OnPropertyGridChanging(wxPropertyGridEvent& evt);
    void OnPropertyGridChanged(wxPropertyGridEvent& evt);

    void OnButtonApply(wxCommandEvent& evt);
    void OnButtonOk(wxCommandEvent& evt);
    void OnButtonCancel(wxCommandEvent& evt);

private:
    MyApp& _app;
    FramePlayer& _framePlayer;
    wxPanel* _panel;
    std::unique_ptr<FrameElements::ElementsPrefs> _ui;
    WrappedProps _wrappedProps;
};
