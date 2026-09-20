/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include "PlaylistFileParser.h"

#include "../../Config/UIStrings.h"
#include "../../Helpers/HelpersWx.h"
#include "../../../HvscSupport/Songlengths.h"
#include "../../../HvscSupport/Stil/Stil.h"
#include "../../../PlaybackController/PlaybackController.h"
#include "../../../PlaybackController/PlaybackWrappers/Input/SidDecoder/TuneUtil.h"

#include <cassert>
#include <memory>
#include <mutex>

namespace
{
	/// @brief └
	constexpr int BOX_CHAR_L = 0x2514;

	/// @brief ├
	constexpr int BOX_CHAR_VERT_RIGHT = 0x251C;

	/// @brief Maps between the two structurally-identical (but separately defined, to avoid a header dependency) RomRequirement enums. Deliberately doesn't use wxMessageBox/throw (unlike the model-layer switches elsewhere) since this can run on a background parser thread.
	PlaylistTreeModelNode::RomRequirement ToModelRomRequirement(TuneUtil::RomRequirement romRequirement)
	{
		switch (romRequirement)
		{
			case TuneUtil::RomRequirement::None:
				return PlaylistTreeModelNode::RomRequirement::None;
			case TuneUtil::RomRequirement::BasicRom:
				return PlaylistTreeModelNode::RomRequirement::BasicRom;
			case TuneUtil::RomRequirement::R64:
				return PlaylistTreeModelNode::RomRequirement::R64;
		}

		assert(false && "Unhandled TuneUtil::RomRequirement case!");
		return PlaylistTreeModelNode::RomRequirement::None;
	}
}

