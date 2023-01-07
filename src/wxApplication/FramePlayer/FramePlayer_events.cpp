/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2023 Jasmin Rutic (bytespiller@gmail.com)
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
#include "../FrameChildren/FramePrefs/FramePrefs.h"
#include "../Helpers/HelpersWx.h"

namespace
{
    using RepeatMode = UIElements::RepeatModeButton::RepeatMode;
    using ExtraOptionId = UIElements::RepeatModeButton::ExtraOptionsHandler::ExtraOptionId;
    using MenuItemId_Player = FrameElements::ElementsPlayer::MenuItemId_Player;

    enum class PopupMenuItemId_Playlist : int
    {
        SkipUnskip = 1,
        ScrollToCurrent,
        ExpandAll,
        CollapseAll
    };

    inline bool ShouldAutoPlay(const MyApp& app)
    {
        return app.GetPlaybackInfo().GetState() == PlaybackController::State::Playing || app.currentSettings->GetOption(Settings::AppSettings::ID::AutoPlay)->GetValueAsBool();
    }
}

void FramePlayer::OnButtonPlayPause(wxCommandEvent& /*evt*/)
{
    OnButtonPlayPause();
}

void FramePlayer::OnButtonStop(wxCommandEvent& /*evt*/)
{
    OnButtonStop();
}

void FramePlayer::OnButtonSubsongNext(wxCommandEvent& /*evt*/)
{
    OnButtonSubsongNext();
}

void FramePlayer::OnButtonSubsongPrev(wxCommandEvent& /*evt*/)
{
    OnButtonSubsongPrev();
}

void FramePlayer::OnButtonTuneNext(wxCommandEvent& /*evt*/)
{
    OnButtonTuneNext();
}

void FramePlayer::OnButtonTunePrev(wxCommandEvent& /*evt*/)
{
    OnButtonTunePrev();
}

void FramePlayer::OnButtonPlaybackMod(wxCommandEvent& /*evt*/)
{
    OpenPlaybackModFrame();
}

void FramePlayer::OnVolumeSlider(wxCommandEvent& evt)
{
    const float newVolume = static_cast<float>(evt.GetInt()) / _ui->sliderVolume->GetMax();
    wxToolTip* cTooltip = _ui->sliderVolume->GetToolTip();
    cTooltip->SetTip(std::to_string(evt.GetInt()).append("%"));
    _app.SetVolume(newVolume);
}

void FramePlayer::OnButtonRepeatMode(wxCommandEvent& evt)
{
    const RepeatMode newRepeatMode = (evt.GetInt() == 0) ? _ui->btnRepeatMode->Cycle() : static_cast<RepeatMode>(evt.GetInt());
    Settings::Option* option = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatMode);
    assert(newRepeatMode != RepeatMode::Undefined); // Neither the (correct) default nor loaded value was applied during the UI init!
    option->UpdateValue(static_cast<int>(newRepeatMode));
}

void FramePlayer::OnSeekBackward(wxCommandEvent& evt)
{
    _app.SeekTo(evt.GetExtraLong());
    UpdateUiState();
}

void FramePlayer::OnSeekForward(wxCommandEvent& evt)
{
    _app.SeekTo(evt.GetExtraLong());
    UpdateUiState();
}

void FramePlayer::OnTreePlaylistItemActivated(wxTreeEvent& evt)
{
    if (!evt.GetItem().IsOk())
    {
        return;
    }

    const SongTreeItemData& tuneData = _ui->treePlaylist->GetSongTreeItemData(evt.GetItem());
    TryPlayPlaylistItem(tuneData);
    UpdateUiState();
}

