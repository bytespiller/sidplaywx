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

#include "FramePlayer.h"
#include "../MyApp.h"
#include "../Config/AppSettings.h"
#include "../Config/UIStrings.h"
#include "../Helpers/DpiSize.h"
#include "../FrameChildren/FramePlaybackMods/FramePlaybackMods.h"
#include "../FrameChildren/FramePrefs/FramePrefs.h"
#include <wx/aboutdlg.h>

using RepeatMode = UIElements::RepeatModeButton::RepeatMode;
static wxString bundledSonglengthsPath("bundled-Songlengths.md5");

FramePlayer::FramePlayer(const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app)
    : wxFrame(NULL, wxID_ANY, title, pos, size),
    _app(app)
{
    SetIcon(wxICON(appicon)); // Comes from .rc

    InitSonglengthsDatabase();

    _themeManager.LoadTheme("default");
    SetupUiElements();

    // Overall bindings
    Bind(wxEVT_CLOSE_WINDOW, &FramePlayer::OnClose, this);

    SubscribeMe(_app, SignalsMyApp::SIGNAL_SEEKING_CEASED, std::bind(&OnSeekingCeased, this));
    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_PLAYBACK_SPEED_CHANGED, std::bind(&UpdateUiState, this));
    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_VOICE_TOGGLED, std::bind(&UpdateUiState, this));
    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_AUDIO_DEVICE_CHANGED, std::bind(&OnAudioDeviceChanged, this, std::placeholders::_1));

    // Final
    UpdateUiState();
    CallAfter(&DeferredInit);
}

void FramePlayer::IndicateExternalFilesIncoming()
{
    GetStatusBar()->SetStatusText(Strings::FramePlayer::STATUS_EXTERNAL_FILES_INCOMING, 0); // TODO
}

FrameElements::ElementsPlayer& FramePlayer::GetUIElements(PassKey<FramePrefs>)
{
    return *_ui;
}

void FramePlayer::InitSonglengthsDatabase()
{
    /*
    This method handles all combinations of exceptional init failure states (with appropriate error message being generated):
    - a: If specified, try to load user-provided database, unless it's non-existant (MSG_ERR_SONGLENGTHS_NOT_FOUND)
    - b: If specified and exists, try to init user-provided database, unless it's corrupted (MSG_ERR_SONGLENGTHS_INIT_FAILED)
    - c: Otherwise, try to load default database, unless it doesn't exist (MSG_ERR_SONGLENGTHS_NOT_FOUND)
    - d: Otherwise, try to init default database, unless it's corrupted (MSG_ERR_SONGLENGTHS_INIT_FAILED)

    Recovery by loading the default database and clearing the option.
    - If successful, a confirmation suffix is added to the message: MSG_ERR_SONGLENGTHS_FALLBACK_SUCCESS indicating invalidated DB path setting.
        - We have to auto-clear it because there's no way to do it via GUI in the wxPropertyGrid. TODO: revise this comment later.
    */

    const wxString& optionSonglengthsPath = _app.currentSettings->GetOption(Settings::AppSettings::ID::SonglengthsPath)->GetValueAsString();
    wxString exceptionMessage;
    bool success = false;
    for (wxFileName path : {optionSonglengthsPath, bundledSonglengthsPath})
    {
        if (path.GetFullPath().IsEmpty())
        {
            continue;
        }

        path.MakeAbsolute();
        if (!path.Exists())
        {
            exceptionMessage = Strings::Error::MSG_ERR_SONGLENGTHS_NOT_FOUND;
            continue;
        }

        success = _silentSidInfoDecoder.TryInitSidDatabase(path.GetFullPath().ToStdWstring());
        if (success)
        {
            break;
        }
        else
        {
            exceptionMessage = Strings::Error::MSG_ERR_SONGLENGTHS_INIT_FAILED;
        }
    }

    if (!exceptionMessage.IsEmpty())
    {
        if (success)
        {
            exceptionMessage = wxString::Format("%s\n%s", exceptionMessage, Strings::Error::MSG_ERR_SONGLENGTHS_FALLBACK_SUCCESS);
        }
        wxMessageBox(exceptionMessage, Strings::FramePlayer::WINDOW_TITLE, wxICON_EXCLAMATION);
        _app.currentSettings->GetOption(Settings::AppSettings::ID::SonglengthsPath)->UpdateValue(""); // Reminder: leave this even if resettable in Prefs.
    }
}

