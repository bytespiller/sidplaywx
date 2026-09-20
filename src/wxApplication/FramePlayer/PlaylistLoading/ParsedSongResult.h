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

#pragma once

#include "../../UIElements/Playlist/Components/PlaylistModel.h"

#include <cstdint>
#include <string>
#include <vector>

/// @brief Everything ParseSongFile() computes for a single file, built entirely from plain (non-wx) data so it can safely cross from a background parser thread to the GUI thread.
/// @remark Deliberately mirrors the constructor parameters of Playlist::AddMainSong()/AddSubsongs() (plus the small amount of bookkeeping FramePlayer::SendFilesToPlaylist() needs) so applying a batch of these on the GUI thread is a direct, mechanical translation.
/// @remark Every text field is UTF-8 (converted via wxString::ToUTF8()/FromUTF8(), which is lossless regardless of build/locale, unlike ToStdString()) rather than wxString, so results can be built on a background thread and moved to the GUI thread without touching wxString's own (not thread-safe to share) internals.
struct ParsedSongResult
{
	/// @brief False if the file couldn't be parsed as a valid tune (mirrors the original loop's "tuneIsValid" check) - such entries are skipped entirely during batch-apply, same as today.
	bool tuneIsValid = false;

	std::string title; // UTF-8
	std::string filepath; // UTF-8
	int defaultSubsong = 0;
	std::uint_least32_t duration = 0;
	std::string hvscPath; // Plain ASCII HVSC catalog path.
	std::string md5; // Plain hex-ASCII.
	std::string author; // UTF-8
	std::string copyright; // UTF-8
	PlaylistTreeModelNode::RomRequirement romRequirement = PlaylistTreeModelNode::RomRequirement::None;
	bool playable = false;

	/// @brief Non-empty only for a MUS file with a companion STR file (mirrors AddMainSong's musCompanionStrFilePath parameter). UTF-8.
	std::string musCompanionStrFilePath;

	/// @brief Parallel to subsongTitles. Empty if there are no (real or fake MUS+STR) subsongs.
	std::vector<std::uint_least32_t> subsongDurations;

	/// @brief Parallel to subsongDurations. UTF-8.
	std::vector<std::string> subsongTitles;
};
