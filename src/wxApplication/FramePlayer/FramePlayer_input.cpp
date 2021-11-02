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
#include "../Config/UIStrings.h"
#include <wx/filedlg.h>

static const wxString WILDCARD_SID = "*.sid;*.c64;*.prg;*.p00;*.str;*.mus";
static const wxString WILDCARD_ZIP = "*.zip";
static const wxString WILDCARD_ALL = "*.*";

void FramePlayer::BrowseFilesAndAddToPlaylist(bool enqueue)
{
    wxFileDialog openFileDialog(this);
    openFileDialog.SetWindowStyle(wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
    openFileDialog.SetWildcard(wxString::Format("%s (%s;%s)|%s;%s", Strings::FramePlayer::BROWSE_FILES_SUPPORTED, WILDCARD_SID, WILDCARD_ZIP, WILDCARD_SID, WILDCARD_ZIP) +\
                               wxString::Format("|%s (%s)|%s", Strings::FramePlayer::BROWSE_FILES_SID, WILDCARD_SID, WILDCARD_SID) +\
                               wxString::Format("|%s (%s)|%s", Strings::FramePlayer::BROWSE_FILES_ZIP, WILDCARD_ZIP, WILDCARD_ZIP) +\
                               wxString::Format("|%s (%s)|%s", Strings::FramePlayer::BROWSE_FILES_ALL, WILDCARD_ALL, WILDCARD_ALL)
                               );

    if (openFileDialog.ShowModal() == wxID_CANCEL)
    {
        return;
    }

    wxArrayString rawPaths;
    openFileDialog.GetPaths(rawPaths);
    DiscoverFilesAndSendToPlaylist(rawPaths, !enqueue, !enqueue);
}

void FramePlayer::BrowseFoldersAndAddToPlaylist(bool enqueue)
{
    wxDirDialog openDirDialog(this);
    openDirDialog.SetWindowStyle(wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST | wxDD_MULTIPLE);
    if (openDirDialog.ShowModal() == wxID_CANCEL)
    {
        return;
    }

    wxArrayString rawPaths;
    openDirDialog.GetPaths(rawPaths);
    DiscoverFilesAndSendToPlaylist(rawPaths, !enqueue, !enqueue);
}
