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

#include "../../Util/BufferHolder.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/fs_zip.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

#include <memory>
#include <string>

namespace Helpers
{
	namespace Wx
	{
		wxString GetTimeFormattedString(uint_least32_t millis, bool zeroDurationSpecial = false);

		namespace Files
		{
			static const std::string FILE_EXTENSION_ZIP = ".zip";

			std::wstring AsAbsolutePathIfPossible(const std::wstring& relPath);
			std::wstring AsRelativePathIfPossible(const std::wstring& absPath);
			wxArrayString GetValidFiles(const wxArrayString& rawFileList);

			inline bool IsZipFile(const wxString& filename)
			{
				return filename.ends_with(FILE_EXTENSION_ZIP);
			}
			inline bool IsWithinZipFile(const wxString& filename)
			{
				return filename.Contains(FILE_EXTENSION_ZIP + "\\") || filename.Contains(FILE_EXTENSION_ZIP + "/");
			}

			std::pair<wxString, wxString> SplitZipArchiveAndFileNames(const wxString& filename);
			std::unique_ptr<BufferHolder> GetFileContentFromZip(const wxString& filename);

			// Like GetFileContentFromZip but for regular files, supporting unicode paths (can't just naively load them directly via libsidplayfp's loader unfortunately due to lack of unicode paths support there).
			std::unique_ptr<BufferHolder> GetFileContentFromDisk(const wxString& filename);
		}

		namespace Audio
		{
			enum class DeviceType
			{
				Any,
				Output
			};

			enum class Backend
			{
				All,
				Filtered
			};

			wxArrayString GetAudioDevicesNames(DeviceType type, Backend backend);
			wxString GetDefaultAudioOutDeviceName();
			int GetSelectedOrDefaultAudioDeviceIndex(const wxString& selectedDeviceName, DeviceType type, Backend backend);
		}

		namespace Input
		{
			wxKeyCode GetMediaKeyCommand();
		}
	}
}
