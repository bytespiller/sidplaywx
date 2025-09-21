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

#include "FramePrefs.h"
#include "../../MyApp.h"
#include "../../Config/AppSettings.h"
#include "../../Config/UIStrings.h"
#include "../../Helpers/HelpersWx.h"
#include "../../../PlaybackController/Util/RomUtil.h"

namespace
{
    constexpr int MIN_DURATION = 0;
    constexpr int MAX_DURATION = 3600;

    constexpr int MIN_POP_SILENCER = 0;
    constexpr int MAX_POP_SILENCER = 1000;

    constexpr double MIN_FILTER_CURVE = 0.0;
    constexpr double MAX_FILTER_CURVE = 1.0;

    constexpr int MIN_SONGLENGTHS_TRIM = -1000;
    constexpr int MAX_SONGLENGTHS_TRIM = 1000;

    struct Effective
    {
        static constexpr bool AfterRestart = true;
        static constexpr bool Immediately = false;
    };

    constexpr int FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MIN = 4;
    constexpr int FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MAX = 16;

    constexpr double FXVIRTUALSTEREO_SIDE_VOLUME_FACTOR_MIN = 0.1;
    constexpr double FXVIRTUALSTEREO_SIDE_VOLUME_FACTOR_MAX = 0.4;
}

static bool __destroyed = false;

FramePrefs::FramePrefs(wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app, FramePlayer& framePlayer)
    : wxDialog(parent, wxID_ANY, title, pos, size, wxCAPTION | wxRESIZE_BORDER | wxSTAY_ON_TOP),
    _app(app),
    _framePlayer(framePlayer)
{
    __destroyed = false;

    //SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    SetDoubleBuffered(true);

    _ui = std::make_unique<FrameElements::ElementsPrefs>(*this);

    // Configure UI
    CenterOnParent();
    FillPropertyGrid();
    Layout();
    _ui->propertyGrid->SetSplitterLeft();
    _ui->propertyGrid->SetDescBoxHeight(_ui->propertyGrid->GetSize().GetHeight() / 3);

    FindWindowById(wxID_APPLY, this)->Enable(false);

    // Events
    _ui->propertyGrid->Bind(wxEVT_PG_CHANGING, &OnPropertyGridChanging, this);
    _ui->propertyGrid->Bind(wxEVT_PG_CHANGED, &OnPropertyGridChanged, this);

    FindWindowById(wxID_APPLY, this)->Bind(wxEVT_BUTTON, &OnButtonApply, this);
    FindWindowById(wxID_OK, this)->Bind(wxEVT_BUTTON, &OnButtonOk, this);
}

bool FramePrefs::Destroy()
{
    __destroyed = true;
    return wxDialog::Destroy();
}

void FramePrefs::AddWrappedProp(SettingId settingId, TypeSerialized type, wxPGProperty* property, wxPropertyGridPage& page, bool requiresRestart, wxString helpString)
{
    if (requiresRestart)
    {
        helpString = wxString::Format("%s\n%s", helpString, Strings::Preferences::SUFFIX_NOTE_APP_RESTART);
    }
    property->SetHelpString(helpString);

    page.Append(property);

    const Settings::Option* const cOption = _app.currentSettings->GetOption(settingId);

    switch (type) // Display actual values
    {
        case TypeSerialized::Double:
            property->SetValue(cOption->GetValueAsDouble());
            break;

        case TypeSerialized::Int:
            property->SetValue(cOption->GetValueAsInt());
            break;

        case TypeSerialized::String:
            property->SetValue(cOption->GetValueAsString());
            break;

        default:
            std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
            break;
    }

    _wrappedProps.emplace(settingId, WrappedProp(settingId, type, *property, requiresRestart));
}

void FramePrefs::AddWrappedProp(SettingId settingId, TypeSerialized type, wxPGProperty* property, wxPropertyGridPage& page, bool requiresRestart, wxString helpString, wxVariant minValue, wxVariant maxValue)
{
    property->SetAttribute(wxPG_ATTR_MIN, minValue);
    property->SetAttribute(wxPG_ATTR_MAX, maxValue);
    AddWrappedProp(settingId, type, property, page, requiresRestart, helpString);
}