void FramePlayer::OnTreePlaylistContextMenuOpen(wxContextMenuEvent& evt)
{
    _ui->treePlaylist->OnContextMenuOpen(evt);

    wxString menuTextSkipUnskip(Strings::PlaylistTree::MENU_ITEM_REMOVE);
    bool skipUnskipValidItem = false;
    if (const UIElements::Playlist::SongTreeItemData* const itemData = _ui->treePlaylist->TryGetContextMenuSongTreeItemData())
    {
        if (itemData->GetStatus() == UIElements::Playlist::SongTreeItemData::ItemStatus::Normal)
        {
            skipUnskipValidItem = true;
        }
        else if (itemData->GetStatus() == UIElements::Playlist::SongTreeItemData::ItemStatus::IgnoredPlayable)
        {
            skipUnskipValidItem = true;
            menuTextSkipUnskip = Strings::PlaylistTree::MENU_ITEM_RESTORE;
        }
    }

    wxMenu* menu = new wxMenu();

    // Skip/Unskip
    {
        wxMenuItem* newItem = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::SkipUnskip), menuTextSkipUnskip);
        newItem->Enable(skipUnskipValidItem);
    }

    // Scroll to current
    {
        menu->AppendSeparator();
        wxMenuItem* newItem = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::ScrollToCurrent), Strings::PlaylistTree::MENU_ITEM_SCROLL_TO_CURRENT);
        newItem->Enable(!_ui->treePlaylist->IsEmpty());
    }

    // Expand all, Collapse all
    {
        menu->AppendSeparator();
        wxMenuItem* newItemExpand = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::ExpandAll), Strings::PlaylistTree::MENU_ITEM_EXPAND_ALL);
        wxMenuItem* newItemCollapse = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::CollapseAll), Strings::PlaylistTree::MENU_ITEM_COLLAPSE_ALL);

        newItemExpand->Enable(!_ui->treePlaylist->IsEmpty());
        newItemCollapse->Enable(!_ui->treePlaylist->IsEmpty());
    }

    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, &OnTreePlaylistContextItem, this);
    PopupMenu(menu);
}

void FramePlayer::OnTreePlaylistContextItem(wxCommandEvent& evt)
{
    const PopupMenuItemId_Playlist id(static_cast<PopupMenuItemId_Playlist>(evt.GetId()));

    switch (id)
    {
        case PopupMenuItemId_Playlist::SkipUnskip:
        {
            if (const UIElements::Playlist::SongTreeItemData* const itemData = _ui->treePlaylist->TryGetContextMenuSongTreeItemData())
            {
                if (itemData->GetStatus() == UIElements::Playlist::SongTreeItemData::ItemStatus::Normal)
                {
                    _ui->treePlaylist->IgnoreSong(itemData->GetId(), static_cast<int>(FrameElements::PlaylistIconId::RemoveSong));
                }
                else if (itemData->GetStatus() == UIElements::Playlist::SongTreeItemData::ItemStatus::IgnoredPlayable)
                {
                    _ui->treePlaylist->RestoreIgnoredSong(itemData->GetId());
                }
                UpdateUiState(); // To refresh the Next/Prev buttons.
            }
            break;
        }
        case PopupMenuItemId_Playlist::ScrollToCurrent:
        {
            if (const UIElements::Playlist::SongTreeItemData* const cItemData = _ui->treePlaylist->TryGetCurrentSongTreeItemData())
            {
                _ui->treePlaylist->GetBase().EnsureVisible(cItemData->GetId());
            }
            break;
        }
        case PopupMenuItemId_Playlist::ExpandAll:
        {
            _ui->treePlaylist->GetBase().ExpandAll();
            break;
        }
        case PopupMenuItemId_Playlist::CollapseAll:
        {
            _ui->treePlaylist->GetBase().CollapseAll();
            break;
        }
    }
}

void FramePlayer::OnTreePlaylistKeyPressed(wxTreeEvent& evt)
{
    const int keycode = evt.GetKeyCode();
    if (keycode == WXK_DELETE || keycode == WXK_NUMPAD_DELETE)
    {
        const wxTreeItemId& itemId = _ui->treePlaylist->GetBase().GetSelection();
        if (itemId.IsOk())
        {
            const UIElements::Playlist::SongTreeItemData& itemData = _ui->treePlaylist->GetSongTreeItemData(itemId);
            const auto status = itemData.GetStatus();
            if (status == UIElements::Playlist::SongTreeItemData::ItemStatus::Normal)
            {
                _ui->treePlaylist->IgnoreSong(itemData.GetId(), static_cast<int>(FrameElements::PlaylistIconId::RemoveSong));
            }
            else if (status == UIElements::Playlist::SongTreeItemData::ItemStatus::IgnoredPlayable)
            {
                _ui->treePlaylist->RestoreIgnoredSong(itemData.GetId());
            }
        }
    }
}

