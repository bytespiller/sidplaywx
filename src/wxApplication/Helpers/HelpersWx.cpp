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

#include "HelpersWx.h"

#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>

#include <cstring>
#include <filesystem>
#include <iconv.h>
#include <portaudio.h>

namespace
{
	inline wxArrayString GetFilesInZip(const wxString& path)
	{
		wxArrayString flatfileList;

		wxFileSystem fs;
		fs.ChangePathTo(path); // Prevent OpenFile from trying relative scope first (always in vain). This yields huuuge speed boost.
		wxString filenameOnly = path;
		filenameOnly.Replace("\\", "/");
		if (!fs.GetPath().IsEmpty())
		{
			filenameOnly.Replace(fs.GetPath(), "");
		}

		wxFSFile* fsFile = fs.OpenFile(filenameOnly, wxFS_READ);
		if (fsFile != NULL)
		{
			wxZipInputStream in(fsFile->GetStream());

			if (in.GetTotalEntries() > 0)
			{
				wxZipEntry* entry = in.GetNextEntry();
				while (entry != nullptr)
				{
					if (!entry->IsDir())
					{
						flatfileList.push_back(wxString::Format("%s/%s", path, entry->GetName()));
					}

					delete entry;
					entry = in.GetNextEntry();
				}

				in.CloseEntry();
			}
		}

		return flatfileList;
	}

	inline wxKeyCode GetMediaKeyDown()
	{
		static std::vector<wxKeyCode> mediaKeys {WXK_MEDIA_STOP, WXK_MEDIA_PLAY_PAUSE, WXK_MEDIA_NEXT_TRACK, WXK_MEDIA_PREV_TRACK};
		wxKeyCode activeMediaKey = WXK_NONE;

		for (const wxKeyCode key : mediaKeys)
		{
			if (wxGetKeyState(key))
			{
				activeMediaKey = key;
				break;
			}
		}

		return activeMediaKey;
	}
}

namespace Helpers
{
	namespace Wx
	{
		wxString StringFromWin1252(const std::string_view& input)
		{
			// Open a conversion descriptor from "WINDOWS-1252" to "UTF-8" and wrap it in a unique_ptr with custom deleter (for RAII behavior)
			std::unique_ptr<std::remove_pointer_t<iconv_t>, decltype(&iconv_close)> conv(iconv_open("UTF-8", "WINDOWS-1252"), iconv_close);
			if (conv.get() == reinterpret_cast<iconv_t>(-1))
			{
				throw std::runtime_error("iconv_open failed: " + std::string(std::strerror(errno)));
			}

			// Set how many input bytes we have
			size_t inBytesLeft = input.size();

			constexpr size_t expansionFactor = 4; // Each input byte may expand up to 4 bytes.

			// Calculate the output buffer size
			size_t outBytesLeft = inBytesLeft * expansionFactor;

			// Pre-allocate a (null filled) string to hold the UTF-8 output
			std::string output(outBytesLeft, '\0');

			char* inbuf = const_cast<char*>(input.data()); // iconv() requires non-const pointers.
			char* outbuf = output.data(); // Beginning of the output buffer.

			// Attempt the conversion
			if (auto result = iconv(conv.get(), &inbuf, &inBytesLeft, &outbuf, &outBytesLeft);
				result == (size_t)-1)
			{
				throw std::runtime_error("iconv conversion error: " + std::string(std::strerror(errno)));
			}

			// Resize 'output' to the actual number of bytes written.
			// 'outBytesLeft' now contains the unused space in the buffer.
			output.resize(output.size() - outBytesLeft);

			return wxString::FromUTF8(output);
		}

		namespace Files
		{
			wxString AsAbsolutePathIfPossible(const wxString& relPath)
			{
				if (relPath.empty())
				{
					return relPath;
				}

				wxString absPath(std::filesystem::weakly_canonical(relPath.ToStdWstring()));
				return (absPath.empty()) ? relPath : absPath; // If it doesn't exist, return the original path unchanged.
			}

			wxString AsRelativePathIfPossible(const wxString& absPath)
			{
				if (absPath.empty())
				{
					return absPath;
				}

				wxString relPath(std::filesystem::relative(absPath.ToStdWstring()));
				return (relPath.empty()) ? absPath : relPath; // If it doesn't exist, return the original path unchanged.
			}