ParsedSongResult PlaylistFileParser::ParseSongFile(const std::string& filepathUtf8, const std::string& musCompanionStrFilePathUtf8, const Songlengths& sidDatabase, Stil& stilInfo, const PlaybackController& playback)
{
	ParsedSongResult result;
	result.filepath = filepathUtf8;
	result.musCompanionStrFilePath = musCompanionStrFilePathUtf8;

	const wxString filepath = wxString::FromUTF8(filepathUtf8);

	// Inspect the tune -------
	std::unique_ptr<SidTune> inspectTune;
	{
		const std::unique_ptr<BufferHolder>& infoTuneBufferHolder = (Helpers::Wx::Files::IsWithinZipFile(filepath))
				? Helpers::Wx::Files::GetFileContentFromZip(filepath)
				: Helpers::Wx::Files::GetFileContentFromDisk(filepath);

		inspectTune = (infoTuneBufferHolder != nullptr) ? std::make_unique<SidTune>(infoTuneBufferHolder->buffer[0], infoTuneBufferHolder->size[0]) : nullptr;
		result.tuneIsValid = inspectTune != nullptr && inspectTune->getStatus();
	}

	if (!result.tuneIsValid)
	{
		return result;
	}

	// Tune title
	wxString songTitle(TuneUtil::GetTuneInfoString(*inspectTune, TuneUtil::SongInfoCategory::Title));
	{
		if (songTitle.IsEmpty()) [[unlikely]] // Fallback/MUS file (rare situation)
		{
			songTitle = wxFileNameFromPath(filepath);
		}

		const int sidsNeeded = inspectTune->getInfo()->sidChips();
		const wxString& songTitleAddendum = (sidsNeeded > 1) ? wxString::Format(" [%iSID]", sidsNeeded) : wxGetEmptyString();
		songTitle.Append(songTitleAddendum);
	}

	// Subsongs count
	result.defaultSubsong = inspectTune->getInfo()->startSong();
	int totalSubsongs = inspectTune->getInfo()->songs();

	// Tune ROM requirement
	const TuneUtil::RomRequirement romRequirement = TuneUtil::GetTuneRomRequirement(*inspectTune);
	result.romRequirement = ToModelRomRequirement(romRequirement);
	result.playable = playback.IsRomLoaded(romRequirement);

	// MD5 & HVSC info (copy the MD5 out of inspectTune's own buffer immediately, since that buffer won't outlive this function)
	{
		// libsidplayfp's createMD5New() delegates to libgcrypt, which is not safe to call concurrently from multiple threads without its own explicit multi-thread initialization (observed in practice: concurrent first-use crashes libgcrypt into a permanent "Fatal-Error" state). Serializing just this call (a small fraction of the total per-file work) keeps the rest of parsing - file I/O, tune header parsing, Songlengths/STIL lookups - fully parallel.
		static std::mutex md5Mutex;
		const std::lock_guard<std::mutex> lock(md5Mutex);
		const char* const md5Raw = inspectTune->createMD5New();
		result.md5 = (md5Raw != nullptr) ? md5Raw : "";
	}

	const Songlengths::HvscInfo hvscInfoMain = sidDatabase.GetHvscInfo(result.md5.c_str());
	result.duration = hvscInfoMain.duration;
	result.hvscPath = hvscInfoMain.hvscPath;

	result.author = Helpers::Wx::StringFromWin1252(TuneUtil::GetTuneInfoString(*inspectTune, TuneUtil::SongInfoCategory::Author)).ToUTF8();
	result.copyright = Helpers::Wx::StringFromWin1252(TuneUtil::GetTuneInfoString(*inspectTune, TuneUtil::SongInfoCategory::Released)).ToUTF8();

	const bool musPlusStr = !result.musCompanionStrFilePath.empty();
	if (musPlusStr)
	{
		totalSubsongs = 3; // Add as fake subsongs so that individual MUS+STR components can be selected by the user if so desired.
	}

	// Reminder: the original code round-trips the title through StringFromWin1252 right before storing it (same as author/copyright above), which is why this doesn't just take songTitle.ToUTF8() directly - and any MUS+STR fake subsong titles built below must be based on this final, converted title (matching what the model itself ends up storing), not the raw songTitle.
	const wxString finalTitle = Helpers::Wx::StringFromWin1252(songTitle.ToStdString());
	result.title = finalTitle.ToUTF8();

	// Add any subsongs
	if (totalSubsongs > 1)
	{
		result.subsongDurations.reserve(totalSubsongs);
		result.subsongTitles.reserve(totalSubsongs);

		// Determine durations
		for (int i = 1; i <= totalSubsongs; ++i)
		{
			result.subsongDurations.emplace_back(sidDatabase.GetHvscInfo(result.md5.c_str(), i).duration);
		}

		// Determine subsong titles (from STIL where possible)
		if (!musPlusStr) // Normal (or standalone MUS) tune
		{
			const Stil::Info info(stilInfo.Get(result.hvscPath));
			for (int i = 1; i <= totalSubsongs; ++i)
			{
				const int boxChar = (i < totalSubsongs) ? BOX_CHAR_VERT_RIGHT : BOX_CHAR_L;
				const std::string& subsongTitle = info.GetFieldAsString(info.names, i);

				const wxString formattedTitle = (!subsongTitle.empty())
					? wxString::Format("%c %s %i: %s", boxChar, Strings::PlaylistTree::SUBSONG, i, Helpers::Wx::StringFromWin1252(subsongTitle)) // STIL title
					: wxString::Format("%c %s %i", boxChar, Strings::PlaylistTree::SUBSONG, i); // Generic subsong title

				result.subsongTitles.emplace_back(formattedTitle.ToUTF8());
			}
		}
		else // MUS+STR tune
		{
			const wxString baseTitle = finalTitle.Mid(0, finalTitle.Length() - 4);
			result.subsongTitles.emplace_back(wxString::Format("%c %s", BOX_CHAR_VERT_RIGHT, baseTitle + " [MUS+STR]").ToUTF8());
			result.subsongTitles.emplace_back(wxString::Format("%c %s", BOX_CHAR_VERT_RIGHT, finalTitle).ToUTF8());
			result.subsongTitles.emplace_back(wxString::Format("%c %s", BOX_CHAR_L, baseTitle + ".str").ToUTF8());
		}
	}

	return result;
}
