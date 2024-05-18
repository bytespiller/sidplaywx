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
        Remove = 1,
        RemoveAllAbove,
        RemoveAllBelow,
        SkipUnskip,
        ExpandAll,
        CollapseAll,
        ScrollToCurrent
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

void FramePlayer::OnTreePlaylistItemActivated(wxDataViewEvent& evt)
{
    if (!evt.GetItem().IsOk())
    {
        return;
    }

    const PlaylistTreeModelNode* const node = PlaylistTreeModel::TreeItemToModelNode(evt.GetItem());
    TryPlayPlaylistItem(*node);

    UpdateUiState();
}

void FramePlayer::OnTreePlaylistContextMenuOpen(wxDataViewEvent& evt)
{
    if (!evt.GetItem().IsOk())
    {
        return; // No context menu item, playlist empty.
    }

    PlaylistTreeModelNode* node = PlaylistTreeModel::TreeItemToModelNode(evt.GetItem());
    wxMenu* menu = new wxMenu();

    // Skip/Unskip subsongs
    if (node->type == PlaylistTreeModelNode::ItemType::Subsong)
    {
        wxString menuTextSkipUnskip(Strings::PlaylistTree::MENU_ITEM_BLACKLIST);
        bool skipUnskipValidItem = false;

        if (node->IsPlayable())
        {
            if (node->GetTag() == PlaylistTreeModelNode::ItemTag::Normal)
            {
                skipUnskipValidItem = true;
            }
            else if (node->GetTag() == PlaylistTreeModelNode::ItemTag::Blacklisted)
            {
                skipUnskipValidItem = true;
                menuTextSkipUnskip = Strings::PlaylistTree::MENU_ITEM_UNBLACKLIST;
            }
        }

        wxMenuItem* newItem = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::SkipUnskip), menuTextSkipUnskip);
        newItem->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::SkipSubsong));
        newItem->Enable(skipUnskipValidItem);
    }

    // Remove main song
    if (node->type == PlaylistTreeModelNode::ItemType::Song)
    {
        // Remove
        {
            wxMenuItem* const item = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::Remove), Strings::PlaylistTree::MENU_ITEM_REMOVE);
            item->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::RemoveMainSong));
        }

        // Remove all above
        {
            wxMenuItem* const item = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::RemoveAllAbove), Strings::PlaylistTree::MENU_ITEM_REMOVE_ALL_ABOVE);
            item->Enable(_ui->treePlaylist->GetSongIndex(node->filepath) > 0);
            item->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::RemoveAllAbove));
        }

        // Remove all below
        {
            wxMenuItem* const item = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::RemoveAllBelow), Strings::PlaylistTree::MENU_ITEM_REMOVE_ALL_BELOW);
            item->Enable(_ui->treePlaylist->GetSongIndex(node->filepath) + 1 < _ui->treePlaylist->GetSongs().size());
            item->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::RemoveAllBelow));
        }
    }

    // Expand all, Collapse all
    {
        menu->AppendSeparator();

        wxMenuItem* newItemExpand = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::ExpandAll), Strings::PlaylistTree::MENU_ITEM_EXPAND_ALL);
        newItemExpand->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::ExpandAll));
        newItemExpand->Enable(!_ui->treePlaylist->IsEmpty());

        wxMenuItem* newItemCollapse = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::CollapseAll), Strings::PlaylistTree::MENU_ITEM_COLLAPSE_ALL);
        newItemCollapse->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::CollapseAll));
        newItemCollapse->Enable(!_ui->treePlaylist->IsEmpty());
    }

    // Scroll to current
    {
        menu->AppendSeparator();
        wxMenuItem* newItem = menu->Append(static_cast<int>(PopupMenuItemId_Playlist::ScrollToCurrent), Strings::PlaylistTree::MENU_ITEM_SCROLL_TO_CURRENT);
        newItem->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::ScrollToCurrent));
        newItem->Enable(!_ui->treePlaylist->IsEmpty());
    }

    menu->Bind(wxEVT_COMMAND_MENU_SELECTED, [&node, this](wxCommandEvent& evt) { OnTreePlaylistContextItem(*node, evt); });
    PopupMenu(menu);
}

