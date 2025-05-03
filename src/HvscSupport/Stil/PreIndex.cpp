/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2024-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "PreIndex.h"

static constexpr const char* PRE_INDEX_FILE_NAME = "stil.index";
static constexpr const char* PRE_INDEX_FORMAT_VERSION = "1";
static constexpr const char PRE_INDEX_NEWLINE = '\n';

bool PreIndex::TryLoadFromCache(const std::string& stilVersion, HvscPathsIndex& pathsIndex)
{
	bool success = false;

	std::ifstream preIndexInStream(PRE_INDEX_FILE_NAME);
	if (preIndexInStream.good())
	{
		std::string line;
		if (NextLineEquals(preIndexInStream, line, PRE_INDEX_FORMAT_VERSION) && // Check the index file format version. If we've changed the format in a new sidplaywx update, this existing older file will be invalidated.
			NextLineEquals(preIndexInStream, line, stilVersion.c_str())) // Check the STIL.txt version for which this pre-index file was generated. If it differs it will be invalidated.
		{
			const int expectedEntriesCount = NextLineAsInt(preIndexInStream, line);
			if (expectedEntriesCount > 0)
			{
				std::string position;
				while(std::getline(preIndexInStream, line) && !line.empty())
				{
					std::getline(preIndexInStream, position);
					try
					{
						pathsIndex[line] = std::stoi(position);
					}
					catch (std::invalid_argument)
					{
						break; // Index file is corrupted.
					}
				}

				if (pathsIndex.size() == expectedEntriesCount) // Check the integrity of the index file.
				{
					success = true;
				}
				else
				{
					pathsIndex.clear(); // Index file is corrupted.
				}
			}
		}
	}

	preIndexInStream.close();
	return success;
}

void PreIndex::RebuildIndexAndCache(const std::string& stilVersion, HvscPathsIndex& pathsIndex, std::ifstream& stilDataStream)
{
	// Do the actual (slow) pre-indexing
	{
		stilDataStream.clear(); // Rewind (just in case).

		std::string line;
		while (std::getline(stilDataStream, line))
		{
			if (line.front() == '/')
			{
				ClipCarriageReturn(line); // Reminder: any existing "line" iterators are invalid now.
				pathsIndex[std::string(line.begin(), line.end())] = stilDataStream.tellg(); // Reminder: we naively convert wstring to string here since the HVSC paths are always ASCII so there's no need for complications.
			}
		}
	}

	// Write a new index file so that we can skip the expensive pre-indexing next time
	{
		std::ofstream preIndexOutStream(PRE_INDEX_FILE_NAME, std::ios::trunc | std::ios::binary);

		if (preIndexOutStream.good())
		{
			preIndexOutStream << PRE_INDEX_FORMAT_VERSION << PRE_INDEX_NEWLINE;
			preIndexOutStream << stilVersion << PRE_INDEX_NEWLINE;
			preIndexOutStream << pathsIndex.size() << PRE_INDEX_NEWLINE;
			for (const auto& [hvscPath, position] : pathsIndex)
			{
				preIndexOutStream << hvscPath << PRE_INDEX_NEWLINE;
				preIndexOutStream << position << PRE_INDEX_NEWLINE;
			}
		}

		preIndexOutStream.close();
	}
}