void FramePlayer::OnDropFilesFramePlayer(wxDropFilesEvent& evt)
{
    if (_exitingApplication) // In case the user clicked Close while adding lots of files.
    {
        return;
    }

    if (evt.GetNumberOfFiles() > 0)
    {
        const Settings::AppSettings::DragDropMode dragDropMode = static_cast<Settings::AppSettings::DragDropMode>(_app.currentSettings->GetOption(Settings::AppSettings::ID::DragDropMode)->GetValueAsInt());
        if (dragDropMode == Settings::AppSettings::DragDropMode::Disabled)
        {
            return;
        }

        const wxString* dropped = evt.GetFiles();
        wxASSERT(dropped);

        const bool replace = dragDropMode != Settings::AppSettings::DragDropMode::Enqueue;
        DiscoverFilesAndSendToPlaylist(wxArrayString(evt.GetNumberOfFiles(), dropped), replace, replace);
    }
}

void FramePlayer::OnDropFilesPlaylist(wxDropFilesEvent& evt)
{
    if (_exitingApplication) // In case the user clicked Close while adding lots of files.
    {
        return;
    }

    if (evt.GetNumberOfFiles() > 0)
    {
        const Settings::AppSettings::DragDropMode dragDropMode = static_cast<Settings::AppSettings::DragDropMode>(_app.currentSettings->GetOption(Settings::AppSettings::ID::DragDropMode)->GetValueAsInt());
        if (dragDropMode == Settings::AppSettings::DragDropMode::Disabled)
        {
            return;
        }

        const wxString* dropped = evt.GetFiles();
        wxASSERT(dropped);

        const bool replace = dragDropMode == Settings::AppSettings::DragDropMode::Replace;
        DiscoverFilesAndSendToPlaylist(wxArrayString(evt.GetNumberOfFiles(), dropped), replace, replace);
    }
}

void FramePlayer::OnMenuOpening(wxMenuEvent& evt)
{
    if (evt.GetMenu()->GetTitle().IsSameAs(Strings::FramePlayer::MENU_FILE)) // TODO: try to find a better way to detect which menu is opened (e.g., "File" in this case).
    {
        const bool playlistEmpty = _ui->treePlaylist->IsEmpty();
        evt.GetMenu()->Enable(static_cast<int>(MenuItemId_Player::PlaylistSave), !playlistEmpty);
        evt.GetMenu()->Enable(static_cast<int>(MenuItemId_Player::PlaylistClear), !playlistEmpty);
    }
}

void FramePlayer::OnMenuItemSelected(wxCommandEvent& evt)
{
    const MenuItemId_Player id = static_cast<MenuItemId_Player>(evt.GetId());
    switch (id)
    {
        // File
        case MenuItemId_Player::OpenFiles:
            BrowseFilesAndAddToPlaylist(false);
            break;

        case MenuItemId_Player::OpenFolders:
            BrowseFoldersAndAddToPlaylist(false);
            break;

        case MenuItemId_Player::EnqueueFiles:
            BrowseFilesAndAddToPlaylist(true);
            break;

        case MenuItemId_Player::EnqueueFolders:
            BrowseFoldersAndAddToPlaylist(true);
            break;

        // Playlist submenu
        case MenuItemId_Player::PlaylistOpen:
        {
            const bool autoPlayFirstImmediately = _app.currentSettings->GetOption(Settings::AppSettings::ID::AutoPlay)->GetValueAsBool();
            OpenNewPlaylist(autoPlayFirstImmediately);
            break;
        }

        case MenuItemId_Player::PlaylistSave:
            TrySaveCurrentPlaylist();
            break;

        case MenuItemId_Player::PlaylistClear:
            OnButtonStop();
            _ui->treePlaylist->ClearPlaylist();
            UpdateUiState();
            break;
        // **

        case MenuItemId_Player::Exit:
            CloseApplication();
            break;

        // Edit
        case MenuItemId_Player::Find:
            OpenSearchBar();
            break;

        case MenuItemId_Player::FindNext:
            OnFindSong(UIElements::SignalsSearchBar::SIGNAL_FIND_NEXT);
            break;

        case MenuItemId_Player::FindPrev:
            OnFindSong(UIElements::SignalsSearchBar::SIGNAL_FIND_PREV);
            break;

        case MenuItemId_Player::PlaybackMods:
            OpenPlaybackModFrame();
            break;

        case MenuItemId_Player::Preferences:
            OpenPrefsFrame();
            break;

        // Help
        case MenuItemId_Player::About:
            DisplayAboutBox();
            break;

        default:
            throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
            break;
    }
}

