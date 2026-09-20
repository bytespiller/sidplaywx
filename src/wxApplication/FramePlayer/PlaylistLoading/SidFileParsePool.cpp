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

#include "SidFileParsePool.h"
#include "PlaylistFileParser.h"

#include <algorithm>

SidFileParsePool::~SidFileParsePool()
{
	Abort();
}

void SidFileParsePool::Start(std::vector<FileTask> files, unsigned int workerCount, const Songlengths& sidDatabase, Stil& stilInfo, const PlaybackController& playback)
{
	Abort(); // Defensive: ensure any prior run is fully stopped first (mirrors PreRender::DoPreRender's own AbortPreRender() call).

	_files = std::move(files);

	_results = std::vector<std::atomic<ParsedSongResult*>>(_files.size());
	for (std::atomic<ParsedSongResult*>& slot : _results)
	{
		slot = nullptr;
	}

	_nextIndex = 0;
	_completedCount = 0;
	_abortRequested = false;

	if (_files.empty())
	{
		return;
	}

	const unsigned int effectiveWorkerCount = std::max(1u, std::min(workerCount, static_cast<unsigned int>(_files.size())));
	_workers.reserve(effectiveWorkerCount);

	for (unsigned int w = 0; w < effectiveWorkerCount; ++w)
	{
		_workers.emplace_back([this, &sidDatabase, &stilInfo, &playback]()
		{
			for (;;)
			{
				if (_abortRequested)
				{
					return;
				}

				const size_t i = _nextIndex.fetch_add(1, std::memory_order_relaxed);
				if (i >= _files.size())
				{
					return;
				}

				const FileTask& task = _files[i];
				std::unique_ptr<ParsedSongResult> result = std::make_unique<ParsedSongResult>(PlaylistFileParser::ParseSongFile(task.filepathUtf8, task.musCompanionStrFilePathUtf8, sidDatabase, stilInfo, playback));

				_results[i].store(result.release(), std::memory_order_release);
				_completedCount.fetch_add(1, std::memory_order_release);
			}
		});
	}
}

void SidFileParsePool::Abort()
{
	_abortRequested = true;

	for (std::thread& worker : _workers)
	{
		if (worker.joinable())
		{
			worker.join();
		}
	}
	_workers.clear();

	// Free any results nobody claimed via TryTakeResult (e.g. the load was aborted mid-flight, or the playlist was cleared before the last batch was applied).
	for (std::atomic<ParsedSongResult*>& slot : _results)
	{
		delete slot.exchange(nullptr, std::memory_order_acq_rel);
	}

	_abortRequested = false;
}

size_t SidFileParsePool::GetTotalCount() const
{
	return _files.size();
}

size_t SidFileParsePool::GetCompletedCount() const
{
	return _completedCount.load(std::memory_order_acquire);
}

bool SidFileParsePool::IsFinished() const
{
	return GetCompletedCount() >= GetTotalCount();
}

std::unique_ptr<ParsedSongResult> SidFileParsePool::TryTakeResult(size_t index)
{
	if (index >= _results.size())
	{
		return nullptr;
	}

	return std::unique_ptr<ParsedSongResult>(_results[index].exchange(nullptr, std::memory_order_acq_rel));
}
