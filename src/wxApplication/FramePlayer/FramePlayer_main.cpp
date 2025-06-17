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

#include "FramePlayer.h"
#include "../MyApp.h"
#include "../Config/AppSettings.h"
#include "../Config/UIStrings.h"
#include "../Helpers/DpiSize.h"
#include "../Helpers/HelpersWx.h"
#include "../FrameChildren/FramePlaybackMods/FramePlaybackMods.h"
#include "../FrameChildren/FramePrefs/FramePrefs.h"
#include "../FrameChildren/FrameTuneInfo/FrameTuneInfo.h"

#include <wx/aboutdlg.h>
#include <wx/display.h>

#ifndef WIN32
#include <wx/tooltip.h>
#include "../../../dev/icon_src/sidplaywx_icon_64x64.xpm" // sidplaywx_icon_64px[_xpm]
#endif

#include <wx/webrequest.h>

using RepeatMode = UIElements::RepeatModeButton::RepeatMode;

static constexpr size_t VISUALIZATION_WAVE_WINDOW_MS = 100;
static const wxString bundledSonglengthsPath("bundled-Songlengths.md5");
static const wxString bundledStilPath("bundled-STIL.txt");

FramePlayer::FramePlayer(const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app)
    : wxFrame(NULL, wxID_ANY, title, pos, size),
    _app(app)
{
#ifdef WIN32
    SetIcon(wxICON(appicon)); // Comes from .rc
#else
    SetIcon(wxICON(sidplaywx_icon_64px)); // Comes from .xpm file (variable in there needs to be const, GIMP saves it wrongly)
#endif

    _themeManager.LoadTheme("default");
    SetupUiElements();

    // Overall bindings
    Bind(wxEVT_CLOSE_WINDOW, &FramePlayer::OnClose, this);

    SubscribeMe(_app, SignalsMyApp::SIGNAL_SEEKING_CEASED, std::bind(&OnSeekingCeased, this));
    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_PLAYBACK_SPEED_CHANGED, std::bind(&UpdateUiState, this));
    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_VOICE_TOGGLED, std::bind(&UpdateUiState, this));
    SubscribeMe(_app.GetPlaybackSignalProvider(), SignalsPlaybackController::SIGNAL_AUDIO_DEVICE_CHANGED, std::bind(&OnAudioDeviceChanged, this, std::placeholders::_1));

    SubscribeMe(*_ui->searchBar, UIElements::SignalsSearchBar::SIGNAL_FIND_NEXT, std::bind(&OnFindSong, this, UIElements::SignalsSearchBar::SIGNAL_FIND_NEXT));
    SubscribeMe(*_ui->searchBar, UIElements::SignalsSearchBar::SIGNAL_FIND_PREV, std::bind(&OnFindSong, this, UIElements::SignalsSearchBar::SIGNAL_FIND_PREV));

    // Final
    if (!TryRestoreMainWindowPositionAndSize())
    {
        // Default position is implied/automatic here
        SetClientSize(DpiSize(640, 512)); // Hardcoded default size
    }

    const bool maximized = _app.currentSettings->GetOption(Settings::AppSettings::ID::MainWindowMaximized)->GetValueAsBool();
    if (maximized)
    {
        Maximize();
    }

    UpdateUiState();

    Show(); // Allow update immediately.
    Update(); // Immediately paint the window so it appears fully formed.
    CallAfter(&DeferredInit); // Init like this so the playlist control is immediately available rather than in a frozen state in case it's loading lots of items.
}

void FramePlayer::IndicateExternalFilesIncoming()
{
    GetStatusBar()->SetStatusText(Strings::FramePlayer::STATUS_EXTERNAL_FILES_INCOMING, 0); // TODO
}

FrameElements::ElementsPlayer& FramePlayer::GetUIElements(PassKey<FramePrefs>)
{
    return *_ui;
}

