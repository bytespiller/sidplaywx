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

#include "ParsedSongResult.h"

class PlaybackController;
class Songlengths;
class Stil;

namespace PlaylistFileParser
{
	/// @brief Parses a single SID/MUS tune file into a plain-data result. Thread-safe / reentrant (safe to call concurrently from multiple background parser threads, and safe to call alongside GUI-thread use of the same sidDatabase/stilInfo/playback objects), PROVIDED it is never given the same filepath as another concurrent call (each file's own I/O is independent).
	/// @param filepathUtf8 Absolute path of the file to parse (as discovered by Helpers::Wx::Files::GetValidFiles), UTF-8 encoded.
	/// @param musCompanionStrFilePathUtf8 Pre-resolved companion .str path for a .mus file (empty if none/not applicable) - resolved up-front on the GUI thread since it's a cheap, order-independent filesystem check (see FramePlayer::SendFilesToPlaylist's pre-pass). UTF-8 encoded.
	/// @param sidDatabase The (read-only, concurrency-safe once loaded) HVSC Songlengths database.
	/// @param stilInfo The STIL info database. Its Get() is reentrant (opens a fresh local file handle per call).
	/// @param playback Used only for IsRomLoaded(); safe for concurrent const reads.
	ParsedSongResult ParseSongFile(const std::string& filepathUtf8, const std::string& musCompanionStrFilePathUtf8, const Songlengths& sidDatabase, Stil& stilInfo, const PlaybackController& playback);
}