void FramePlayer::SetupUiElements()
{
    assert(!_initialized);
    _initialized = true;

    // Init params
    // TODO: recall size & position if available

    // Visuals
    _panel = new wxPanel(this);
    _panel->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    _panel->SetDoubleBuffered(true); // Panel must be double-buffered, otherwise there's child controls' flicker, and seekbar would not refresh properly.

    // Status bar
    wxStatusBar* statusBar = CreateStatusBar(3); // TODO: move to FrameElements::ElementsPlayer
    const int arr[] = {-1, -2, -1};
    SetStatusWidths(3, arr);
    statusBar->SetDoubleBuffered(true); // Prevent flicker on rapid updates (e.g., adding files).

    wxInitAllImageHandlers();

    _ui = std::make_unique<FrameElements::ElementsPlayer>(*_panel, *_app.currentSettings, _themeManager.GetTheme("default"));

    // Setup controls
    const int cVolume = _app.currentSettings->GetOption(Settings::AppSettings::ID::Volume)->GetValueAsInt();
    _app.SetVolume(static_cast<float>(cVolume) / _ui->sliderVolume->GetMax());
    _ui->sliderVolume->SetValue(cVolume);
    _ui->sliderVolume->SetToolTip(new wxToolTip(wxString::Format("%i%", cVolume)));
    _ui->sliderVolume->Enable(_app.currentSettings->GetOption(Settings::AppSettings::ID::VolumeControlEnabled)->GetValueAsBool());

    { // RepeatMode
        const int modeImageIndex = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatMode)->GetValueAsInt() - 1;
        _ui->btnRepeatMode->SetRepeatModeImage(static_cast<RepeatMode>(modeImageIndex));
    }

    { // Taskbar progress indication
        const int opt = _app.currentSettings->GetOption(Settings::AppSettings::ID::TaskbarProgress)->GetValueAsInt();
	    _ui->compositeSeekbar->SetTaskbarProgressOption(static_cast<UIElements::CompositeSeekBar::TaskbarProgressOption>(opt));
    }

    // Bindings
    _ui->btnPlayPause->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OnButtonPlayPause, this);
    _ui->btnStop->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OnButtonStop, this);
    _ui->btnNextSubsong->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OnButtonSubsongNext, this);
    _ui->btnPrevSubsong->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OnButtonSubsongPrev, this);
    _ui->btnNextTrack->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OnButtonTuneNext, this);
    _ui->btnPrevTrack->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OnButtonTunePrev, this);
    _ui->btnPlaybackMod->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &OnButtonPlaybackMod, this);

    _ui->sliderVolume->Bind(wxEVT_SLIDER, &OnVolumeSlider, this); // Reminder: context menu is bound in the FrameElementsPlayer to keep stuff tidy.
    _ui->btnRepeatMode->Bind(wxEVT_BUTTON, &OnButtonRepeatMode, this);

    _ui->compositeSeekbar->Bind(UIElements::EVT_CSB_SeekBackward, &OnSeekBackward, this);
    _ui->compositeSeekbar->Bind(UIElements::EVT_CSB_SeekForward, &OnSeekForward, this);

    _ui->treePlaylist->GetBase().Bind(wxEVT_TREE_ITEM_ACTIVATED, &OnTreePlaylistItemActivated, this);
    _ui->treePlaylist->GetBase().Bind(wxEVT_CONTEXT_MENU, &OnTreePlaylistContextMenuOpen, this);
    _ui->treePlaylist->GetBase().Bind(wxEVT_TREE_KEY_DOWN, &OnTreePlaylistKeyPressed, this);

    SubscribeMe(*_ui->btnRepeatMode, UIElements::SignalsRepeatModeButton::SIGNAL_EXTRA_OPTION_SELECTED, [this](int param)
    {
        OnRepeatModeExtraOptionToggled(static_cast<ExtraOptionId>(param));
    });

    _timer = std::make_unique<wxTimer>(this);
    Bind(wxEVT_TIMER, &OnTimer, this);

    // Menu
    SetMenuBar(_ui->menuBar->GetWxMenuBarPtr());
    Bind(wxEVT_COMMAND_MENU_SELECTED, &OnMenuItemSelected, this);

    // Other
    SetMinClientSize({_ui->labelTitle->GetSize().GetWidth() + _ui->labelTitle->GetPosition().x, _ui->treePlaylist->GetBase().GetPosition().y});

    _panel->DragAcceptFiles(true);
    _panel->Bind(wxEVT_DROP_FILES, &OnDropFilesFramePlayer, this);

    _ui->treePlaylist->GetBase().DragAcceptFiles(true);
    _ui->treePlaylist->GetBase().Bind(wxEVT_DROP_FILES, &OnDropFilesPlaylist, this);
}