bool FramePlayer::TryRestoreMainWindowPositionAndSize()
{
    const wxString& rawPos = _app.currentSettings->GetOption(Settings::AppSettings::ID::MainWindowPosition)->GetValueAsString();
    const wxString& rawSize = _app.currentSettings->GetOption(Settings::AppSettings::ID::MainWindowSize)->GetValueAsString();

    if (!rawPos.IsEmpty() && !rawSize.IsEmpty())
    {
        // Extract the settings
        const wxArrayString& rawXY = wxSplit(rawPos, ',');
        const int x = wxAtoi(rawXY.front());
        const int y = wxAtoi(rawXY.back());

        const wxArrayString& rawWH = wxSplit(rawSize, 'x');
        const int w = wxAtoi(rawWH.front());
        const int h = wxAtoi(rawWH.back());

        if (w < GetMinClientSize().GetWidth() || h < GetMinClientSize().GetHeight())
        {
            return false;
        }

        // DPI aware position & size
        const wxPoint& restorePosition = FromDIP(wxPoint(x, y));
        const wxSize& restoreSize = DpiSize(w, h);

        // Restore window position & size (if it fits on the current screen/viewport)
        wxDisplay display(wxDisplay::GetFromWindow(this));
        if (display.GetClientArea().Contains(wxRect(restorePosition, restoreSize)))
        {
            SetPosition(restorePosition);
            SetClientSize(restoreSize);
            return true;
        }
    }

    return false;
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

        success = _sidDatabase.TryLoad(path.GetFullPath().ToStdWstring());
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
        else
        {
            exceptionMessage
                .append('\n')
                .append(Strings::Error::MSG_ERR_SONGLENGTHS_FALLBACK_FAILED);
        }

        wxMessageBox(exceptionMessage, Strings::FramePlayer::WINDOW_TITLE, wxICON_EXCLAMATION);
        _app.currentSettings->GetOption(Settings::AppSettings::ID::SonglengthsPath)->UpdateValue(""); // Reminder: leave this even if resettable in Prefs.
    }
}

void FramePlayer::InitStilInfo()
{
    /*
    This method handles all combinations of exceptional init failure states (with appropriate error message being generated):
    - a: If specified, try to load user-provided database, unless it's non-existant (MSG_ERR_STIL_NOT_FOUND)
    - b: If specified and exists, try to init user-provided database, unless it's corrupted (MSG_ERR_STIL_INIT_FAILED)
    - c: Otherwise, try to load default database, unless it doesn't exist (MSG_ERR_STIL_NOT_FOUND)
    - d: Otherwise, try to init default database, unless it's corrupted (MSG_ERR_STIL_INIT_FAILED)

    Recovery by loading the default database and clearing the option.
    - If successful, a confirmation suffix is added to the message: MSG_ERR_STIL_FALLBACK_SUCCESS indicating invalidated DB path setting.
        - We have to auto-clear it because there's no way to do it via GUI in the wxPropertyGrid. TODO: revise this comment later.
    */

    if (!_sidDatabase.IsLoaded())
    {
        return; // STIL piggybacks on the Songlengths database to cross-reference the tune's MD5 and HVSC paths. Since it's unavailable, there's no point to init the STIL.
    }

    const wxString& optionStilPath = _app.currentSettings->GetOption(Settings::AppSettings::ID::StilPath)->GetValueAsString();

    wxString exceptionMessage;
    bool success = false;
    for (wxFileName path : {optionStilPath, bundledStilPath})
    {
        if (path.GetFullPath().IsEmpty())
        {
            continue;
        }

        path.MakeAbsolute();
        if (!path.Exists())
        {
            exceptionMessage = Strings::Error::MSG_ERR_STIL_NOT_FOUND;
            continue;
        }

        GetStatusBar()->PushStatusText(Strings::FramePlayer::STATUS_LOADING_STIL, 0);
        success = _stilInfo.TryLoad(path.GetFullPath().ToStdWstring());
        GetStatusBar()->PopStatusText(0);

        if (success)
        {
            break;
        }
        else
        {
            exceptionMessage = Strings::Error::MSG_ERR_STIL_INIT_FAILED;
        }
    }

    if (!exceptionMessage.IsEmpty())
    {
        if (success)
        {
            exceptionMessage = wxString::Format("%s\n%s", exceptionMessage, Strings::Error::MSG_ERR_STIL_FALLBACK_SUCCESS);
        }
        wxMessageBox(exceptionMessage, Strings::FramePlayer::WINDOW_TITLE, wxICON_EXCLAMATION);
        _app.currentSettings->GetOption(Settings::AppSettings::ID::StilPath)->UpdateValue(""); // Reminder: leave this even if resettable in Prefs.
    }
}