void FramePrefs::FillPropertyGrid()
{
    wxPropertyGridPage* page;
    page = _ui->propertyGrid->AddPage();

    // Audio Output
    page->Append(new wxPropertyCategory(Strings::Preferences::CATEGORY_AUDIO_OUTPUT));
    {
        {
            const char* settingId = Settings::AppSettings::ID::AudioOutputDevice;
            const wxArrayString& supportedOutDevices = Helpers::Wx::Audio::GetAudioDevicesNames(Helpers::Wx::Audio::DeviceType::Output, Helpers::Wx::Audio::Backend::Filtered);
            wxPGProperty* propOut = new wxEnumProperty(Strings::Preferences::OPT_DEVICE, settingId, supportedOutDevices);
            AddWrappedProp(settingId, TypeSerialized::String, propOut, *page, Effective::Immediately, Strings::Preferences::DESC_DEVICE);

            const int filteredDeviceIndex = Helpers::Wx::Audio::TryGetFilteredFromAbsoluteAudioDeviceIndex(_app.GetPlaybackInfo().GetAudioConfig().device);
            if (filteredDeviceIndex != wxNOT_FOUND)
            {
                propOut->SetChoiceSelection(filteredDeviceIndex); // Select actual device.
            }
        }

        AddWrappedProp(Settings::AppSettings::ID::LowLatency, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_LOW_LATENCY), *page, Effective::Immediately, Strings::Preferences::DESC_LOW_LATENCY);

        // Out channels
        {
            const wxArrayString outChannelsOptions =
            {
                Strings::Preferences::ITEM_OUT_CHANNELS_MONO,
                Strings::Preferences::ITEM_OUT_CHANNELS_DEFAULT,
                Strings::Preferences::ITEM_OUT_CHANNELS_EXPANDED
            };

            const char* SettingId = Settings::AppSettings::ID::OutChannels;
            wxPGProperty* prop = new wxEnumProperty(Strings::Preferences::OPT_OUT_CHANNELS, SettingId, outChannelsOptions);
            AddWrappedProp(SettingId, TypeSerialized::Int, prop, *page, Effective::Immediately, Strings::Preferences::DESC_OUT_CHANNELS);

            const int selection = _app.currentSettings->GetOption(SettingId)->GetValueAsInt();
            prop->SetChoiceSelection(selection);
        }

        // FX VirtualStereo speakers distance
        {
            wxArrayString options;
            options.reserve(FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MAX - FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MIN);
            for (int ms = FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MIN; ms <= FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MAX; ++ms)
            {
                options.emplace_back(wxString::Format("Size %i", ms));
            }

            const char* SettingId = Settings::AppSettings::ID::VirtualStereoSpeakerDistance;
            wxPGProperty* prop = new wxEnumProperty(Strings::Preferences::OPT_OUT_FXVIRTUALSTEREO_SPEAKER_DISTANCE, SettingId, options);
            AddWrappedProp(SettingId, TypeSerialized::Int, prop, *page, Effective::Immediately, Strings::Preferences::DESC_OUT_FXVIRTUALSTEREO_SPEAKER_DISTANCE);
            const int selection = _app.currentSettings->GetOption(SettingId)->GetValueAsInt() - FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MIN;
            prop->SetChoiceSelection(selection);
        }

        // FX VirtualStereo side volume factor
        AddWrappedProp(Settings::AppSettings::ID::VirtualStereoSideVolumeFactor, TypeSerialized::Double, new wxFloatProperty(Strings::Preferences::OPT_OUT_FXVIRTUALSTEREO_SIDE_VOLUME), *page, Effective::Immediately, Strings::Preferences::DESC_OUT_FXVIRTUALSTEREO_SIDE_VOLUME, FXVIRTUALSTEREO_SIDE_VOLUME_FACTOR_MIN, FXVIRTUALSTEREO_SIDE_VOLUME_FACTOR_MAX);

        // Enable/disable
        RefreshVirtualStereoSettings();
    }

    // Playback
    page->Append(new wxPropertyCategory(Strings::Preferences::CATEGORY_PLAYBACK_BEHAVIOR));
    {
        AddWrappedProp(Settings::AppSettings::ID::PreRenderEnabled, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_PRERENDER), *page, Effective::Immediately, Strings::Preferences::DESC_PRERENDER);
        AddWrappedProp(Settings::AppSettings::ID::AutoPlay, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_AUTOPLAY), *page, Effective::Immediately, Strings::Preferences::DESC_AUTOPLAY);

        AddWrappedProp(Settings::AppSettings::ID::RepeatModeDefaultSubsong, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_START_DEFAULT_SUBSONG), *page, Effective::Immediately, Strings::Preferences::DESC_START_DEFAULT_SUBSONG);
        AddWrappedProp(Settings::AppSettings::ID::RepeatModeIncludeSubsongs, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_INCLUDE_SUBSONGS), *page, Effective::Immediately, Strings::Preferences::DESC_INCLUDE_SUBSONGS);

        AddWrappedProp(Settings::AppSettings::ID::SongFallbackDuration, TypeSerialized::Int, new wxIntProperty(Strings::Preferences::OPT_FALLBACK_DURATION), *page, Effective::Immediately, Strings::Preferences::DESC_FALLBACK_DURATION, 1, MAX_DURATION);
        AddWrappedProp(Settings::AppSettings::ID::SkipShorter, TypeSerialized::Int, new wxIntProperty(Strings::Preferences::OPT_SKIP_SHORTER), *page, Effective::Immediately, Strings::Preferences::DESC_SKIP_SHORTER, MIN_DURATION, MAX_DURATION);
        AddWrappedProp(Settings::AppSettings::ID::PopSilencer, TypeSerialized::Int, new wxIntProperty(Strings::Preferences::OPT_POP_SILENCER), *page, Effective::Immediately, Strings::Preferences::DESC_POP_SILENCER, MIN_POP_SILENCER, MAX_POP_SILENCER);

        {
            const wxArrayString dragDropModeOptions =
            {
                Strings::Preferences::ITEM_DRAGDROP_MODE_DUAL,
                Strings::Preferences::ITEM_DRAGDROP_MODE_REPLACE,
                Strings::Preferences::ITEM_DRAGDROP_MODE_ENQUEUE,
                Strings::Preferences::ITEM_DRAGDROP_MODE_DISABLED
            };

            const char* SettingId = Settings::AppSettings::ID::DragDropMode;
            wxPGProperty* prop = new wxEnumProperty(Strings::Preferences::OPT_DRAGDROP_MODE, SettingId, dragDropModeOptions);
            AddWrappedProp(SettingId, TypeSerialized::Int, prop, *page, Effective::Immediately, Strings::Preferences::DESC_DRAGDROP_MODE);
            const int selection = _app.currentSettings->GetOption(SettingId)->GetValueAsInt();
            prop->SetChoiceSelection(selection);
        }
    }

    // Visual
    page->Append(new wxPropertyCategory(Strings::Preferences::CATEGORY_VISUAL_BEHAVIOR));
    {
#ifdef WIN32
        {
            const wxArrayString systemThemeOptions =
            {
                Strings::Preferences::ITEM_SYSTEM_THEME_AUTO,
                Strings::Preferences::ITEM_SYSTEM_THEME_LIGHT,
                Strings::Preferences::ITEM_SYSTEM_THEME_DARK
            };

            const char* SettingId = Settings::AppSettings::ID::SystemTheme;
            wxPGProperty* prop = new wxEnumProperty(Strings::Preferences::OPT_SYSTEM_THEME, SettingId, systemThemeOptions);
            AddWrappedProp(SettingId, TypeSerialized::Int, prop, *page, Effective::AfterRestart, Strings::Preferences::DESC_SYSTEM_THEME);
            const int selection = _app.currentSettings->GetOption(SettingId)->GetValueAsInt();
            prop->SetChoiceSelection(selection);
        }
#endif

        AddWrappedProp(Settings::AppSettings::ID::SelectionFollowsPlayback, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_SELECTION_FOLLOWS_PLAYBACK), *page, Effective::Immediately, Strings::Preferences::DESC_SELECTION_FOLLOWS_PLAYBACK);
        AddWrappedProp(Settings::AppSettings::ID::AutoExpandSubsongs, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_AUTOEXPAND_SUBSONGS), *page, Effective::Immediately, Strings::Preferences::DESC_AUTOEXPAND_SUBSONGS);