void FramePlayer::OnTimer(wxTimerEvent& /*evt*/)
{
    const PlaybackController& playbackInfo = _app.GetPlaybackInfo();
    PlaybackController::State cState = playbackInfo.GetState();

    const bool hiresUpdate = _ui->compositeSeekbar->IsSeekPreviewing() || cState == PlaybackController::State::Seeking || cState == PlaybackController::State::Playing;
    SetRefreshTimerThrottled(!hiresUpdate);

    if (cState != PlaybackController::State::Stopped && cState != PlaybackController::State::Undefined)
    {
        const uint_least32_t playbackTimeMs = playbackInfo.GetTime();
        if (playbackTimeMs > 0)
        {
            UpdatePeriodicDisplays(playbackTimeMs);

            // Playback (repeat) control
            const int trimMs = _app.currentSettings->GetOption(Settings::AppSettings::ID::SonglengthsTrim)->GetValueAsInt();
            if (playbackTimeMs >= _ui->compositeSeekbar->GetDurationValue() + trimMs)
            {
                OnSongDurationReached();
            }
        }
    }

    // Media Keys
    if (_app.currentSettings->GetOption(Settings::AppSettings::ID::MediaKeys)->GetValueAsBool())
    {
        switch(Helpers::Wx::Input::GetMediaKeyCommand()) // Because the RegisterHotKey is only implemented under MSW, we use this universal solution for now.
        {
            case WXK_MEDIA_PLAY_PAUSE:
            {
                if (cState != PlaybackController::State::Undefined && _ui->treePlaylist->TryGetCurrentSongTreeItemData() != nullptr)
                {
                    OnButtonPlayPause();
                }
                break;
            }
            case WXK_MEDIA_STOP:
            {
                if (cState != PlaybackController::State::Stopped && cState != PlaybackController::State::Undefined)
                {
                    OnButtonStop();
                }
                break;
            }
            case WXK_MEDIA_NEXT_TRACK:
            {
                const bool optIncludeSubsongs = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatModeIncludeSubsongs)->GetValueAsBool();
                const bool lastSubsong = _ui->treePlaylist->GetNextSubsong() == nullptr;
                if (!(optIncludeSubsongs && !lastSubsong && OnButtonSubsongNext()))
                {
                    OnButtonTuneNext();
                }
                break;
            }
            case WXK_MEDIA_PREV_TRACK:
            {
                const bool optIncludeSubsongs = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatModeIncludeSubsongs)->GetValueAsBool();
                const bool firstSubsong = _ui->treePlaylist->GetPrevSubsong() == nullptr;
                if (!(optIncludeSubsongs && !firstSubsong && OnButtonSubsongPrev()))
                {
                    OnButtonTunePrev();
                }
                break;
            }
        }
    }
}

void FramePlayer::OnIconize(wxIconizeEvent& evt)
{
    const int timerInterval = (evt.IsIconized()) ? TIMER_REFRESH_INTERVAL_IDLE : _timerCanonicalRefreshInterval;
    SetRefreshTimerInterval(timerInterval);
}