void FramePlayer::InitStilInfo(PassKey<FramePrefs>)
{
    InitStilInfo();
}

void FramePlayer::SetupUiElements()
{
    assert(!_initialized);
    _initialized = true;

    // Visuals
    _panel = new wxPanel(this);
    SetBackgroundColour(_panel->GetBackgroundColour()); // Ensure the global GetBackgroundColour() returns the panel's (visible) color, not the underlying Window color (which is obscured by the panel).
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
    _ui->sliderVolume->SetToolTip(new wxToolTip(wxString::Format("%i%%", cVolume)));
#ifdef WIN32 // Toggle is problematic on wxGTK & wxOSX (control must always stay enabled).
    _ui->sliderVolume->Enable(_app.currentSettings->GetOption(Settings::AppSettings::ID::VolumeControlEnabled)->GetValueAsBool());
#endif

    // RepeatMode
    {
        const int modeImageIndex = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatMode)->GetValueAsInt() - 1;
        _ui->btnRepeatMode->SetRepeatModeImage(static_cast<RepeatMode>(modeImageIndex));
    }

    // Taskbar progress indication
    {
        const int opt = _app.currentSettings->GetOption(Settings::AppSettings::ID::TaskbarProgress)->GetValueAsInt();
	    _ui->compositeSeekbar->SetTaskbarProgressOption(static_cast<UIElements::CompositeSeekBar::TaskbarProgressOption>(opt));
    }

    // Bindings
    _ui->btnPlayPause->Bind(wxEVT_BUTTON, &OnButtonPlayPause, this);
    _ui->btnStop->Bind(wxEVT_BUTTON, &OnButtonStop, this);
    _ui->btnNextSubsong->Bind(wxEVT_BUTTON, &OnButtonSubsongNext, this);
    _ui->btnPrevSubsong->Bind(wxEVT_BUTTON, &OnButtonSubsongPrev, this);
    _ui->btnNextTrack->Bind(wxEVT_BUTTON, &OnButtonTuneNext, this);
    _ui->btnPrevTrack->Bind(wxEVT_BUTTON, &OnButtonTunePrev, this);
    _ui->btnPlaybackMod->Bind(wxEVT_BUTTON, &OnButtonPlaybackMod, this);
    _ui->btnTuneInfo->Bind(wxEVT_BUTTON, [this](const wxCommandEvent& /*evt*/)
    {
        if (_app.GetPlaybackInfo().IsValidSongLoaded())
        {
            ShowTuneInfo();
        }
    });

    _ui->sliderVolume->Bind(wxEVT_SLIDER, &OnVolumeSlider, this); // Reminder: context menu is bound in the FrameElementsPlayer to keep stuff tidy.
    _ui->btnRepeatMode->Bind(wxEVT_BUTTON, &OnButtonRepeatMode, this);

    _ui->compositeSeekbar->Bind(UIElements::EVT_CSB_SeekBackward, &OnSeekBackward, this);
    _ui->compositeSeekbar->Bind(UIElements::EVT_CSB_SeekForward, &OnSeekForward, this);

    _ui->treePlaylist->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &OnTreePlaylistItemActivated, this);
    _ui->treePlaylist->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &OnTreePlaylistContextMenuOpen, this);
    _ui->treePlaylist->Bind(wxEVT_KEY_DOWN, &OnTreePlaylistKeyPressed, this);

    _ui->labelTitle->Bind(wxEVT_ENTER_WINDOW, &OnLabelTitleMouseOver, this);

    SubscribeMe(*_ui->btnRepeatMode, UIElements::SignalsRepeatModeButton::SIGNAL_EXTRA_OPTION_SELECTED, [this](int param)
    {
        OnRepeatModeExtraOptionToggled(static_cast<ExtraOptionId>(param));
    });

    // Global throttleable UI-update timer
    _timerRefresh = std::make_unique<wxTimer>(this);
    Bind(wxEVT_TIMER, &OnTimerRefresh, this, _timerRefresh->GetId());

    // Menu
    SetMenuBar(_ui->menuBar);
    Bind(wxEVT_MENU_OPEN, &OnMenuOpening, this);
    Bind(wxEVT_COMMAND_MENU_SELECTED, &OnMenuItemSelected, this);

    // Other
    SetMinClientSize({_ui->labelTitle->GetSize().GetWidth() + _ui->labelTitle->GetPosition().x, _ui->treePlaylist->GetPosition().y});

    _panel->DragAcceptFiles(true);
    _panel->Bind(wxEVT_DROP_FILES, &OnDropFilesFramePlayer, this);

    _ui->treePlaylist->DragAcceptFiles(true);
    _ui->treePlaylist->Bind(wxEVT_DROP_FILES, &OnDropFilesPlaylist, this);
    _ui->treePlaylist->SetFocus(); // Playlist should have the focus on app start for easy keyboard navigation.

    // Apply visualization preference
    if (_app.currentSettings->GetOption(Settings::AppSettings::ID::VisualizationEnabled)->GetValueAsBool())
    {
        EnableVisualization(true);
    }
}

