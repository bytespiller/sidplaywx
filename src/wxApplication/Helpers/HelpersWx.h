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

#pragma once

#include "../../Util/BufferHolder.h"
#include "../../Util/Const.h"
#include "../../Util/HelpersGeneral.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/filename.h>
#include <wx/stdpaths.h>

#include <wx/fs_zip.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

#include <memory>
#include <string>

namespace Helpers
{
	namespace Wx
	{
		/// @brief Several times faster than using the wxString::Format or naive concatenation.
		inline const wxString FastJoin(const wxString& base, const char* const a, const char* const b)
		{
			wxString x(base);
			x += a;
			x += b;
			return x;
		}

		/// @brief Several times faster than using the wxString::Format or naive concatenation.
		inline const wxString FastJoin(const wxString& base, const char* const a, const char* const b, const char* const c)
		{
			wxString x(base);
			x += a;
			x += b;
			x += c;
			return x;
		}

		/// @brief Extracts a value from the string between delimiters and deposits it in the outResult returning true if successful. Otherwise just returns false.
		inline bool ExtractValue(const wxString& source, const wxString& start, const wxString& end, wxString& outResult)
		{
			const int indexStart = source.Find(start);
			if (indexStart != -1)
			{
				const size_t indexEnd = source.find(end, indexStart + start.length());
				if (indexEnd != wxString::npos)
				{
					outResult = source.substr(indexStart + start.length(), indexEnd - (indexStart + start.length()));
					return true;
				}
			}

			return false;
		}

		inline const wxString GetTimeFormattedString(uint_least32_t millis, bool zeroDurationSpecial = false)
		{
			if (zeroDurationSpecial && millis == 0)
			{
				return "??:??";
			}

			const long min = millis / Const::MILLISECONDS_IN_MINUTE;

			millis -= Const::MILLISECONDS_IN_MINUTE * min;
			const long sec = millis / Const::MILLISECONDS_IN_SECOND;

			return FastJoin(Helpers::General::GetZeroPaddedString(min), ":", Helpers::General::GetZeroPaddedString(sec).c_str());
		}

		/// @brief Songlengths.md5, STIL.txt and .sid files use the Windows1252 encoding for strings.
		wxString StringFromWin1252(const std::string_view& input);

		namespace Files
		{
			static const std::string FILE_EXTENSION_ZIP = ".zip";
			static const std::string FILE_EXTENSION_PLAYLIST = ".m3u8";
			static const std::string DEFAULT_PLAYLIST_NAME = "default" + FILE_EXTENSION_PLAYLIST;

			wxString AsAbsolutePathIfPossible(const wxString& relPath);
			wxString AsRelativePathIfPossible(const wxString& absPath);
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

			/// @brief Like GetFileContentFromZip but for regular files, supporting unicode paths (can't just naively load them directly via libsidplayfp's loader unfortunately due to lack of unicode paths support there).
			std::unique_ptr<BufferHolder> GetFileContentFromDisk(const wxString& filename);

			bool TrySavePlaylist(wxString fullpath, const std::vector<wxString>& fileList);
			wxArrayString LoadPathsFromPlaylist(const wxString& fullpath);

			/// @brief Returns a canonical destination path for sidplaywx config files.
			inline wxString GetConfigFilePath(const wxString& filename)
			{
#ifdef WIN32
				return filename; // Simply save to the app folder (portable). If changing this in the future, don't forget to create the target dir(s) in the downstream (like Linux code).
#elif __WXGTK__
				wxFileName path(wxStandardPaths::Get().GetUserConfigDir() + wxFILE_SEP_PATH);

				if (path.GetDirs().IsEmpty() || path.GetDirs().Last() != ".config")
				{
					path.AppendDir(".config");
				}

				path.AppendDir("sidplaywx");
				path.SetFullName(filename);

				return path.GetFullPath();
#else
				#error "Please define a config file path logic for this platform." // Possibly OSX (~/Library/Application Support/sidplaywx/$filename)?
#endif
			}
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

			/// @brief Converts absolute output audio device index to filtered index (supported output audio devices).
			/// @return Filtered index or wxNOT_FOUND (-1).
			int TryGetFilteredFromAbsoluteAudioDeviceIndex(int absoluteIndex);

			/// @brief Gets audio device index from the device name if it exists and if it's supported by this version of the app.
			/// @return Absolute index of the specified audio device, or of a default device or paNoDevice if nothing is available.
			int TryGetAudioDeviceIndex(const wxString& deviceName);
		}
	}
}
