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

#include <wx/ipc.h>
#include <functional>

class MyServer : public wxServer
{
public:
	class Connection : public wxConnection
	{
	public:
		using ReceptionCallback = std::function<void(const wxArrayString&)>;
		using VoidCallback = std::function<void()>;

	public:
		Connection() = delete;
		Connection(ReceptionCallback receptionCallback, VoidCallback voidCallback);

	public:
		bool OnPoke(const wxString& topic, const wxString& item, const void* data, size_t size, wxIPCFormat format) override;

	private:
		ReceptionCallback _receptionCallback;
		VoidCallback _voidCallback;
	};

public:
	MyServer() = delete;
	MyServer(Connection::ReceptionCallback receptionCallback, Connection::VoidCallback voidCallback);

public:
	wxConnectionBase* OnAcceptConnection(const wxString& topic) override;

private:
	Connection::ReceptionCallback _receptionCallback;
	Connection::VoidCallback _voidCallback;
};