void FramePlayer::OnClose(wxCloseEvent& /*evt*/)
{
    CloseApplication();
}

void FramePlayer::OnButtonPlayPause()
{
    if (_ui->btnPlayPause->IsPlay())
    {
        switch (_app.GetPlaybackInfo().GetState())
        {
            case PlaybackController::State::Seeking:
                // Fall-through: when seeking is underway we just toggle the resume state silently.
            case PlaybackController::State::Paused:
                _app.ResumePlayback();
                break;
            default: // Usually Stopped (but could also be some other state if weird situation, app will say "error" then).
                _app.ReplayLoadedTune();
        }
    }
    else
    {
        _app.PausePlayback();
    }

    UpdateUiState();
}

void FramePlayer::OnButtonStop()
{
    _app.StopPlayback();
    UpdateUiState();
}

bool FramePlayer::OnButtonSubsongNext()
{
    const SongTreeItemData* const subsongData = _ui->treePlaylist->GetNextSubsong();
    bool success = subsongData != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*subsongData);
        }
        UpdateUiState();
    }

    return success;
}

bool FramePlayer::OnButtonSubsongPrev()
{
    const SongTreeItemData* const subsongData = _ui->treePlaylist->GetPrevSubsong();
    bool success = subsongData != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*subsongData);
        }
        UpdateUiState();
    }

    return success;
}

bool FramePlayer::OnButtonTuneNext()
{
    const SongTreeItemData* const songData = _ui->treePlaylist->GetNextSong();
    bool success = songData != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*songData);
        }
        UpdateUiState();
    }

    return success;
}

bool FramePlayer::OnButtonTunePrev()
{
    const SongTreeItemData* const songData = _ui->treePlaylist->GetPrevSong();
    bool success = songData != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*songData);
        }
        UpdateUiState();
    }

    return success;
}

void FramePlayer::OnSongDurationReached()
{
    const int ivalue = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatMode)->GetValueAsInt();
    const RepeatMode repeatMode = static_cast<RepeatMode>(ivalue);
    const bool includeSubsongs = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatModeIncludeSubsongs)->GetValueAsBool();

    switch (repeatMode)
    {
        case RepeatMode::InfiniteDuration:
        {
            // Nothing to do...
            break;
        }
        case RepeatMode::Normal:
        {
            _app.StopPlayback();

            const bool playingSubsong = includeSubsongs && TryPlayNextValidSubsong();
            if (!playingSubsong)
            {
                TryPlayNextValidSong();
            }

            break;
        }
        case RepeatMode::PlayOnce:
        {
            _app.StopPlayback();
            break;
        }
        case RepeatMode::RepeatAll:
        {
            _app.StopPlayback();

            const bool playingNextSubsong = includeSubsongs && TryPlayNextValidSubsong();
            const bool reachedTheEnd = !playingNextSubsong && !TryPlayNextValidSong();
            if (reachedTheEnd)
            {
                wxTreeItemIdValue cookie;
                const SongTreeItemData& firstTuneData = _ui->treePlaylist->GetSongTreeItemData(_ui->treePlaylist->GetBase().GetFirstChild(_ui->treePlaylist->GetRootItem(), cookie));
                if (firstTuneData.GetStatus() == SongTreeItemData::ItemStatus::Normal)
                {
                    if (!TryPlayPlaylistItem(firstTuneData))
                    {
                        TryPlayNextValidSong();
                    }
                }
            }
            break;
        }
        case RepeatMode::RepeatOne:
        {
            _app.StopPlayback();
            _app.ReplayLoadedTune();
            break;
        }
        default:
            std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
            break;
    }

    UpdateUiState();
}

void FramePlayer::OnSeekingCeased()
{
    const PlaybackController::State state = _app.GetPlaybackInfo().GetState();
    if (state != PlaybackController::State::Seeking && state != PlaybackController::State::Stopped)
    {
        UpdatePlaybackStatusBar(); // Clear the "Seeking..." status text.
        //UpdatePeriodicDisplays(_app.GetPlaybackInfo().GetTime());
        UpdateUiState();
    }
}