void FramePlayer::OnTreePlaylistContextItem(PlaylistTreeModelNode& node, wxCommandEvent& evt)
{
    const PopupMenuItemId_Playlist id(static_cast<PopupMenuItemId_Playlist>(evt.GetId()));
    switch (id)
    {
        case PopupMenuItemId_Playlist::Remove:
            DoRemoveSongTreeItem(node);
            break;

        case PopupMenuItemId_Playlist::RemoveAllAbove:
            DoRemoveAllSongTreeItemsAbove(node);
            break;

        case PopupMenuItemId_Playlist::RemoveAllBelow:
            DoRemoveAllSongTreeItemsBelow(node);
            break;

        case PopupMenuItemId_Playlist::SkipUnskip:
            DoToggleSubsongBlacklistState(node);
            break;

        case PopupMenuItemId_Playlist::ExpandAll:
            _ui->treePlaylist->ExpandAll();
            break;

        case PopupMenuItemId_Playlist::CollapseAll:
            _ui->treePlaylist->CollapseAll();
            break;

        case PopupMenuItemId_Playlist::ScrollToCurrent:
            if (const PlaylistTreeModelNode* activeSong = _ui->treePlaylist->GetActiveSong())
            {
                _ui->treePlaylist->EnsureVisible(*activeSong);
                _ui->treePlaylist->Select(*activeSong);
            }
            break;
    }
}

void FramePlayer::OnTreePlaylistKeyPressed(wxKeyEvent& evt)
{
    const int keycode = evt.GetKeyCode();
    if (keycode != WXK_DELETE && keycode != WXK_NUMPAD_DELETE)
    {
        evt.Skip();
        return;
    }

    const wxDataViewItem& item = _ui->treePlaylist->GetSelection();
    if (item.IsOk())
    {
        PlaylistTreeModelNode* node = PlaylistTreeModel::TreeItemToModelNode(item);
        if (node->type == PlaylistTreeModelNode::ItemType::Song)
        {
            DoRemoveSongTreeItem(*node);
        }
        else
        {
            DoToggleSubsongBlacklistState(*node);
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
    const wxMenu* const menu = evt.GetMenu();
    if (menu == nullptr) // Bogus menu event (e.g., right click on the titlebar in MSW).
    {
        return;
    }

    const bool playlistEmpty = _ui->treePlaylist->IsEmpty();
    if (menu->GetTitle().IsSameAs(Strings::FramePlayer::MENU_FILE)) // TODO: try to find a better way to detect which menu is opened (e.g., "File" in this case).
    {
        evt.GetMenu()->Enable(static_cast<int>(MenuItemId_Player::PlaylistSave), !playlistEmpty);
        evt.GetMenu()->Enable(static_cast<int>(MenuItemId_Player::PlaylistClear), !playlistEmpty);
    }
    else if (menu->GetTitle().IsSameAs(Strings::FramePlayer::MENU_EDIT))
    {
        evt.GetMenu()->Enable(static_cast<int>(MenuItemId_Player::Find), !playlistEmpty);
        evt.GetMenu()->Enable(static_cast<int>(MenuItemId_Player::FindNext), !playlistEmpty);
        evt.GetMenu()->Enable(static_cast<int>(MenuItemId_Player::FindPrev), !playlistEmpty);
    }
}

void FramePlayer::OnMenuItemSelected(wxCommandEvent& evt)
{
    const MenuItemId_Player id = static_cast<MenuItemId_Player>(evt.GetId());
    switch (id)
    {
        // --- File ---
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

        case MenuItemId_Player::Exit:
            CloseApplication();
            break;

        // --- File > Playlist submenu ---
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
            _ui->treePlaylist->Clear();
            UpdateUiState();
            break;

        // --- Edit ---
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

        // --- View ---
        case MenuItemId_Player::StayTopmost:
            ToggleTopmost();
            break;

        case MenuItemId_Player::VisualizationEnabled:
            ToggleVisualizationEnabled();
            break;

        // --- Help ---
        case MenuItemId_Player::CheckUpdates:
            CheckUpdates();
            break;

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
            const int ivalue = _app.currentSettings->GetOption(Settings::AppSettings::ID::RepeatMode)->GetValueAsInt();
            const RepeatMode repeatMode = static_cast<RepeatMode>(ivalue);
            if (repeatMode != RepeatMode::InfiniteDuration)
            {
                const int trimMs = _app.currentSettings->GetOption(Settings::AppSettings::ID::SonglengthsTrim)->GetValueAsInt();
                if (playbackTimeMs >= _ui->compositeSeekbar->GetDurationValue() + trimMs)
                {
                    OnSongDurationReached();
                }
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
                if (cState != PlaybackController::State::Undefined && _ui->treePlaylist->GetActiveSong() != nullptr)
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
            {
                const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetActiveSong();
                const bool preRenderEnabled = _app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool();
                const int preRenderDurationMs = (preRenderEnabled && node != nullptr) ? GetEffectiveSongDuration(*node) : 0;
                _app.ReplayLoadedTune(preRenderDurationMs);
            }
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

bool FramePlayer::OnButtonSubsongNext() // TODO: much repeating in these four methods, try to DRY refactor.
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetNextSubsong();
    bool success = node != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*node);
        }
        UpdateUiState();
    }

    return success;
}

bool FramePlayer::OnButtonSubsongPrev() // TODO: much repeating in these four methods, try to DRY refactor.
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetPrevSubsong();
    bool success = node != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*node);
        }
        UpdateUiState();
    }

    return success;
}