			wxArrayString GetValidFiles(const wxArrayString& rawFileList)
			{
				const wxString& selfExecutablePath = wxStandardPaths::Get().GetExecutablePath();

				wxArrayString filesChecked;
				for (const wxString& fileOrFolder : rawFileList)
				{
					if (wxFileExists(fileOrFolder))
					{
						if (IsZipFile(fileOrFolder))
						{
							const wxArrayString& result = GetFilesInZip(fileOrFolder); // Files are obtained in a "flat" manner (no need for recursion for folders).
							filesChecked.reserve(filesChecked.GetCount() + result.GetCount());
							std::copy(result.begin(), result.end(), std::back_inserter(filesChecked));
						}
						else if (fileOrFolder.EndsWith(Helpers::Wx::Files::FILE_EXTENSION_PLAYLIST)) // Playlist file
						{
							const wxArrayString& filesInPlaylist = Helpers::Wx::Files::LoadPathsFromPlaylist(fileOrFolder);
							filesChecked.reserve(filesChecked.GetCount() + filesInPlaylist.GetCount());
							std::copy(filesInPlaylist.begin(), filesInPlaylist.end(), std::back_inserter(filesChecked));
						}
						else // Plain file
						{
							if (fileOrFolder != selfExecutablePath)
							{
								filesChecked.push_back(fileOrFolder);
							}
						}
					}
					else if (IsWithinZipFile(fileOrFolder))
					{
						const wxString& archiveFilename = SplitZipArchiveAndFileNames(fileOrFolder).first;
						if (wxFileExists(archiveFilename))
						{
							filesChecked.push_back(fileOrFolder);
						}
					}
					else if (wxDirExists(fileOrFolder))
					{
						// Neccessary in order to process any encountered Zip files.
						wxArrayString filesInFolder;
						wxDir::GetAllFiles(fileOrFolder, &filesInFolder);
						filesInFolder = GetValidFiles(filesInFolder);

						filesChecked.reserve(filesChecked.GetCount() + filesInFolder.GetCount());
						std::copy(filesInFolder.begin(), filesInFolder.end(), std::back_inserter(filesChecked));
					}
				}

				return filesChecked;
			}

			std::pair<wxString, wxString> SplitZipArchiveAndFileNames(const wxString& filename)
			{
				const size_t toExt = filename.find(FILE_EXTENSION_ZIP);
				assert(toExt != wxString::npos);

				const size_t extLen = FILE_EXTENSION_ZIP.length();
				const size_t filenameLen = filename.length();

				const wxString& zipArchive = filename.Left(toExt + extLen);
				const wxString& zipFile = (toExt + extLen != filenameLen) ? filename.Right(filenameLen - (toExt + extLen + 1)) : "";
				return std::pair<wxString, wxString>(zipArchive, zipFile);
			}

			std::unique_ptr<BufferHolder> GetFileContentFromZip(const wxString& filename)
			{
				assert(IsWithinZipFile(filename));
				std::unique_ptr<BufferHolder> bufferHolder;

				const auto& archiveAndFile = SplitZipArchiveAndFileNames(filename);

				wxFileSystem fs;
				fs.ChangePathTo(archiveAndFile.first); // Prevent OpenFile from trying relative scope first (always in vain). This yields some speed boost.
				wxString filenameOnly = archiveAndFile.first;
				filenameOnly.Replace("\\", "/");
				if (!fs.GetPath().IsEmpty())
				{
					filenameOnly.Replace(fs.GetPath(), "");
				}

				wxFSFile* zip = fs.OpenFile(wxString::Format("%s#zip:%s", filenameOnly, archiveAndFile.second), wxFS_READ);
				if (zip != NULL)
				{
					wxInputStream* in = zip->GetStream();

					const size_t zipStreamSize = in->GetSize();

					if (zipStreamSize != 0)
					{
						bufferHolder = std::make_unique<BufferHolder>(zipStreamSize);
						if (!in->ReadAll(bufferHolder->buffer, bufferHolder->size))
						{
							bufferHolder = nullptr;
						}
					}

					delete zip;
				}

				return bufferHolder;
			}

			std::unique_ptr<BufferHolder> GetFileContentFromDisk(const wxString& filename)
			{
				std::unique_ptr<BufferHolder> bufferHolder;

				wxFileSystem fs;
				fs.ChangePathTo(wxFileName(filename).GetPath()); // Prevent OpenFile from trying relative scope first (always in vain). This yields some speed boost.
				wxFSFile* file = fs.OpenFile(filename, wxFS_READ);
				if (file != NULL)
				{
					wxInputStream* in = file->GetStream();

					const size_t fileStreamSize = in->GetSize();

					if (fileStreamSize != 0)
					{
						bufferHolder = std::make_unique<BufferHolder>(fileStreamSize);
						if (!in->ReadAll(bufferHolder->buffer, bufferHolder->size))
						{
							bufferHolder = nullptr;
						}
					}

					delete file;
				}

				return bufferHolder;
			}

