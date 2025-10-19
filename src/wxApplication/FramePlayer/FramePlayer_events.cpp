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
#include "../FrameChildren/FramePrefs/FramePrefs.h"
#include "../Helpers/HelpersWx.h"

#ifndef WIN32
#include <wx/tooltip.h>
#endif

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
        ScrollToCurrent,
        BrowseLocation
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
    const bool success = TryPlayPlaylistItem(*node);

    if (!success && !node->IsPlayable())
    {
        wxString romName;

        switch (node->romRequirement)
        {
            case PlaylistTreeModelNode::RomRequirement::None:
                break;
            case PlaylistTreeModelNode::RomRequirement::BasicRom:
                romName = (_app.GetPlaybackInfo().IsRomLoaded(SidDecoder::RomRequirement::BasicRom)) ? "" : "BASIC";
                break;
            case PlaylistTreeModelNode::RomRequirement::R64:
                romName = (_app.GetPlaybackInfo().IsRomLoaded(SidDecoder::RomRequirement::R64)) ? "" : "KERNAL";
                break;
            default:
                romName = "UNKNOWN (CHARGEN?)";
                break;
        }

        if (!romName.IsEmpty())
        {
            wxMessageBox(wxString::Format(Strings::Error::MSG_ERR_ROM_FILE_REQUIRED, romName, node->filepath), Strings::FramePlayer::WINDOW_TITLE, wxICON_INFORMATION);
        }
    }

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

        wxMenuItem* newItem = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::SkipUnskip), menuTextSkipUnskip);
        newItem->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::SkipSubsong));
        menu->Append(newItem);
        newItem->Enable(skipUnskipValidItem);
    }

    // Remove main song
    if (node->type == PlaylistTreeModelNode::ItemType::Song)
    {
        // Remove
        {
            wxMenuItem* const item = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::Remove), Strings::PlaylistTree::MENU_ITEM_REMOVE);
            item->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::RemoveMainSong));
            menu->Append(item);
        }

        // Remove all above
        {
            wxMenuItem* const item = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::RemoveAllAbove), Strings::PlaylistTree::MENU_ITEM_REMOVE_ALL_ABOVE);
            item->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::RemoveAllAbove));
            menu->Append(item);
            item->Enable(_ui->treePlaylist->GetSongIndex(node->filepath) > 0);
        }

        // Remove all below
        {
            wxMenuItem* const item = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::RemoveAllBelow), Strings::PlaylistTree::MENU_ITEM_REMOVE_ALL_BELOW);
            item->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::RemoveAllBelow));
            menu->Append(item);
            item->Enable(_ui->treePlaylist->GetSongIndex(node->filepath) + 1 < _ui->treePlaylist->GetSongs().size());
        }
    }

    // Expand all, Collapse all
    {
        menu->AppendSeparator();

        const bool relevant = std::any_of(_ui->treePlaylist->GetSongs().cbegin(), _ui->treePlaylist->GetSongs().cend(), [](const PlaylistTreeModelNodePtr& qSong) { return qSong->GetSubsongCount() > 0; });

        // Expand all
        {
            wxMenuItem* newItemExpand = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::ExpandAll), Strings::PlaylistTree::MENU_ITEM_EXPAND_ALL);
            newItemExpand->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::ExpandAll));
            menu->Append(newItemExpand);
            newItemExpand->Enable(relevant);
        }

        // Collapse all
        {
            wxMenuItem* newItemCollapse = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::CollapseAll), Strings::PlaylistTree::MENU_ITEM_COLLAPSE_ALL);
            newItemCollapse->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::CollapseAll));
            menu->Append(newItemCollapse);
            newItemCollapse->Enable(relevant);
        }
    }

    // Scroll to current
    {
        menu->AppendSeparator();
        wxMenuItem* newItem = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::ScrollToCurrent), Strings::PlaylistTree::MENU_ITEM_SCROLL_TO_CURRENT);
        newItem->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::ScrollToCurrent));
        menu->Append(newItem);
        newItem->Enable(!_ui->treePlaylist->IsEmpty());
    }

    // Browse location
    {
        menu->AppendSeparator();
        wxMenuItem* newItem = new wxMenuItem(menu, static_cast<int>(PopupMenuItemId_Playlist::BrowseLocation), Strings::Common::ACTION_BROWSE_LOCATION);
        newItem->SetBitmap(*_ui->playlistContextMenuIcons.at(FrameElements::PlaylistContextMenuIconId::BrowseLocation));
        menu->Append(newItem);
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

        case PopupMenuItemId_Playlist::BrowseLocation:
			const wxString& dir = wxPathOnly(node.filepath);
			if (!dir.IsEmpty() && !wxLaunchDefaultApplication(dir))
			{
				wxMessageBox(dir, Strings::Common::GENERIC_NOT_FOUND, wxICON_WARNING);
			}
            break;
    }
}