void FramePlayer::OnRepeatModeExtraOptionToggled(ExtraOptionId extraOptionId)
{
    switch (extraOptionId)
    {
        case ExtraOptionId::DefaultSubsong:
        {
            Settings::Option* option = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatModeDefaultSubsong);
            option->UpdateValue(!option->GetValueAsBool());
            _ui->btnRepeatMode->SetExtraOptionEnabled(ExtraOptionId::DefaultSubsong, option->GetValueAsBool());
            break;
        }
        case ExtraOptionId::IncludeSubsongs:
        {
            Settings::Option* option = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatModeIncludeSubsongs);
            option->UpdateValue(!option->GetValueAsBool());
            _ui->btnRepeatMode->SetExtraOptionEnabled(ExtraOptionId::IncludeSubsongs, option->GetValueAsBool());
            break;
        }
        default:
        {
            throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
        }
    }
}

int audioDeviceRevertCount = 0;
void FramePlayer::OnAudioDeviceChanged(bool success)
{
    ++audioDeviceRevertCount;

    UpdateUiState();
    if (!success)
    {
        if (_framePrefs != nullptr && _framePrefs->IsVisible())
        {
            _framePrefs->Destroy();
        }

        if (audioDeviceRevertCount == 1)
        {
            wxMessageBox(Strings::Error::MSG_ERR_AUDIO_CONFIG, Strings::FramePlayer::WINDOW_TITLE, wxICON_ERROR);
            _app.ReapplyPlaybackSettings();
        }
        else
        {
            const bool didReset = _app.ResetToDefaultsRecovery(Strings::Error::MSG_ERR_RESET_DEFAULTS_RECOVERY);
            if (didReset)
            {
                _app.ReapplyPlaybackSettings();
            }
            else
            {
                CloseApplication();
            }
        }
    }

    audioDeviceRevertCount = 0;
}

const FramePlayer::SongTreeItemData* const FramePlayer::DoFindSong(const wxString& query, const wxTreeItemId& startPosition, bool forwardDirection)
{
    return _ui->treePlaylist->FindSiblingIf([&](const SongTreeItemData& cSongData)
    {
        return cSongData.GetTitle().Lower().Contains(query);
    }, _ui->treePlaylist->GetRootItem(), startPosition, forwardDirection);
}

void FramePlayer::OnFindSong(UIElements::SignalsSearchBar signalId)
{
    const char* COLOR_HIT = "#D1FFC0";
    const char* COLOR_WRAPPED = "#F3FFB3";
    const char* COLOR_MISS = "#FFCCCB";

    const wxString& query = _ui->searchBar->GetQuery();
    if (!query.IsEmpty())
    {
        wxTreeItemId currentSelection = _ui->treePlaylist->GetBase().GetSelection();

        // Ensure search point is a main song and not a subsong
        {
            wxTreeItemId parentId = _ui->treePlaylist->GetBase().GetItemParent(currentSelection);
            if (parentId.GetID() != _ui->treePlaylist->GetRootItem().GetID())
            {
                currentSelection = parentId;
            }
        }

        // Find next/prev
        const bool forwardDirection = signalId != UIElements::SignalsSearchBar::SIGNAL_FIND_PREV;
        bool wrapAround = false;
        const SongTreeItemData* targetItem = DoFindSong(query, currentSelection, forwardDirection);

        if (targetItem == nullptr) // Next/prev result not found, try to wrap around
        {
            targetItem = DoFindSong(query, nullptr, forwardDirection); // Wrap around.
            wrapAround = true;
        }

        if (targetItem == nullptr)
        {
            _ui->searchBar->FlashInputBox(wxColour(COLOR_MISS));
        }
        else
        {
            _ui->treePlaylist->SelectItem(targetItem->GetId());
            _ui->searchBar->FlashInputBox((wrapAround) ? wxColour(COLOR_WRAPPED) : wxColour(COLOR_HIT));
        }
    }
}