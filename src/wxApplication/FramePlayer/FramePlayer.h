/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "ElementsPlayer.h"
#include "../Theme/ThemeManager.h"
#include "../../PlaybackController/PlaybackWrappers/Input/SidDecoder.h"
#include "../../Util/SimpleSignal/SimpleSignalListener.h"

class FramePlaybackMods;
class FramePrefs;
class MyApp;

enum class SignalsMyApp;
enum class SignalsPlaybackController;

namespace UIElements
{
    enum class SignalsRepeatModeButton;
    enum class SignalsSearchBar;
}

class FramePlayer : public wxFrame, private SimpleSignalListener<SignalsMyApp>, private SimpleSignalListener<SignalsPlaybackController>, private SimpleSignalListener<UIElements::SignalsRepeatModeButton>, private SimpleSignalListener<UIElements::SignalsSearchBar>
{
private:
    using SimpleSignalListener<SignalsMyApp>::SubscribeMe;
    using SimpleSignalListener<SignalsPlaybackController>::SubscribeMe;
    using SimpleSignalListener<UIElements::SignalsRepeatModeButton>::SubscribeMe;
    using SimpleSignalListener<UIElements::SignalsSearchBar>::SubscribeMe;

private:
    using ExtraOptionId = UIElements::RepeatModeButton::ExtraOptionsHandler::ExtraOptionId;

private:
    static constexpr int TIMER_REFRESH_INTERVAL_HIGH = 10;
    static constexpr int TIMER_REFRESH_INTERVAL_IDLE = 100;

    static constexpr int TIMER_INTERVAL_GLOBAL_HOTKEYS_POLLING = 30;

public:
    template <typename T>
    class PassKey { friend T; PassKey() {} PassKey(PassKey const&) {} };

public:
    FramePlayer() = delete;
    FramePlayer(const wxString& title, const wxPoint& pos, const wxSize& size, MyApp& app);

#pragma region *** main ***

public:
    void IndicateExternalFilesIncoming();
    FrameElements::ElementsPlayer& GetUIElements(PassKey<FramePrefs>);
    void ForceAppExitOnPrefsClose(PassKey<FramePrefs>);
    void ForceStopPlayback(PassKey<FramePrefs>);

private:
    // File
    void InitSonglengthsDatabase();
    void SetupUiElements();
    void DeferredInit();
    void SetRefreshTimerInterval(int desiredInterval);
    void CloseApplication();

    // Edit
    void OpenSearchBar();
    void OpenPlaybackModFrame();
    void OpenPrefsFrame();

    // View
    void ToggleTopmost();
    bool IsTopmost() const; // Helper

    void ToggleVisualizationEnabled();
    void EnableVisualization(bool enable); // Helper

    // Help
    void CheckUpdates();
    void DisplayAboutBox();

#pragma endregion
#pragma region *** visual ***

private:
    void UpdateUiState();
    void UpdatePlaybackStatusBar();
    void UpdatePeriodicDisplays(const uint_least32_t playbackTimeMs);
    void DisplayCurrentSongInfo(bool justClear = false);
    void SetRefreshTimerThrottled(bool throttle);

#pragma endregion
#pragma region *** playlist (iodetail) ***

public:
    std::vector<wxString> GetCurrentPlaylistFilePaths(bool includeBlacklistedSongs);
    void DiscoverFilesAndSendToPlaylist(const wxArrayString& rawPaths, bool clearPrevious = true, bool autoPlayFirstImmediately = true);
    void UpdateIgnoredSongs(PassKey<FramePrefs>);

private:
    void SendFilesToPlaylist(const wxArrayString& files, bool clearPrevious = true, bool autoPlayFirstImmediately = true);
    void PadColumnsWidth();
    void PadColumnWidth(PlaylistTreeModel::ColumnId columnId);
    void UpdateIgnoredSongs();
    void UpdateIgnoredSong(PlaylistTreeModelNode& mainSongNode);
    long GetEffectiveSongDuration(const PlaylistTreeModelNode& node) const;

#pragma endregion
#pragma region *** input ***

private:
    void BrowseFilesAndAddToPlaylist(bool enqueue);
    void BrowseFoldersAndAddToPlaylist(bool enqueue);
    void OpenNewPlaylist(bool autoPlayFirstImmediately);
    bool TrySaveCurrentPlaylist();

#pragma endregion
#pragma region *** transport ***

private:
    bool TryPlayPlaylistItem(const PlaylistTreeModelNode& node);