#ifdef WIN32
        {
            wxArrayString taskbarProgressOptions;
            taskbarProgressOptions.push_back(Strings::Preferences::ITEM_TASKBAR_PROGRESS_ENABLED);
            taskbarProgressOptions.push_back(Strings::Preferences::ITEM_TASKBAR_PROGRESS_COLOR);
            taskbarProgressOptions.push_back(Strings::Preferences::ITEM_TASKBAR_PROGRESS_DISABLED);

            const char* SettingId = Settings::AppSettings::ID::TaskbarProgress;
            wxPGProperty* prop = new wxEnumProperty(Strings::Preferences::OPT_TASKBAR_PROGRESS, SettingId, taskbarProgressOptions);
            AddWrappedProp(SettingId, TypeSerialized::Int, prop, *page, Effective::Immediately, Strings::Preferences::DESC_TASKBAR_PROGRESS);
            const int selection = _app.currentSettings->GetOption(SettingId)->GetValueAsInt();
            prop->SetChoiceSelection(selection);
        }
#endif
    }

    // HVSC
    {
        wxPropertyCategory* categoryHvsc = new wxPropertyCategory(Strings::Preferences::CATEGORY_HVSC);
        categoryHvsc->SetHelpString(Strings::Preferences::DESC_CATEGORY_HVSC);
        page->Append(categoryHvsc);

        {
            wxFileProperty* filePropertyHandler = new wxFileProperty(Strings::Preferences::OPT_SONGLENGTHS_PATH);
            filePropertyHandler->SetAttribute(wxPG_FILE_WILDCARD, wxString::Format("%s (*.md5)|*.md5", Strings::Preferences::WILDCARD_DESC_MD5));
            AddWrappedProp(Settings::AppSettings::ID::SonglengthsPath, TypeSerialized::String, filePropertyHandler, *page, Effective::AfterRestart, Strings::Preferences::DESC_SONGLENGTHS_PATH);
        }

        AddWrappedProp(Settings::AppSettings::ID::SonglengthsTrim, TypeSerialized::Int, new wxIntProperty(Strings::Preferences::OPT_SONGLENGTHS_TRIM), *page, Effective::Immediately, Strings::Preferences::DESC_SONGLENGTHS_TRIM, MIN_SONGLENGTHS_TRIM, MAX_SONGLENGTHS_TRIM);

        {
            wxFileProperty* filePropertyHandler = new wxFileProperty(Strings::Preferences::OPT_STIL_PATH);
            filePropertyHandler->SetAttribute(wxPG_FILE_WILDCARD, wxString::Format("%s|STIL.txt", Strings::Preferences::WILDCARD_DESC_STIL_TXT));
            AddWrappedProp(Settings::AppSettings::ID::StilPath, TypeSerialized::String, filePropertyHandler, *page, Effective::Immediately, Strings::Preferences::DESC_STIL_PATH);
        }
    }

    // Emulation
    {
        wxPropertyCategory* categoryEmulation = new wxPropertyCategory(Strings::Preferences::CATEGORY_EMULATION);
        categoryEmulation->SetHelpString(Strings::Preferences::DESC_CATEGORY_EMULATION);
        page->Append(categoryEmulation);
        {
            // Default C64 model
            {
                wxArrayString defaultC64ModelOptions;
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_PREFER_PAL);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_PREFER_NTSC);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_PREFER_OLD_NTSC);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_PREFER_DREAN);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_PREFER_PAL_M);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_PAL);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_NTSC);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_OLD_NTSC);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_DREAN);
                defaultC64ModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_C64_MODEL_FORCE_PAL_M);

                const char* SettingId = Settings::AppSettings::ID::DefaultC64Model;
                wxPGProperty* prop = new wxEnumProperty(Strings::Preferences::OPT_DEFAULT_C64_MODEL, SettingId, defaultC64ModelOptions);
                AddWrappedProp(SettingId, TypeSerialized::Int, prop, *page, Effective::Immediately, Strings::Preferences::DESC_DEFAULT_C64_MODEL);
                const int selection = _app.currentSettings->GetOption(SettingId)->GetValueAsInt();
                prop->SetChoiceSelection(selection);
            }

            // Default SID model
            {
                wxArrayString defaultSidModelOptions;
                defaultSidModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_SID_MODEL_PREFER_6581);
                defaultSidModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_SID_MODEL_PREFER_8580);
                defaultSidModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_SID_MODEL_FORCE_6581);
                defaultSidModelOptions.push_back(Strings::Preferences::ITEM_DEFAULT_SID_MODEL_FORCE_8580);

                const char* SettingId = Settings::AppSettings::ID::DefaultSidModel;
                wxPGProperty* prop = new wxEnumProperty(Strings::Preferences::OPT_DEFAULT_SID_MODEL, SettingId, defaultSidModelOptions);
                AddWrappedProp(SettingId, TypeSerialized::Int, prop, *page, Effective::Immediately, Strings::Preferences::DESC_DEFAULT_SID_MODEL);
                const int selection = _app.currentSettings->GetOption(SettingId)->GetValueAsInt();
                prop->SetChoiceSelection(selection);
            }

            // Filter
            AddWrappedProp(Settings::AppSettings::ID::FilterCurve6581, TypeSerialized::Double, new wxFloatProperty(Strings::Preferences::OPT_FILTER_CURVE_6581), *page, Effective::Immediately, Strings::Preferences::DESC_FILTER_CURVE_COMMON, MIN_FILTER_CURVE, MAX_FILTER_CURVE);
            AddWrappedProp(Settings::AppSettings::ID::FilterCurve8580, TypeSerialized::Double, new wxFloatProperty(Strings::Preferences::OPT_FILTER_CURVE_8580), *page, Effective::Immediately, Strings::Preferences::DESC_FILTER_CURVE_COMMON, MIN_FILTER_CURVE, MAX_FILTER_CURVE);

            // DigiBoost
            AddWrappedProp(Settings::AppSettings::ID::DigiBoost, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_DIGIBOOST), *page, Effective::Immediately, Strings::Preferences::DESC_DIGIBOOST);

            // KERNAL
            {
                wxFileProperty* filePropertyHandler = new wxFileProperty(Strings::Preferences::OPT_ROM_KERNAL_PATH);
                AddWrappedProp(Settings::AppSettings::ID::RomKernalPath, TypeSerialized::String, filePropertyHandler, *page, Effective::AfterRestart, Strings::Preferences::DESC_ROM_KERNAL_PATH);
            }

            // BASIC
            {
                wxFileProperty* filePropertyHandler = new wxFileProperty(Strings::Preferences::OPT_ROM_BASIC_PATH);
                AddWrappedProp(Settings::AppSettings::ID::RomBasicPath, TypeSerialized::String, filePropertyHandler, *page, Effective::AfterRestart, Strings::Preferences::DESC_ROM_BASIC_PATH);
            }

            // CHARGEN
            {
                wxFileProperty* filePropertyHandler = new wxFileProperty(Strings::Preferences::OPT_ROM_CHARGEN_PATH);
                AddWrappedProp(Settings::AppSettings::ID::RomChargenPath, TypeSerialized::String, filePropertyHandler, *page, Effective::AfterRestart, Strings::Preferences::DESC_ROM_CHARGEN_PATH);
            }
        }
    }

    // System
    page->Append(new wxPropertyCategory(Strings::Preferences::CATEGORY_APPLICATION));
    {
        AddWrappedProp(Settings::AppSettings::ID::RememberPlaylist, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_REMEMBER_PLAYLIST), *page, Effective::Immediately, Strings::Preferences::DESC_REMEMBER_PLAYLIST);