void FramePlayer::DeferredInit()
{
    InitSonglengthsDatabase();
    InitStilInfo();

    // Show/hide STIL info
    EnableStilInfoDisplay(_app.currentSettings->GetOption(Settings::AppSettings::ID::StilInfoEnabled)->GetValueAsBool());

    // Apply window topmost preference
    if (_app.currentSettings->GetOption(Settings::AppSettings::ID::StayTopmost)->GetValueAsBool() != IsTopmost())
    {
        ToggleTopmost();
    }

    // Global hotkeys-polling timer
    _timerGlobalHotkeysPolling = std::make_unique<wxTimer>(this);
    Bind(wxEVT_TIMER, &OnTimerGlobalHotkeysPolling, this, _timerGlobalHotkeysPolling->GetId());
    _timerGlobalHotkeysPolling->Start(TIMER_INTERVAL_GLOBAL_HOTKEYS_POLLING);

    // Handle the "Open With" situation (not done in a constructor in order to have the wxYield/Refresh work while adding to playlist).
    if (_app.argc > 1)
    {
        DiscoverFilesAndSendToPlaylist(_app.argv.GetArguments());
    }
    else if (_app.currentSettings->GetOption(Settings::AppSettings::ID::RememberPlaylist)->GetValueAsBool())
    {
        wxString playlistPath = Helpers::Wx::Files::GetConfigFilePath(Helpers::Wx::Files::DEFAULT_PLAYLIST_NAME);

#ifndef WIN32
        if (!wxFileExists(playlistPath))
        {
            playlistPath = Helpers::Wx::Files::DEFAULT_PLAYLIST_NAME; // Use default (read only)
        }
#endif

        if (wxFileExists(playlistPath))
        {
            // Load default playlist from file
            const wxArrayString& playlistFiles = Helpers::Wx::Files::LoadPathsFromPlaylist(playlistPath);
            DiscoverFilesAndSendToPlaylist(playlistFiles, true, false); // Reminder: this line can take a long time but is asynchronous and the program can be used before the next line is reached.

            // Restore last song selection (unless already playing something, e.g., user selected a song while the large playlist is still loading)
            const bool userSelectionAlreadyMade = _ui->treePlaylist->GetActiveSong() != nullptr;
            if (!userSelectionAlreadyMade)
            {
                const wxString& targetFilePath = _app.currentSettings->GetOption(Settings::AppSettings::ID::LastSongName)->GetValueAsString();
                PlaylistTreeModelNode* targetItemNode = _ui->treePlaylist->GetSong(targetFilePath); // Can be nullptr in case the target file path no longer exists or is an empty string.

                // Determine subsong if needed
                if (targetItemNode != nullptr && targetItemNode->GetSubsongCount() > 0)
                {
                    const int targetSubsongIndex = _app.currentSettings->GetOption(Settings::AppSettings::ID::LastSubsongIndex)->GetValueAsInt();
                    targetItemNode = &targetItemNode->GetSubsong(targetSubsongIndex);
                }

                // Pre-select the song
                if (targetItemNode != nullptr)
                {
                    TryPlayPlaylistItem(*targetItemNode);
                    OnButtonStop();
                }
            }
        }
    }
}