			bool TrySavePlaylist(wxString fullpath, const std::vector<wxString>& fileList)
			{
				if (fullpath == DEFAULT_PLAYLIST_NAME)
				{
					fullpath = GetConfigFilePath(fullpath);
				}

				// Save to playlist...
				wxTextFile file;
				bool success = wxFileExists(fullpath) ? file.Open(fullpath) : file.Create(fullpath);
				if (success)
				{
					file.Clear();
					file.AddLine("#");

					for (const wxString& path : fileList)
					{
						file.AddLine(path);
					}

					success = file.Write();
					file.Close();
				}

				return success;
			}

			wxArrayString LoadPathsFromPlaylist(const wxString& fullpath)
			{
				wxArrayString retFileList;

				wxTextFile file;
				bool success = file.Open(fullpath);
				if (success)
				{
					retFileList.reserve(file.GetLineCount() - 1); // The very first line in the file (which is '#') is always skipped (because we only ever use the GetNextLine method).
					while (!file.Eof())
					{
						const wxString& line = file.GetNextLine();
						if (line.StartsWith("#") || line.IsEmpty())
						{
							continue;
						}
						retFileList.Add(line);
					}
				}

				return retFileList;
			}
		}

		namespace Audio
		{
			wxArrayString GetAudioDevicesNames(Audio::DeviceType type, Audio::Backend backend)
			{
				wxArrayString deviceNames;

				const int numDevices = Pa_GetDeviceCount();
				if (numDevices > 0)
				{
					for(int i = 0; i < numDevices; ++i)
					{
						const PaDeviceInfo* const deviceInfo = Pa_GetDeviceInfo(i);
						const PaHostApiInfo* const hostApiInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);

						// DirectSound has huge problems after libsidplayfp 2.5.0 (I don't know why, maybe the OpenMP messes it up somehow, who knows...).
						if (backend == Audio::Backend::All || (hostApiInfo->type != paWDMKS && hostApiInfo->type != paDirectSound))
						{
							if (type != Audio::DeviceType::Output || deviceInfo->maxOutputChannels > 0)
							{
								deviceNames.push_back(wxString::Format("%s: %s", hostApiInfo->name, deviceInfo->name));
							}
						}
					}
				}

				return deviceNames;
			}

			wxString GetDefaultAudioOutDeviceName()
			{
				const PaDeviceIndex defaultOutDevice = Pa_GetDefaultOutputDevice();
				if (defaultOutDevice == paNoDevice)
				{
					return "";
				}

				return GetAudioDevicesNames(Audio::DeviceType::Any, Audio::Backend::All).Item(defaultOutDevice);
			}

			int TryGetFilteredFromAbsoluteAudioDeviceIndex(int absoluteIndex)
			{
				const wxString targetName = GetAudioDevicesNames(Audio::DeviceType::Any, Audio::Backend::All).Item(absoluteIndex);
				const int filteredIndex = GetAudioDevicesNames(Audio::DeviceType::Output, Audio::Backend::Filtered).Index(targetName);
				return filteredIndex;
			}

			int TryGetAudioDeviceIndex(const wxString& deviceName)
			{
				if (deviceName.IsEmpty())
				{
					return paNoDevice;
				}

				const wxArrayString& outDevices = GetAudioDevicesNames(Audio::DeviceType::Any, Audio::Backend::All);
				int index = 0;
				for (const wxString& cName: outDevices)
				{
					if (cName == deviceName)
					{
						if (TryGetFilteredFromAbsoluteAudioDeviceIndex(index) == wxNOT_FOUND)
						{
							break;
						}

						return index;
					}
					++index;
				}

				return paNoDevice; // Stored device no longer present.
			}
		}

		namespace Input
		{
			bool mediaKeyDownProcessed = true;
			wxKeyCode GetMediaKeyCommand()
			{
				const wxKeyCode activeMediaKey = GetMediaKeyDown();
				const wxKeyCode retMediaKey = (mediaKeyDownProcessed) ? WXK_NONE : activeMediaKey; // Return real key only if we didn't process this particular KeyDown "event" already (to prevent repeating).
				mediaKeyDownProcessed = activeMediaKey != WXK_NONE;
				return retMediaKey;
			}
		}
	}
}
