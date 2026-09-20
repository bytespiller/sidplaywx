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

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class PlaybackController;
class Songlengths;
class Stil;

/// @brief Parses a batch of SID/MUS files across a pool of background worker threads. The GUI thread drives progress by polling GetCompletedCount()/IsFinished() (e.g. from a wxTimer tick) and draining ready results with TryTakeResult(), mirroring how PreRender publishes its own progress via plain atomics for the owner to poll rather than pushing a per-item notification.
/// @remark Not thread-safe for its own public API to be called from multiple threads simultaneously - Start()/Abort() and the polling getters are all meant to be called only from the GUI thread that owns this pool, while the parsing itself happens on the internal worker threads.
class SidFileParsePool
{
public:
	struct FileTask
	{
		std::string filepathUtf8;
		std::string musCompanionStrFilePathUtf8; // Empty if this file has no MUS+STR companion.
	};

public:
	SidFileParsePool() = default;
	SidFileParsePool(const SidFileParsePool&) = delete;
	~SidFileParsePool();

public:
	/// @brief Starts parsing "files" across up to "workerCount" background threads. Safe to call again for a new batch (any still-running prior batch is aborted and joined first).
	/// @param sidDatabase Must outlive this pool's run (e.g. a FramePlayer member) - referenced, not copied.
	/// @param stilInfo Must outlive this pool's run - referenced, not copied. Its Get() must be reentrant (see Stil.h).
	/// @param playback Must outlive this pool's run - referenced, not copied.
	void Start(std::vector<FileTask> files, unsigned int workerCount, const Songlengths& sidDatabase, Stil& stilInfo, const PlaybackController& playback);

	/// @brief Aborts any in-progress parsing and joins all worker threads (discarding any results nobody claimed via TryTakeResult). Safe to call even if not started / already finished. Automatically called by the destructor.
	void Abort();

	size_t GetTotalCount() const;
	size_t GetCompletedCount() const;
	bool IsFinished() const;

	/// @brief Takes ownership of the parsed result at the given (original file list) index if it's ready, leaving that slot empty so it won't be returned again. Returns nullptr if not ready yet or the index is out of range.
	std::unique_ptr<ParsedSongResult> TryTakeResult(size_t index);

private:
	std::vector<FileTask> _files;
	std::vector<std::thread> _workers;
	std::vector<std::atomic<ParsedSongResult*>> _results;
	std::atomic<size_t> _nextIndex = 0;
	std::atomic<size_t> _completedCount = 0;
	std::atomic_bool _abortRequested = false;
};