#ifdef WIN32
        AddWrappedProp(Settings::AppSettings::ID::MediaKeys, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_MEDIA_KEYS), *page, Effective::Immediately, Strings::Preferences::DESC_MEDIA_KEYS);
#endif
        AddWrappedProp(Settings::AppSettings::ID::SingleInstance, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_SINGLE_INSTANCE), *page, Effective::Immediately, Strings::Preferences::DESC_SINGLE_INSTANCE);

        wxString descRestoreDefaultsStr(Strings::Preferences::DESC_RESTORE_DEFAULTS);
#ifndef WIN32
        descRestoreDefaultsStr
            .Append('\n')
            .Append(Strings::Preferences::DESC_RESTORE_DEFAULTS_LINUX)
            .Append(_app.currentSettings->GetSettingsFilePath());
#endif
        AddWrappedProp(Settings::AppSettings::ID::RestoreDefaults, TypeSerialized::Int, new wxBoolProperty(Strings::Preferences::OPT_RESTORE_DEFAULTS), *page, Effective::Immediately, descRestoreDefaultsStr);
    }

    // Final
    page->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, true);
}

void FramePrefs::OnPropertyGridChanging(wxPropertyGridEvent& evt)
{
    const FramePrefs::WrappedProp& wProp = GetWrappedProp(*evt.GetProperty());
    const FramePrefs::SettingId cId = wProp.settingId;

    if (strcmp(cId, Settings::AppSettings::ID::SonglengthsPath) == 0)
    {
        const wxString& pendingValue = evt.GetValue().GetString();
        if (!wxFileExists(pendingValue))
        {
            evt.Veto();
            evt.SetValidationFailureBehavior(wxPGVFBFlags::Beep | wxPGVFBFlags::MarkCell);
        }
    }
    else if (strcmp(cId, Settings::AppSettings::ID::RomKernalPath) == 0)
    {
        const std::wstring pendingValue = evt.GetValue().GetString().ToStdWstring();
        const bool seemsOk = RomUtil::PreCheckRoms(pendingValue, L"", L"").IsValidated(RomUtil::RomType::Kernal);
        if (!seemsOk)
        {
            evt.Veto();
            evt.SetValidationFailureBehavior(wxPGVFBFlags::Beep | wxPGVFBFlags::MarkCell);
            wxMessageBox(Strings::Error::MSG_ERR_ROM_KERNAL, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR);
        }
        else if (SidDecoder _tempSidDecoder; _tempSidDecoder.TrySetRoms(pendingValue, L"", L"").IsValidated(RomUtil::RomType::Kernal))
        {
            wxMessageBox(_tempSidDecoder.GetEngineInfo().kernalDesc(), Strings::Preferences::TITLE_ROM_INFO, wxICON_INFORMATION);
        }
    }
    else if (strcmp(cId, Settings::AppSettings::ID::RomBasicPath) == 0)
    {
        const std::wstring pendingValue = evt.GetValue().GetString().ToStdWstring();
        const bool seemsOk = RomUtil::PreCheckRoms(L"", pendingValue, L"").IsValidated(RomUtil::RomType::Basic);
        if (!seemsOk)
        {
            evt.Veto();
            evt.SetValidationFailureBehavior(wxPGVFBFlags::Beep | wxPGVFBFlags::MarkCell);
            wxMessageBox(Strings::Error::MSG_ERR_ROM_BASIC, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR);
        }
        else if (SidDecoder _tempSidDecoder; _tempSidDecoder.TrySetRoms(L"", pendingValue, L"").IsValidated(RomUtil::RomType::Basic))
        {
            wxMessageBox(_tempSidDecoder.GetEngineInfo().basicDesc(), Strings::Preferences::TITLE_ROM_INFO);
        }
    }
    else if (strcmp(cId, Settings::AppSettings::ID::RomChargenPath) == 0)
    {
        const std::wstring pendingValue = evt.GetValue().GetString().ToStdWstring();
        const bool seemsOk = RomUtil::PreCheckRoms(L"", L"", pendingValue).IsValidated(RomUtil::RomType::Chargen);
        if (!seemsOk)
        {
            evt.Veto();
            evt.SetValidationFailureBehavior(wxPGVFBFlags::Beep | wxPGVFBFlags::MarkCell);
            wxMessageBox(Strings::Error::MSG_ERR_ROM_CHARGEN, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR);
        }
        else if (SidDecoder _tempSidDecoder; _tempSidDecoder.TrySetRoms(L"", L"", pendingValue).IsValidated(RomUtil::RomType::Chargen))
        {
            wxMessageBox(_tempSidDecoder.GetEngineInfo().chargenDesc(), Strings::Preferences::TITLE_ROM_INFO);
        }
    }
}

