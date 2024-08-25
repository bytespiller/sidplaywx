/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "Stil.h"
#include "PreIndex.h"

static const std::wstring STIL_VERSION_PREFIX(L"#  STIL v");
static constexpr const char STIL_CHAR_COMMENT = '#';
static constexpr size_t STIL_LABEL_LENGTH = 9; // Labels (NAME, TITLE, ARTIST, AUTHOR, COMMENT) always end with a colon and a space and have 9 characters total (all padded with spaces except the COMMENT).

#pragma region Helpers

namespace
{
	// Joins the vector to a string.
	inline std::wstring Join(const std::vector<std::wstring>& data, const std::wstring& separator = L", ", bool showCount = false)
	{
		if (data.empty())
		{
			return L"";
		}

		std::wstring ret;

		size_t cnt = 0;
		const size_t len = data.size();
		for (const std::wstring& item : data)
		{
			++cnt;

			if (showCount && len > 1)
			{
				ret.append(std::to_wstring(cnt) + L") ");
			}

			ret.append(item);

			if (cnt < len)
			{
				ret.append(separator);
			}
		}

		return ret;
	}

	// Adds a line content (skipping the label prefix) into the target field under an existing key.
	inline void AddLineContent(Stil::Field& target, int key, const std::wstring& line)
	{
		target[key].emplace_back(line.substr(STIL_LABEL_LENGTH));
	}
}

#pragma endregion
#pragma region Info struct

std::vector<std::wstring> Stil::Info::GetField(const Field& field, int subsong) const
{
	if (field.empty())
	{
		return std::vector<std::wstring>();
	}

	if (field.find(subsong) != field.end())
	{
		return field.at(subsong); // A subsong-specific entry exists, yay!
	}

	if (field.size() == 1 && field.find(1) != field.end())
	{
		return field.at(1); // Fallback to the first and only one.
	}

	return std::vector<std::wstring>(); // Any others are specific to some other sub-songs, so we came up empty for this particular subsong.
}

std::wstring Stil::Info::GetFieldAsString(const Field& field, int subsong, const std::wstring& separator, bool showCount) const
{
	const std::vector<std::wstring>& data = Stil::Info::GetField(field, subsong);
	return Join(data, separator, showCount);
}

#pragma endregion
#pragma region Actual Stil class

Stil::Stil()
{
	_hvscPathsIndex.rehash(0); // This makes all clear() calls instant (orders of magnitude faster) and doesn't affect the filling speed. The app closes faster.
}

Stil::~Stil()
{
	Unload();
}

bool Stil::TryLoad(const std::wstring& stilFilepath)
{
	Unload();
	std::string stilVersion;

	// Lock & load the STIL.txt file
	{
		_stilDataStream.open(stilFilepath.c_str(), std::ios::binary);
		_stilDataStream.clear();
		if (!_stilDataStream.good())
		{
			return false;
		}

		// Determine the STIL.txt version
		{
			std::wstring line;
			const size_t len = STIL_VERSION_PREFIX.length();
			while (std::getline(_stilDataStream, line))
			{
				if (line.substr(0, len) == STIL_VERSION_PREFIX)
				{
					const size_t end = line.find(L' ', len);
					if (end != std::wstring::npos)
					{
						const std::wstring& wideStilVersion = line.substr(len, end - len);
						stilVersion = std::string(wideStilVersion.begin(), wideStilVersion.end()); // Reminder: we naively convert wstring to string here since the HVSC version is always ASCII so there's no need for complications.
					}
					break;
				}
			}

			if (stilVersion.empty())
			{
				return false; // Unknown STIL.txt version. Possibly corrupted file or not STIL.txt?
			}

			_stilDataStream.clear(); // Rewind & reset the stream.
		}
	}

	// Load the pre-index file if valid
	if (!PreIndex::TryLoadFromCache(stilVersion, _hvscPathsIndex))
	{
		// Pre-index the positions of STIL tunes, so when we want to fetch data for any tune later, we can do so without having to parse the entire file again
		PreIndex::RebuildIndexAndCache(stilVersion, _hvscPathsIndex, _stilDataStream);
	}

	_stilFilepath = stilFilepath;

	return IsLoaded();
}

void Stil::Unload()
{
	_hvscPathsIndex.clear();
	_stilFilepath.clear();
	_stilDataStream.close();
}

bool Stil::IsLoaded() const
{
	return !_hvscPathsIndex.empty();
}

Stil::Info Stil::Get(const std::string& tuneHvscPath)
{
	Stil::Info data;
	if (!IsLoaded())
	{
		return data; // STIL not loaded.
	}

	if (tuneHvscPath.empty())
	{
		return data; // Unknown SID tune.
	}

	// Parse info
	{
		const auto& itStart = _hvscPathsIndex.find(tuneHvscPath);
		if (itStart != _hvscPathsIndex.end())
		{
			_stilDataStream.clear();
			_stilDataStream.seekg(itStart->second);

			int subsongKey = 1; // Reminder: this is not index, but an unordered dict key!
			std::wstring line;
			while (std::getline(_stilDataStream, line))
			{
				ClipCarriageReturn(line); // Reminder: any existing "line" iterators are invalid now.

				// Treat an empty line as the end of this tune's data
				if (line.empty())
				{
					break;
				}

				const wchar_t firstChar = line.front(); // Reminder: shouldn't call .front() on an empty string, thus we have a preceding check for it above.

				// Treat beginning of the next tune as the end of this tune's data
				if (firstChar == L'/')
				{
					break;
				}

				// Skip a comment line
				if (firstChar == STIL_CHAR_COMMENT)
				{
					continue;
				}

				// Subsong selector
				if (firstChar == L'(' && line.length() > 2 && line.at(1) == L'#')
				{
					subsongKey = std::stoi(line.substr(2, line.length() - 1));
					continue;
				}

				// Extract NAME, TITLE, ARTIST, AUTHOR, COMMENT fields
				if (line.length() > STIL_LABEL_LENGTH)
				{
					const std::wstring& fieldLabel = line.substr(0, STIL_LABEL_LENGTH);

					if (fieldLabel == L"   NAME: ")
					{
						AddLineContent(data.names, subsongKey, line);
						continue;
					}

					if (fieldLabel == L"  TITLE: ")
					{
						AddLineContent(data.titles, subsongKey, line);
						continue;
					}

					if (fieldLabel == L" ARTIST: ")
					{
						AddLineContent(data.artists, subsongKey, line);
						continue;
					}

					if (fieldLabel == L" AUTHOR: ")
					{
						AddLineContent(data.authors, subsongKey, line);
						continue;
					}

					if (fieldLabel == L"COMMENT: ")
					{
						AddLineContent(data.comments, subsongKey, line);
						continue;
					}
					else if (fieldLabel == L"         ") // Multi-line comment's next line.
					{
						// Append it only if an actual comment exists (it would be weird if not, but we do this check anyway)
						if (data.comments.find(subsongKey) != data.comments.end() && !data.comments.at(subsongKey).back().empty())
						{
							data.comments[subsongKey].back().append(L" ").append(line.substr(STIL_LABEL_LENGTH));
						}

						continue;
					}
				}
			}
		}
	}

	return data;
}

#pragma endregion