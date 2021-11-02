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

#include <wx/snglinst.h>
#include <memory>
#include <map>

#include "MyClient.h"
#include "MyServer.h"

class SingleInstanceManager
{
public:
    static constexpr const int FILE_LIST_RECEPTION_CONTINUITY_PERIOD = 800; // How long to wait (ms) to see if more files are incoming (delay needed for sorting, all audio players on MSW (e.g., Foobar2k) do this with similar delay).
    static constexpr const int IPC_BUSY_WAIT_TICK = 1; // ms
    static constexpr const int IPC_BUSY_TIMEOUT = 5000; // 5000 is system default

    using FileListIncomingNotifyCallback = std::function<void()>;
    using FileListReceiverCallback = std::function<void(wxMilliClock_t, const wxArrayString&)>;

public:
    SingleInstanceManager();

public:
    bool TryLock();
    void RegisterFileListIncomingNotifyCallback(FileListIncomingNotifyCallback callback);
    void RegisterFileListReceiver(FileListReceiverCallback callback);

    bool TryWaitCanonicalInstanceReady();
    bool BringCanonicalInstanceToForeground();
    bool SendFilesToCanonicalInstance(const wxString& rawPathsWithMillisSuffix);

private:
    void OnReceiveFiles(const wxArrayString& rawPathsWithMillisSuffix);
    void OnBringToForeground();

    bool TryPoke(const wxString& item, const wxString& param);

private:
    class FileListHandler : public wxTimer
    {
    public:
        FileListHandler() = delete;
        explicit FileListHandler(FileListReceiverCallback callback);

    public:
        void Add(wxArrayString rawPathsWithMillisSuffix);

    private:
        void Notify() override;

    private:
        FileListReceiverCallback _fileListReceiver;
        std::map<wxMilliClock_t, wxArrayString> _sortedFileListBatches;
        wxMilliClock_t _fileListReceptionTime = 0;
    };

private:
    std::unique_ptr<wxSingleInstanceChecker> _instanceHolder;
    std::unique_ptr<MyClient> _ipcClient;
    std::unique_ptr<MyServer> _ipcServer;
    std::unique_ptr<FileListHandler> _fileListHandler;
    FileListIncomingNotifyCallback _fileListIncomingNotifyCallback;
};