bool FramePlayer::OnButtonTuneNext() // TODO: much repeating in these four methods, try to DRY refactor.
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetNextSong();
    bool success = node != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*node);
        }
        UpdateUiState();
    }

    return success;
}

bool FramePlayer::OnButtonTunePrev() // TODO: much repeating in these four methods, try to DRY refactor.
{
    const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetPrevSong();
    bool success = node != nullptr;

    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }
    else
    {
        if (success)
        {
            success = TryPlayPlaylistItem(*node);
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
        case RepeatMode::RepeatAll: // Don't reorder or insert new after due to fall-through.
        {
            if (_ui->treePlaylist->GetSongs().size() == 1 && _ui->treePlaylist->GetSongs().front()->GetSubsongCount() == 0)
            {
                // fall-through to RepeatOne
            }
            else
            {
                _app.StopPlayback();

                if (!_ui->treePlaylist->IsEmpty())
                {
                    const bool playingNextSubsong = includeSubsongs && TryPlayNextValidSubsong();
                    const bool reachedTheEnd = !playingNextSubsong && !TryPlayNextValidSong();
                    if (reachedTheEnd)
                    {
                        const PlaylistTreeModelNode& firstTuneNode = *_ui->treePlaylist->GetSongs().front();
                        if (firstTuneNode.GetTag() == PlaylistTreeModelNode::ItemTag::Normal)
                        {
                            if (!TryPlayPlaylistItem(firstTuneNode))
                            {
                                TryPlayNextValidSong();
                            }
                        }
                    }
                }

                break;
            }
        }
        case RepeatMode::RepeatOne: // Don't reorder or insert new before due to fall-through above.
        {
            _app.StopPlayback();

            const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetActiveSong();
            if (node != nullptr)
            {
                const int preRenderDurationMs = (_app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled)->GetValueAsBool()) ? GetEffectiveSongDuration(*node) : 0;
                _app.ReplayLoadedTune(preRenderDurationMs, true);
            }

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
        case ExtraOptionId::PreRenderEnabled:
        {
            Settings::Option* option = _app.currentSettings->GetOption(Settings::AppSettings::ID::PreRenderEnabled);
            option->UpdateValue(!option->GetValueAsBool());
            _ui->btnRepeatMode->SetExtraOptionEnabled(ExtraOptionId::PreRenderEnabled, option->GetValueAsBool());
            OnButtonStop();
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

PlaylistTreeModelNode* FramePlayer::DoFindSong(const wxString& query, const PlaylistTreeModelNode& startNode, bool forwardDirection)
{
    const PlaylistTreeModelNodePtrArray& songs = _ui->treePlaylist->GetSongs();
    auto itStart = std::find_if(songs.begin(), songs.end(), [&startNode](const PlaylistTreeModelNodePtr& qNode)
    {
        return startNode.filepath.IsSameAs(qNode->filepath);
    });

    if (itStart == songs.end())
    {
        return nullptr; // startNode not found (can happen if the playlist is empty/cleared).
    }

    const wxString queryLower = query.Lower();

    if (forwardDirection)
    {
        auto itNextResult = std::find_if(++itStart, songs.end(), [&queryLower](const PlaylistTreeModelNodePtr& qNode)
        {
            return qNode->title.Lower().Contains(queryLower);
        });

        if (itNextResult == songs.end())
        {
            return nullptr; // No further results.
        }

        return itNextResult->get();
    }
    else
    {
        auto itPrevResult = std::find_if(std::reverse_iterator(itStart), songs.rend(), [&queryLower](const PlaylistTreeModelNodePtr& qNode)
        {
            return qNode->title.Lower().Contains(queryLower);
        });

        if (itPrevResult == songs.rend())
        {
            return nullptr; // No further results.
        }

        return itPrevResult->get();
    }
}

void FramePlayer::DoRemoveSongTreeItem(PlaylistTreeModelNode& node)
{
    const PlaylistTreeModelNode* activeSong = _ui->treePlaylist->GetActiveSong();
    if (activeSong != nullptr && activeSong->filepath == node.filepath)
    {
        _app.UnloadActiveTune();
    }

    _ui->treePlaylist->Remove(node);
    PadColumnsWidth();
    UpdateUiState(); // To refresh the Next/Prev buttons.
}

void FramePlayer::DoRemoveAllSongTreeItemsAbove(PlaylistTreeModelNode& node)
{
    const PlaylistTreeModelNode* activeSong = _ui->treePlaylist->GetActiveSong();

    // Remove songs above
    {
        // Enumerate songs to remove up to the selected song
        std::vector<PlaylistTreeModelNode*> removeSongs;
        for (const PlaylistTreeModelNodePtr& song : _ui->treePlaylist->GetSongs())
        {
            if (song.get() == &node)
            {
                break;
            }

            removeSongs.emplace_back(song.get());
        }

        // Remove songs
        for (PlaylistTreeModelNode* const cSong : removeSongs)
        {
            if (activeSong != nullptr && activeSong->filepath == cSong->filepath)
            {
                _app.UnloadActiveTune();
                activeSong = nullptr;
            }

            _ui->treePlaylist->Remove(*cSong);
        }
    }

    _ui->treePlaylist->EnsureVisible(node);

    PadColumnsWidth();
    UpdateUiState(); // To refresh the Next/Prev buttons.
}

void FramePlayer::DoRemoveAllSongTreeItemsBelow(PlaylistTreeModelNode& node)
{
    const PlaylistTreeModelNode* activeSong = _ui->treePlaylist->GetActiveSong();

    // Remove songs below
    {
        // Enumerate songs to remove after the selected song
        std::vector<PlaylistTreeModelNode*> removeSongs;
        bool found = false;
        for (const PlaylistTreeModelNodePtr& song : _ui->treePlaylist->GetSongs())
        {
            if (found)
            {
                removeSongs.emplace_back(song.get());
            }
            else
            {
                found = song.get() == &node;
            }
        }

        // Remove songs
        for (PlaylistTreeModelNode* const cSong : removeSongs)
        {
            if (activeSong != nullptr && activeSong->filepath == cSong->filepath)
            {
                _app.UnloadActiveTune();
                activeSong = nullptr;
            }

            _ui->treePlaylist->Remove(*cSong);
        }
    }

    _ui->treePlaylist->EnsureVisible(node);

    PadColumnsWidth();
    UpdateUiState(); // To refresh the Next/Prev buttons.
}

void FramePlayer::DoToggleSubsongBlacklistState(PlaylistTreeModelNode& node)
{
    switch (node.GetTag())
    {
        case PlaylistTreeModelNode::ItemTag::Normal:
            _ui->treePlaylist->SetItemTag(node, PlaylistTreeModelNode::ItemTag::Blacklisted);
            break;

        case PlaylistTreeModelNode::ItemTag::Blacklisted:
            _ui->treePlaylist->SetItemTag(node, PlaylistTreeModelNode::ItemTag::Normal);
            break;
    }

    UpdateUiState(); // To refresh the Next/Prev buttons.
}

void FramePlayer::OnFindSong(UIElements::SignalsSearchBar signalId)
{
    const char* COLOR_HIT = "#D1FFC0"; // TODO: these colors should probably be defined in the theme XML instead.
    const char* COLOR_WRAPPED = "#F3FFB3";
    const char* COLOR_MISS = "#FFCCCB";

    const wxString& query = _ui->searchBar->GetQuery();
    if (query.IsEmpty())
    {
        return;
    }

    PlaylistTreeModelNode* nodeCurrent = nullptr;

    // Ensure search point is a main song and not a subsong
    {
        wxDataViewItem currentSelection = _ui->treePlaylist->GetSelection();
        if (currentSelection != nullptr && currentSelection.IsOk())
        {
            nodeCurrent = PlaylistTreeModel::TreeItemToModelNode(currentSelection);
            nodeCurrent = (nodeCurrent->type == PlaylistTreeModelNode::ItemType::Song) ? nodeCurrent : nodeCurrent->GetParent();
        }
    }

    // Find next/prev
    const bool forwardDirection = signalId != UIElements::SignalsSearchBar::SIGNAL_FIND_PREV;
    bool wrapAround = false;
    const PlaylistTreeModelNode* targetItem = (nodeCurrent == nullptr) ? nullptr : DoFindSong(query, *nodeCurrent, forwardDirection);

    if (targetItem == nullptr) // Next/prev result not found, try to wrap around
    {
        PlaylistTreeModelNode* nodeStart = (forwardDirection) ? _ui->treePlaylist->GetSongs().front().get() : _ui->treePlaylist->GetSongs().back().get();
        targetItem = DoFindSong(query, *nodeStart, forwardDirection); // Wrap around.
        wrapAround = true;
    }

    if (targetItem == nullptr)
    {
        _ui->searchBar->FlashInputBox(wxColour(COLOR_MISS));
    }
    else
    {
        _ui->treePlaylist->Select(*targetItem);
        _ui->treePlaylist->EnsureVisible(*targetItem);
        _ui->searchBar->FlashInputBox((wrapAround) ? wxColour(COLOR_WRAPPED) : wxColour(COLOR_HIT));
    }
}