void FramePlayer::SetRefreshTimerInterval(int desiredInterval)
{
    _canonicalTimerRefreshInterval = desiredInterval;

    const int cInterval = _timerRefresh->GetInterval();
    const int effectiveInterval = (IsIconized()) ? TIMER_REFRESH_INTERVAL_IDLE : _canonicalTimerRefreshInterval;
    if (cInterval != effectiveInterval)
    {
        _timerRefresh->Start(effectiveInterval);
    }
}

void FramePlayer::CloseApplication()
{
    _exitingApplication = true;

    if (_timerGlobalHotkeysPolling != nullptr)
    {
        _timerGlobalHotkeysPolling->Stop();
    }

    if (_timerRefresh != nullptr)
    {
        _timerRefresh->Stop(); // Segfault can occur otherwise.
    }

    _app.StopPlayback();
    Hide();

    // Main window size & position
    _app.currentSettings->GetOption(Settings::AppSettings::ID::MainWindowMaximized)->UpdateValue(IsMaximized());
    if (!IsMaximized()) // Don't save position & size if maximized (remember non-maximized state).
    {
        _app.currentSettings->GetOption(Settings::AppSettings::ID::MainWindowPosition)->UpdateValue(wxString::Format("%i,%i", GetScreenPosition().x, GetScreenPosition().y));
        _app.currentSettings->GetOption(Settings::AppSettings::ID::MainWindowSize)->UpdateValue(wxString::Format("%ix%i", GetClientSize().x, GetClientSize().y));
    }

    // Audio volume
    _app.currentSettings->GetOption(Settings::AppSettings::ID::Volume)->UpdateValue(_ui->sliderVolume->GetValue());
    _app.currentSettings->GetOption(Settings::AppSettings::ID::VolumeControlEnabled)->UpdateValue(_ui->sliderVolume->IsEnabled());

    // Save the current playlist
    if (_app.currentSettings->GetOption(Settings::AppSettings::ID::RememberPlaylist)->GetValueAsBool())
    {
        // Save playlist...
        const std::vector<wxString>& fileList = GetCurrentPlaylistFilePaths(false);
        Helpers::Wx::Files::TrySavePlaylist(Helpers::Wx::Files::DEFAULT_PLAYLIST_NAME, fileList);

        // Store the last played song & subsong as an internal option...
        const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetActiveSong();
        const wxString& cSongName = (node == nullptr) ? "" : node->filepath;
        const int cSongIndex = (node == nullptr) ? 0 : node->defaultSubsong;
        _app.currentSettings->GetOption(Settings::AppSettings::ID::LastSongName)->UpdateValue(cSongName);
        _app.currentSettings->GetOption(Settings::AppSettings::ID::LastSubsongIndex)->UpdateValue(cSongIndex);
    }

    _app.currentSettings->TrySave();

#if defined(WIN32) && defined(wxUSE_DDE_FOR_IPC)
    new wxLogNull; // Suppress popup-errors (irrelevant DDE DMLERR_INVALIDPARAMETER edge-case message which I think might be a wx library defect and I can't do anything about it).
#endif

    wxExit();
}

void FramePlayer::OpenSearchBar()
{
    _ui->searchBar->ToggleVisibility();
}

