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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

namespace IpcSetup
{
    static const wxString TOPIC = "sidplaywx";
    static constexpr const char INTERNAL_FILES_SEPARATOR = '^';

    namespace IpcItem
    {
        static const wxString Files = "Files";
        static const wxString BringToForeground = "Raise";
        static const wxString PingServer = "Ping";
    }

	static const wxString HOST_NAME = "localhost"; // From docs: machine name under UNIX - use 'localhost' for same machine; ignored when using native DDE in Windows.

#ifdef _WIN32
    static const wxString SERVICE = wxString::Format("%s_ipc", TOPIC);
#else
    static const wxString SERVICE = wxString::Format("/tmp/%s_ipc", TOPIC); // From docs: Under Unix, the service name may be either an integer port identifier in which case an Internet domain socket will be used for the communications, or a valid file name (which shouldn't exist and will be deleted afterwards) in which case a Unix domain socket is created.
#endif
}