void FramePlayer::OnTreePlaylistKeyPressed(wxKeyEvent& evt)
{
    // Handle the DELETE key
    const int keycode = evt.GetKeyCode();
    if (keycode == WXK_DELETE || keycode == WXK_NUMPAD_DELETE)
    {
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

        return;
    }

    // Skip the event processing otherwise
#ifdef __WXGTK__
    // Prevent the GTK hijacking the CTRL+F
    if (evt.ControlDown())
    {
        _ui->btnRepeatMode->SetFocus(); // A hack to cause the player window's menu key combos to be evaluated. This button is never disabled, so it's safe to focus it for this purpose.
    }

    // Disable the GTK's native type-ahead feature (doesn't work in the wxDataViewCtrl)
    evt.Skip(false);
#else
    evt.Skip();
#endif
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
    wxMenu* const menu = evt.GetMenu();
    if (menu == nullptr) // Bogus menu event (e.g., right click on the titlebar in MSW).
    {
        return;
    }

    const bool playlistEmpty = _ui->treePlaylist->IsEmpty();
    if (menu->GetTitle().IsSameAs(Strings::FramePlayer::MENU_FILE)) // TODO: try to find a better way to detect which menu is opened (e.g., "File" in this case).
    {
        menu->Enable(static_cast<int>(MenuItemId_Player::PlaylistSave), !playlistEmpty);
        menu->Enable(static_cast<int>(MenuItemId_Player::PlaylistClear), !playlistEmpty);
    }
    else if (menu->GetTitle().IsSameAs(Strings::FramePlayer::MENU_EDIT))
    {
        menu->Enable(static_cast<int>(MenuItemId_Player::Find), !playlistEmpty);
        menu->Enable(static_cast<int>(MenuItemId_Player::FindNext), !playlistEmpty);
        menu->Enable(static_cast<int>(MenuItemId_Player::FindPrev), !playlistEmpty);
    }
    else if (menu->GetTitle().IsSameAs(Strings::FramePlayer::MENU_VIEW))
    {
        menu->Enable(static_cast<int>(MenuItemId_Player::TuneInfo), _app.GetPlaybackInfo().IsValidSongLoaded());
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

        case MenuItemId_Player::PlaylistResetDemo:
            [[fallthrough]];
        case MenuItemId_Player::PlaylistClear:
            OnButtonStop();
            _enqueuedFiles.Clear(); // Clear any pending files.
            _addingFilesToPlaylist = false; // Break the loop.
            _ui->treePlaylist->Clear();
            UpdateUiState();

            if (id == MenuItemId_Player::PlaylistResetDemo)
            {
                const wxArrayString& rawPaths = Helpers::Wx::Files::LoadPathsFromPlaylist(Helpers::Wx::Files::DEFAULT_PLAYLIST_NAME);
                DiscoverFilesAndSendToPlaylist(rawPaths, true, false);
            }

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
#ifdef WIN32
        case MenuItemId_Player::StayTopmost:
            ToggleTopmost();
            break;
#endif
        case MenuItemId_Player::VisualizationEnabled:
            ToggleVisualizationEnabled();
            break;

        case MenuItemId_Player::StilInfoEnabled:
            ToggleStilInfoEnabled();
            break;

        case MenuItemId_Player::TuneInfo:
            ShowTuneInfo();
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

void FramePlayer::OnGlobalHotkey(wxKeyEvent& evt)
{
    PlaybackController::State cState = _app.GetPlaybackInfo().GetState();
    switch(evt.GetKeyCode()) // Because the RegisterHotKey is only implemented under MSW, we use this universal solution for now.
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

void FramePlayer::OnTimerRefresh(wxTimerEvent& /*evt*/)
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
}

void FramePlayer::OnIconize(wxIconizeEvent& evt)
{
    const int timerInterval = (evt.IsIconized()) ? TIMER_REFRESH_INTERVAL_IDLE : _canonicalTimerRefreshInterval;
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
                [[fallthrough]]; // When seeking is underway we just toggle the resume state silently.
            case PlaybackController::State::Paused:
                _app.ResumePlayback();
                break;
            default: // Usually Stopped (but could also be some other state if weird situation, app will say "error" then).
            {
                const PlaylistTreeModelNode* const node = _ui->treePlaylist->GetActiveSong();
                if (node != nullptr)
                {
                    TryPlayPlaylistItem(*node);
                }
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

#pragma region Track navigation

bool FramePlayer::OnButtonSubsongNext()
{
    return DoChangePlaylistTrack(_ui->treePlaylist->GetNextSubsong());
}

bool FramePlayer::OnButtonSubsongPrev()
{
    return DoChangePlaylistTrack(_ui->treePlaylist->GetPrevSubsong());
}

bool FramePlayer::OnButtonTuneNext()
{
    return DoChangePlaylistTrack(_ui->treePlaylist->GetNextSong());
}

bool FramePlayer::OnButtonTunePrev()
{
    return DoChangePlaylistTrack(_ui->treePlaylist->GetPrevSong());
}

bool FramePlayer::DoChangePlaylistTrack(const PlaylistTreeModelNode* const targetNode)
{
    const bool shouldAutoplay = ShouldAutoPlay(_app); // Must check *before* it changes.
    bool success = targetNode != nullptr;

    if (success)
    {
        success = TryPlayPlaylistItem(*targetNode);
    }

    if (!shouldAutoplay || !success)
    {
        OnButtonStop();
    }

    UpdateUiState();

    return success;
}

#pragma endregion

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

void FramePlayer::OnAudioDeviceChanged(bool success)
{
    UpdateUiState();

    if (!success)
    {
        _app.CallAfter([&]() // Call this next frame because the Prefs window is still not done in the current frame and would cause all kinds of problems.
        {
            const bool didReset = _app.ResetToDefaultsRecovery(Strings::Error::MSG_ERR_RESET_DEFAULTS_RECOVERY);
            if (didReset) // User selected "Yes"
            {
                _app.ReapplyPlaybackSettings();
            }
            else // User selected "No"
            {
                CloseApplication();
            }
        });
    }
}

PlaylistTreeModelNode* FramePlayer::DoFindSong(const wxString& query, const PlaylistTreeModelNode& startNode, bool forwardDirection, bool restart)
{
    const PlaylistTreeModelNodePtrArray& songs = _ui->treePlaylist->GetSongs();
    auto itStart = std::find_if(songs.cbegin(), songs.cend(), [&startNode](const PlaylistTreeModelNodePtr& qNode)
    {
        return startNode.filepath.IsSameAs(qNode->filepath);
    });

    if (itStart == songs.cend())
    {
        return nullptr; // startNode not found (can happen if the playlist is empty/cleared).
    }

    const wxString queryLower = query.Lower();

    if (forwardDirection)
    {
        const auto itNextResult = std::find_if(itStart + ((restart) ? 0 : 1), songs.cend(), [&queryLower](const PlaylistTreeModelNodePtr& qNode)
        {
            return qNode->title.Lower().Contains(queryLower);
        });

        if (itNextResult == songs.cend())
        {
            return nullptr; // No further results.
        }

        return itNextResult->get();
    }
    else
    {
        const auto itPrevResult = std::find_if(std::reverse_iterator(itStart) - ((restart) ? 1 : 0), songs.rend(), [&queryLower](const PlaylistTreeModelNodePtr& qNode)
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
    const PlaylistTreeModelNode* targetItem = (nodeCurrent == nullptr) ? nullptr : DoFindSong(query, *nodeCurrent, forwardDirection, false);

    if (targetItem == nullptr) // Next/prev result not found, try to wrap around
    {
        const PlaylistTreeModelNode* const nodeStart = (forwardDirection) ? _ui->treePlaylist->GetSongs().front().get() : _ui->treePlaylist->GetSongs().back().get();
        targetItem = DoFindSong(query, *nodeStart, forwardDirection, true); // Wrap around.
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