void FramePlayer::OpenPlaybackModFrame()
{
    // Toggle open-close (if exists)
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

    // First-time create
    _framePlaybackMods = new FramePlaybackMods(this, Strings::PlaybackMods::WINDOW_TITLE, wxDefaultPosition, wxSize(600, 350), _app);
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

void FramePlayer::ToggleTopmost()
{
    const bool topmost = IsTopmost();
    if (topmost) {
        SetWindowStyle(GetWindowStyle() & ~wxSTAY_ON_TOP);
    }
    else {
        SetWindowStyle(GetWindowStyle() | wxSTAY_ON_TOP);
    }

    _ui->menuBar->Check(static_cast<int>(FrameElements::ElementsPlayer::MenuItemId_Player::StayTopmost), !topmost);
    _app.currentSettings->GetOption(Settings::AppSettings::ID::StayTopmost)->UpdateValue(!topmost);
}

void FramePlayer::ToggleVisualizationEnabled()
{
    const bool enable = !_app.currentSettings->GetOption(Settings::AppSettings::ID::VisualizationEnabled)->GetValueAsBool();
    _app.currentSettings->GetOption(Settings::AppSettings::ID::VisualizationEnabled)->UpdateValue(enable);
    EnableVisualization(enable);
}

void FramePlayer::ToggleStilInfoEnabled()
{
    const bool toggle = !_app.currentSettings->GetOption(Settings::AppSettings::ID::StilInfoEnabled)->GetValueAsBool();
    EnableStilInfoDisplay(toggle);
    _app.currentSettings->GetOption(Settings::AppSettings::ID::StilInfoEnabled)->UpdateValue(toggle);
}

void FramePlayer::EnableStilInfoDisplay(bool enable)
{
    const bool shouldShow = enable && _stilInfo.IsLoaded();

    if (shouldShow && _app.GetPlaybackInfo().IsValidSongLoaded())
    {
        DisplayCurrentSongInfo();
    }
    else // Clear (disable ScrollingLabels' internal timers)
    {
        _ui->labelStilNameTitle->SetText("");
        _ui->labelStilArtistAuthor->SetText("");
        _ui->labelStilComment->SetText("");
    }

    _ui->sizerStilRight->Show(shouldShow);
    _ui->sizerStilRight->GetContainingWindow()->Layout();

    _ui->menuBar->Check(static_cast<int>(FrameElements::ElementsPlayer::MenuItemId_Player::StilInfoEnabled), shouldShow);
}

void FramePlayer::EnableVisualization(bool enable)
{
    const size_t bufferWindowSize = _app.SetVisualizationWaveformWindow((enable) ? VISUALIZATION_WAVE_WINDOW_MS : 0);
    _ui->waveformVisualization->SetBufferWindow(bufferWindowSize);
    _ui->waveformVisualization->Show(enable);
    _panel->Layout();

    _ui->menuBar->Check(static_cast<int>(FrameElements::ElementsPlayer::MenuItemId_Player::VisualizationEnabled), enable);
}

void FramePlayer::ShowTuneInfo()
{
    // Open (if exists)
    if (_frameTuneInfo != nullptr)
    {
        _frameTuneInfo->ShowAndUpdate(_ui->treePlaylist->GetActiveSong());
        return;
    }

    // First-time create
    _frameTuneInfo = new FrameTuneInfo(this, Strings::TuneInfo::WINDOW_TITLE, wxDefaultPosition, wxSize(640, 490), _app.GetPlaybackInfo(), _stilInfo);
    _frameTuneInfo->ShowAndUpdate(_ui->treePlaylist->GetActiveSong());
}

bool FramePlayer::IsTopmost() const
{
    return (GetWindowStyle() & wxSTAY_ON_TOP) != 0;
}

void FramePlayer::ForceAppExitOnPrefsClose(PassKey<FramePrefs>)
{
    assert(_framePrefs != nullptr && _framePrefs->IsVisible());
    _exitingApplication = true;
}

void FramePlayer::ForceStopPlayback(PassKey<FramePrefs>)
{
    OnButtonStop();
}

static int requestUpdateLastId = 0;
void FramePlayer::CheckUpdates()
{
    ++requestUpdateLastId; // Workaround: can't cancel the previous wxWebRequest due to its problematic lifecycle on Linux (a known wxWidgets defect from the year 2024). Using the unique_ptr or vector of requests causes a segfault on Linux in wxWidgets v3.2.7 when the check is repeated or the app is closing.
    wxWebRequest requestUpdateCheck = wxWebSession::GetDefault().CreateRequest(this, "https://api.github.com/repos/bytespiller/sidplaywx/releases/latest", requestUpdateLastId);

    requestUpdateCheck.SetHeader("User-Agent", "sidplaywx (update check)");
    //requestUpdateCheck.SetHeader("Accept-Encoding", "gzip"); // I never got the GitHub API to return compressed response (checked via headers).
    requestUpdateCheck.SetHeader("Accept", "application/vnd.github+json");
    requestUpdateCheck.SetHeader("X-GitHub-Api-Version", "2022-11-28"); // If you specify an API version that is no longer supported, you will receive a 400 error.

    Bind(wxEVT_WEBREQUEST_STATE, [&](wxWebRequestEvent& evt)
    {
        if (evt.GetId() != requestUpdateLastId)
        {
            return; // React only on the latest one (workaround for not being able to fully wxWebRequest::Cancel the previous request reliably).
        }

        switch (evt.GetState())
        {
            case wxWebRequest::State_Completed:
            {
                const wxWebResponse& r = evt.GetResponse();
                if (r.IsOk() && r.GetStatus() == 200)
                {
                    wxString remoteVersionTag; // Note: starts with "v"
                    if (Helpers::Wx::ExtractValue(r.AsString(), "\"tag_name\":\"", "\"", remoteVersionTag))
                    {
                        const bool latest = remoteVersionTag == wxString("v").Append(Strings::APP_VERSION_TAG);
                        if (latest)
                        {
                            wxMessageBox(wxString::Format(Strings::FramePlayer::UPDATE_CHECK_LATEST, Strings::APP_VERSION_TAG), Strings::FramePlayer::WINDOW_TITLE);
                        }
                        else if (wxMessageBox(wxString::Format(Strings::FramePlayer::UPDATE_CHECK_NEWER, remoteVersionTag, Strings::APP_VERSION_TAG), Strings::FramePlayer::WINDOW_TITLE, wxYES_NO) == wxYES)
                        {
                            wxLaunchDefaultBrowser("https://github.com/bytespiller/sidplaywx/releases/latest");
                        }
                    }
                    else
                    {
                        wxMessageBox("Update check failed.", Strings::FramePlayer::WINDOW_TITLE);
                    }
                }
                else
                {
                    wxMessageBox("Update check failed.\nStatus code: " + r.GetStatus(), Strings::FramePlayer::WINDOW_TITLE);
                }

                break;
            }
            case wxWebRequest::State_Failed:
                wxMessageBox("Update check failed.\nRequest failed: " + evt.GetErrorDescription(), Strings::FramePlayer::WINDOW_TITLE);
                break;
        }
    });

    requestUpdateCheck.Start();
}

void FramePlayer::DisplayAboutBox()
{
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName(Strings::FramePlayer::WINDOW_TITLE);
    aboutInfo.SetVersion(Strings::APP_VERSION_TAG); // Reminder: don't forget to increase.
    aboutInfo.SetDescription(Strings::About::DESCRIPTION);
    aboutInfo.SetCopyright(L"(C) 2021-2025 Jasmin Rutić"); // Reminder: don't forget to bump.
    aboutInfo.SetWebSite("https://github.com/bytespiller/sidplaywx");

    aboutInfo.SetLicense(Strings::About::LICENSE);

    aboutInfo.AddDeveloper(wxString(Strings::About::DEVELOPER_LIBRARIES) + "\n" +
                           wxString::Format("%s %s", _silentSidInfoDecoder.GetEngineInfo().name(), _silentSidInfoDecoder.GetEngineInfo().version()) + "\n" + // libsidplayfp
                           wxString(Pa_GetVersionInfo()->versionText) + "\n" + // PortAudio
                           wxVERSION_STRING // wxWidgets
                          );

    aboutInfo.AddArtist(Strings::About::HVSC);

#ifndef WIN32
    aboutInfo.SetIcon(wxICON(sidplaywx_icon_64px));
#endif

    wxAboutBox(aboutInfo);
}
