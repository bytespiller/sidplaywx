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

#include "MyClient.h"

bool MyClient::TryConnect(const wxString& host, const wxString& service, const wxString& topic)
{
	Disconnect();

    // suppress the log messages from MakeConnection()
    wxLogNull nolog;

    _connection = std::unique_ptr<wxConnection>(static_cast<wxConnection*>(MakeConnection(host, service, topic)));
    return _connection != nullptr;
}

bool MyClient::IsConnected() const
{
	return _connection != nullptr;
}

wxConnection* MyClient::GetConnection()
{
	return _connection.get();
}

void MyClient::Disconnect()
{
	_connection = nullptr; // wxConnection's destructor also calls its own Disconnect();
}
