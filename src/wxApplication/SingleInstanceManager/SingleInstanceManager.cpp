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

#include "SingleInstanceManager.h"
#include "IpcSetup.h"

// FileListHandler inner private class ------------------------
SingleInstanceManager::FileListHandler::FileListHandler(FileListReceiverCallback callback) :
	_fileListReceiver(callback)
{
}

void SingleInstanceManager::FileListHandler::Add(wxArrayString rawPathsWithMillisSuffix)
{
	_fileListReceptionTime = wxGetLocalTimeMillis();

	const wxMilliClock_t timestamp = wxStrtoll(rawPathsWithMillisSuffix.Last(), nullptr, 10);
	if (timestamp > 0)
	{
		rawPathsWithMillisSuffix.Remove(rawPathsWithMillisSuffix.Last());
	}

	_sortedFileListBatches.emplace(timestamp, rawPathsWithMillisSuffix);

	StartOnce(FILE_LIST_RECEPTION_CONTINUITY_PERIOD); // Re/Start the timer.
}

void SingleInstanceManager::FileListHandler::Notify() // On timer expired.
{
	wxArrayString fileList;
	for (const auto& batchFileList : _sortedFileListBatches)
	{
		fileList.reserve(batchFileList.second.size());
		std::copy(batchFileList.second.begin(), batchFileList.second.end(), std::back_inserter(fileList));
	}

	_sortedFileListBatches.clear();

	_fileListReceiver(_fileListReceptionTime, fileList);
}

// SingleInstanceManager --------------------------------------

SingleInstanceManager::SingleInstanceManager() :
	_instanceHolder(std::make_unique<wxSingleInstanceChecker>()),
	_ipcClient(std::make_unique<MyClient>())
{
}

bool SingleInstanceManager::TryLock()
{
	const bool canonicalInstanceExists = _instanceHolder->IsAnotherRunning() && TryWaitCanonicalInstanceReady();
	if (canonicalInstanceExists)
	{
		return false;
	}

	_ipcServer = std::make_unique<MyServer>([this](const wxArrayString& rawPathsWithMillisSuffix){OnReceiveFiles(rawPathsWithMillisSuffix);}, [this](){OnBringToForeground();});
	return _ipcServer != nullptr && _ipcServer->Create(IpcSetup::SERVICE);
}

void SingleInstanceManager::RegisterFileListIncomingNotifyCallback(FileListIncomingNotifyCallback callback)
{
	_fileListIncomingNotifyCallback = callback;
}

void SingleInstanceManager::RegisterFileListReceiver(FileListReceiverCallback callback)
{
	_fileListHandler = std::make_unique<FileListHandler>(callback);
}

bool SingleInstanceManager::TryWaitCanonicalInstanceReady()
{
	return TryPoke(IpcSetup::IpcItem::PingServer, "");
}

bool SingleInstanceManager::BringCanonicalInstanceToForeground()
{
	return TryPoke(IpcSetup::IpcItem::BringToForeground, "");
}

bool SingleInstanceManager::SendFilesToCanonicalInstance(const wxString& rawPathsWithMillisSuffix)
{
	return TryPoke(IpcSetup::IpcItem::Files, rawPathsWithMillisSuffix);
}

void SingleInstanceManager::OnReceiveFiles(const wxArrayString& rawPathsWithMillisSuffix)
{
	assert(!rawPathsWithMillisSuffix.IsEmpty());
	if (!rawPathsWithMillisSuffix.IsEmpty())
	{
		_fileListIncomingNotifyCallback();
		_fileListHandler->Add(rawPathsWithMillisSuffix);
	}
}

void SingleInstanceManager::OnBringToForeground()
{
	if (wxTopLevelWindow* wnd = dynamic_cast<wxTopLevelWindow*>(wxTheApp->GetTopWindow()))
	{
		if (wnd->IsIconized())
		{
			wnd->Iconize(false);
		}
		wnd->Raise();
	}
}

bool SingleInstanceManager::TryPoke(const wxString& item, const wxString& param)
{
	wxLogNull shutup; // Popup-errors suppressed until we exit this method.

	int timeWaited = 0;
	bool success = false;

	while (true)
	{
		if (!_ipcClient->IsConnected())
		{
			_ipcClient->TryConnect(IpcSetup::HOST_NAME, IpcSetup::SERVICE, IpcSetup::TOPIC);
		}

		if (_ipcClient->IsConnected())
		{
			success = _ipcClient->GetConnection()->Poke(item, param);
			if (!success)
			{
				_ipcClient->Disconnect();
				//wxMessageBox(wxString::Format("Poke failed: %s, %s", item, param), wxString::Format("sidplaywx - %lu", wxGetProcessId()), wxICON_ERROR);
			}
		}

		timeWaited += IPC_BUSY_WAIT_TICK;
		if (!success && timeWaited < IPC_BUSY_TIMEOUT)
		{
			wxMilliSleep(IPC_BUSY_WAIT_TICK);
		}
		else
		{
			break;
		}
	}

	return success;
}