void FramePlayer::DeferredInit()
{
    SetClientSize(DpiSize(530, 512)); // TODO: load from options

    // Handle the "Open With" situation (not done in a constructor in order to have the wxYield/Refresh work while adding to playlist).
    if (_app.argc > 1)
    {
        DiscoverFilesAndSendToPlaylist(_app.argv.GetArguments());
    }
}

void FramePlayer::SetRefreshTimerInterval(int desiredInterval)
{
    _timerCanonicalRefreshInterval = desiredInterval;

    const int cInterval = _timer->GetInterval();
    const int effectiveInterval = (IsIconized()) ? TIMER_REFRESH_INTERVAL_IDLE : _timerCanonicalRefreshInterval;
    if (cInterval != effectiveInterval)
    {
        _timer->Start(effectiveInterval);
    }
}

void FramePlayer::CloseApplication()
{
    _exitingApplication = true;
    _timer->Stop(); // Segfault can occur otherwise.
    _app.StopPlayback();

    // TODO: save window settings etc.
    _app.currentSettings->GetOption(Settings::AppSettings::ID::Volume)->UpdateValue(_ui->sliderVolume->GetValue());
    _app.currentSettings->GetOption(Settings::AppSettings::ID::VolumeControlEnabled)->UpdateValue(_ui->sliderVolume->IsEnabled());

    _app.currentSettings->TrySave();

#if defined(_WIN32) && defined(wxUSE_DDE_FOR_IPC)
    new wxLogNull; // Suppress popup-errors (irrelevant DDE DMLERR_INVALIDPARAMETER edge-case message which I think might be a wx library defect and I can't do anything about it).
#endif

    wxExit();
}

void FramePlayer::OpenPlaybackModFrame()
{
    if (_framePlaybackMods != nullptr)
    {
        if (_framePlaybackMods->IsVisible())
        {
            _framePlaybackMods->Hide();
        }
        else
        {
            _framePlaybackMods->Show();
        }

        return;
    }

    _framePlaybackMods = new FramePlaybackMods(this, Strings::PlaybackMods::WINDOW_TITLE, wxDefaultPosition, DpiSize(430, 240), _app);
    _framePlaybackMods->Show();
}

void FramePlayer::OpenPrefsFrame()
{
    _framePrefs = new FramePrefs(this, Strings::Preferences::WINDOW_TITLE, wxDefaultPosition, DpiSize(430, 500), _app, *this);
    _framePrefs->ShowModal(); // Reminder: this one gets Destroy()-ed, not Close()-d.
    if (_exitingApplication)
    {
        CloseApplication();
    }
}

void FramePlayer::ForceAppExitOnPrefsClose(PassKey<FramePrefs>)
{
    assert(_framePrefs != nullptr && _framePrefs->IsVisible());
    _exitingApplication = true;
}

void FramePlayer::DisplayAboutBox()
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(Strings::FramePlayer::WINDOW_TITLE);
    aboutInfo.SetVersion("0.6.0"); // TODO
    aboutInfo.SetDescription(Strings::About::DESCRIPTION);
    aboutInfo.SetCopyright(L"(C) 2021 Jasmin Rutić"); // TODO
    aboutInfo.SetWebSite("https://github.com/bytespiller/sidplaywx");

    aboutInfo.SetLicense(Strings::About::LICENSE);

    aboutInfo.AddDeveloper(wxString(Strings::About::DEVELOPER_LIBRARIES) + "\n" +
                           wxString::Format("%s %s", _silentSidInfoDecoder.GetEngineInfo().name(), _silentSidInfoDecoder.GetEngineInfo().version()) + "\n" + // libsidplayfp
                           wxString(Pa_GetVersionInfo()->versionText) + "\n" + // PortAudio
                           wxVERSION_STRING + "\n" + // wxWidgets
                           wxString("Nano SVG (https://github.com/memononen/nanosvg)")
                          );

    aboutInfo.AddArtist(Strings::About::ARTISTS_HVSC);

    wxAboutBox(aboutInfo);
}
