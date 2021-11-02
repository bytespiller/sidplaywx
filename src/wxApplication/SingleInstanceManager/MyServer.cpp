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

#include "MyServer.h"
#include "IpcSetup.h"
#include "../../Util/HelpersGeneral.h"

// MyServer::Connection ---------------------------------------
MyServer::Connection::Connection(ReceptionCallback receptionCallback, VoidCallback voidCallback) :
	_receptionCallback(receptionCallback),
	_voidCallback(voidCallback)
{
}

bool MyServer::Connection::OnPoke(const wxString& topic, const wxString& item, const void* data, size_t size, wxIPCFormat /*format*/)
{
	if (topic != IpcSetup::TOPIC)
	{
		return false;
	}

	if (item == IpcSetup::IpcItem::BringToForeground)
	{
		_voidCallback();
		return true;
	}
	else if (item == IpcSetup::IpcItem::Files)
	{
		assert(size > 0);
		if (size == 0)
		{
			return false;
		}

		wxArrayString arr;

		{
			const wxString rawPathsStr = wxString::FromUTF8(static_cast<const char*>(data));
			const std::vector<wxString>& rawPathsVec = Helpers::General::SplitString(rawPathsStr, IpcSetup::INTERNAL_FILES_SEPARATOR);

			arr.reserve(rawPathsVec.size());

			for (const wxString& str : rawPathsVec)
			{
				arr.Add(str);
			}
		}

		_receptionCallback(arr);

		return true;
	}
	else if (item == IpcSetup::IpcItem::PingServer)
	{
		return true;
	}

	return false;
}

// MyServer ---------------------------------------------------
MyServer::MyServer(Connection::ReceptionCallback receptionCallback, Connection::VoidCallback voidCallback) :
	_receptionCallback(receptionCallback),
	_voidCallback(voidCallback)
{
}

wxConnectionBase* MyServer::OnAcceptConnection(const wxString& topic)
{
	if (topic != IpcSetup::TOPIC)
	{
		return nullptr;
	}

	return new Connection(_receptionCallback, _voidCallback);
}