void FramePrefs::OnPropertyGridChanged(wxPropertyGridEvent& evt)
{
    FramePrefs::WrappedProp& wProp = GetWrappedProp(*evt.GetProperty());
    wProp.changed = true;

    _ui->propertyGrid->RefreshProperty(evt.GetProperty()); // Necessary to refresh in case of error-highlight previously (wxWidgets' edge-case issue).

    FindWindowById(wxID_APPLY, this)->Enable(true);

    // Enable/disable VirtualStereo settings
    if (evt.GetProperty()->GetLabel() == Strings::Preferences::OPT_OUT_CHANNELS)
    {
        RefreshVirtualStereoSettings();
    }
}

void FramePrefs::OnButtonApply(wxCommandEvent& /*evt*/)
{
    if (_ui->propertyGrid->IsAnyModified())
    {
        bool requiresAppRestart = false;
        bool restoreDefaults = false;

        for (auto& prop : _wrappedProps)
        {
            if (!prop.second.changed)
            {
                continue;
            }

            if (prop.second.requiresRestart)
            {
                requiresAppRestart = true;
            }

            Settings::Option& option = *_app.currentSettings->GetOption(prop.first);
            const int propertyValueInt = prop.second.property.GetValue().GetInteger();

            switch (prop.second.type)
            {
                case TypeSerialized::Int:
                    option.UpdateValue(propertyValueInt);

                    if (prop.first == Settings::AppSettings::ID::VirtualStereoSpeakerDistance)
                    {
                        const int millis = propertyValueInt + FXVIRTUALSTEREO_SPEAKER_DISTANCE_MILLISECONDS_MIN;
                        option.UpdateValue(millis);
                    }
                    else if (prop.first == Settings::AppSettings::ID::PreRenderEnabled)
                    {
                        _framePlayer.ForceStopPlayback({});
                    }
                    else if (prop.first == Settings::AppSettings::ID::SongFallbackDuration)
                    {
                        _framePlayer.UpdateIgnoredSongs({}); // Just in case the "skip shorter" is affected by this.
                        if (_app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool())
                        {
                            _framePlayer.ForceStopPlayback({}); // Fallback duration setting can be changed in realtime (and that's immediately reflected in the seekbar), but that's not supported when playing in a pre-render mode (in case new duration is longer), so we simply stop the playback to force new pre-render upon manual playback restart.
                        }
                    }
                    else if (prop.first == Settings::AppSettings::ID::SkipShorter)
                    {
                        _framePlayer.UpdateIgnoredSongs({});
                    }
                    else if (prop.first == Settings::AppSettings::ID::TaskbarProgress)
                    {
                        const int opt = _app.currentSettings->GetOption(Settings::AppSettings::ID::TaskbarProgress)->GetValueAsInt();
	                    _framePlayer.GetUIElements({}).compositeSeekbar->SetTaskbarProgressOption(static_cast<UIElements::CompositeSeekBar::TaskbarProgressOption>(opt));
                    }
                    else if (prop.first == Settings::AppSettings::ID::RememberPlaylist)
                    {
                        if (propertyValueInt == 0)
                        {
                            Helpers::Wx::Files::TrySavePlaylist(Helpers::Wx::Files::DEFAULT_PLAYLIST_NAME, {}); // Immediately delete the default playlist file if it exists.
                            _app.currentSettings->GetOption(Settings::AppSettings::ID::LastSongName)->UpdateValue("");
                            _app.currentSettings->GetOption(Settings::AppSettings::ID::LastSubsongIndex)->UpdateValue(0);
                        }
                    }
                    else if (prop.first == Settings::AppSettings::ID::RestoreDefaults)
                    {
                        restoreDefaults = propertyValueInt == 1;
                    }
                    else if (prop.first == Settings::AppSettings::ID::MediaKeys)
                    {
                        if (propertyValueInt == 1)
                        {
                            _framePlayer.TryRegisterMediaKeys({});
                        }
                        else
                        {
                            _framePlayer.UnregisterMediaKeys({});
                        }
                    }

                    break;

                case TypeSerialized::Double:
                {
                    const double propertyValueDouble = prop.second.property.GetValue().GetDouble();
                    option.UpdateValue(propertyValueDouble);
                    break;
                }

                case TypeSerialized::String:
                    // Reminder: here the option.UpdateValue(...) must be done per-case "manually".
                    if (prop.first == Settings::AppSettings::ID::AudioOutputDevice)
                    {
                        const wxString& deviceName = prop.second.property.GetChoices().Item(propertyValueInt).GetText();
                        option.UpdateValue(deviceName.ToStdWstring());
                    }
                    else if (prop.first == Settings::AppSettings::ID::SonglengthsPath)
                    {
                        const wxString& relPath = Helpers::Wx::Files::AsRelativePathIfPossible(prop.second.property.GetValue().GetString().ToStdWstring());
                        option.UpdateValue(relPath);
                    }
                    else if (prop.first == Settings::AppSettings::ID::StilPath)
                    {
                        const wxString& relPath = Helpers::Wx::Files::AsRelativePathIfPossible(prop.second.property.GetValue().GetString().ToStdWstring());
                        option.UpdateValue(relPath);
                        _framePlayer.InitStilInfo({});
                    }
                    else if (prop.first == Settings::AppSettings::ID::RomKernalPath || prop.first == Settings::AppSettings::ID::RomBasicPath || prop.first == Settings::AppSettings::ID::RomChargenPath)
                    {
                        const wxString& relPath = Helpers::Wx::Files::AsRelativePathIfPossible(prop.second.property.GetValue().GetString().ToStdWstring());
                        option.UpdateValue(relPath);
                    }

                    break;

                default:
                    std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
                    break;
            }

            prop.second.changed = false; // To prevent "Ok" button reapplying again.
        }

        FindWindowById(wxID_APPLY, this)->Enable(false);

        if (restoreDefaults)
        {
            _app.currentSettings->ResetTo(_app.currentSettings->GetDefaultSettings());
            _framePlayer.ForceAppExitOnPrefsClose({}); // Since we're modal, the caller must close the app after we return (wxExit() etc. won't do anything from here).
            Destroy();
        }
        else
        {
            const bool success = _app.ReapplyPlaybackSettings();
            if (success)
            {
                _app.currentSettings->TrySave();
            }

            if (requiresAppRestart)
            {
                wxMessageBox(Strings::Preferences::MSG_APP_RESTART_REQUIRED, Strings::Preferences::WINDOW_TITLE);
            }
        }
    }
}