    bool TryPlayNextValidSong();
    bool TryPlayPrevValidSong();
    bool TryPlayNextValidSubsong();
    bool TryPlayPrevValidSubsong();

#pragma endregion
#pragma region *** wx Event handlers ***

private:
    void OnButtonPlayPause(wxCommandEvent& evt);
    void OnButtonStop(wxCommandEvent& evt);
    void OnButtonSubsongNext(wxCommandEvent& evt);
    void OnButtonSubsongPrev(wxCommandEvent& evt);
    void OnButtonTuneNext(wxCommandEvent& evt);
    void OnButtonTunePrev(wxCommandEvent& evt);
    void OnButtonPlaybackMod(wxCommandEvent& evt);

    void OnVolumeSlider(wxCommandEvent& evt);
    void OnButtonRepeatMode(wxCommandEvent& evt);
    void OnSeekBackward(wxCommandEvent& evt);
    void OnSeekForward(wxCommandEvent& evt);

    void OnTreePlaylistItemActivated(wxDataViewEvent& evt);
    void OnTreePlaylistContextMenuOpen(wxDataViewEvent& evt);
    void OnTreePlaylistContextItem(PlaylistTreeModelNode& node, wxCommandEvent& evt);
    void OnTreePlaylistKeyPressed(wxKeyEvent& evt);
    void OnDropFilesFramePlayer(wxDropFilesEvent& evt);
    void OnDropFilesPlaylist(wxDropFilesEvent& evt);

    void OnMenuOpening(wxMenuEvent& evt);
    void OnMenuItemSelected(wxCommandEvent& evt);

    void OnTimerGlobalHotkeysPolling(wxTimerEvent& evt);
    void OnTimerRefresh(wxTimerEvent& evt);

    void OnIconize(wxIconizeEvent& evt);
    void OnClose(wxCloseEvent& evt);

#pragma endregion
#pragma region *** Non-wx events ***

private:
    void OnButtonPlayPause();
    void OnButtonStop();
    bool OnButtonSubsongNext();
    bool OnButtonSubsongPrev();
    bool OnButtonTuneNext();
    bool OnButtonTunePrev();

    void OnSongDurationReached();
    void OnSeekingCeased();
    void OnRepeatModeExtraOptionToggled(ExtraOptionId extraOptionId);

    void OnAudioDeviceChanged(bool success);

    PlaylistTreeModelNode* DoFindSong(const wxString& query, const PlaylistTreeModelNode& startNode, bool forwardDirection, bool restart);
    void OnFindSong(UIElements::SignalsSearchBar signalId);
    void DoRemoveSongTreeItem(PlaylistTreeModelNode& node);
    void DoRemoveAllSongTreeItemsAbove(PlaylistTreeModelNode& node);
    void DoRemoveAllSongTreeItemsBelow(PlaylistTreeModelNode& node);
    void DoToggleSubsongBlacklistState(PlaylistTreeModelNode& node);

#pragma endregion

private:
    bool _initialized = false;
    bool _exitingApplication = false;
    MyApp& _app;
    wxPanel* _panel;
    ThemeManager _themeManager;
    SidDecoder _silentSidInfoDecoder;
    std::unique_ptr<FrameElements::ElementsPlayer> _ui;
    std::unique_ptr<wxTimer> _timerRefresh;
    std::unique_ptr<wxTimer> _timerGlobalHotkeysPolling;
    FramePlaybackMods* _framePlaybackMods = nullptr;
    FramePrefs* _framePrefs = nullptr;
    wxArrayString _enqueuedFiles;
    bool _addingFilesToPlaylist = false;
    int _canonicalTimerRefreshInterval = TIMER_REFRESH_INTERVAL_IDLE;
};