void FramePrefs::OnButtonOk(wxCommandEvent& evt)
{
    OnButtonApply(evt);
    if (!__destroyed)
    {
        Destroy();
    }
}

void FramePrefs::OnButtonCancel(wxCommandEvent& /*evt*/)
{
    Destroy();
}

FramePrefs::WrappedProp& FramePrefs::GetWrappedProp(const wxPGProperty& property)
{
    const wxString& targetName = property.GetName();
    const auto it = std::find_if(_wrappedProps.begin(), _wrappedProps.end(), [targetName](const auto& keyValue)
    {
        const FramePrefs::WrappedProp& cWrappedProp = keyValue.second;
        //return property.IsSameAs(cWrappedProp.property); // This doesn't work as expected (it just returns a first item).
        return cWrappedProp.property.GetName() == targetName;
    });

    assert(it != _wrappedProps.end());
    return (*it).second;
}

void FramePrefs::RefreshVirtualStereoSettings()
{
    wxPGProperty& propParent = *_ui->propertyGrid->GetPropertyByLabel(Strings::Preferences::OPT_OUT_CHANNELS);
    wxPGProperty& propChild1 = *_ui->propertyGrid->GetPropertyByLabel(Strings::Preferences::OPT_OUT_FXVIRTUALSTEREO_SPEAKER_DISTANCE);
    wxPGProperty& propChild2 = *_ui->propertyGrid->GetPropertyByLabel(Strings::Preferences::OPT_OUT_FXVIRTUALSTEREO_SIDE_VOLUME);

    Settings::AppSettings::OutChannels mode = static_cast<Settings::AppSettings::OutChannels>(propParent.GetValue().GetInteger());
    const bool enabled = mode == Settings::AppSettings::OutChannels::VirtualStereo;
    propChild1.Enable(enabled);
    propChild2.Enable(enabled);
}
