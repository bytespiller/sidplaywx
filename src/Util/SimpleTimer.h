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

#include <atomic>
#include <functional>
#include <thread>

/// @brief Utility class for periodic non-precise non-critical short-term callback invocation (e.g., updating UI every 100ms, delayed call etc.).
class SimpleTimer
{
public:
	enum class Type
	{
		OneShot,
		Repeating
	};

	using Callback = std::function<void(void)>;

public:
	SimpleTimer() = delete;
	SimpleTimer(SimpleTimer&) = delete;

	SimpleTimer(Type type, unsigned long delayMs, Callback callback);
	~SimpleTimer();

	SimpleTimer operator=(const SimpleTimer&) = delete;

public:
	bool IsRunning() const;

	/// @brief Returns currently elapsed milliseconds (not physically accurate).
	unsigned long GetElapsed() const;

	/// @brief Returns configured delay milliseconds (i.e., a fixed value).
	unsigned long GetDelay() const;

	void Restart();
	void Abort();

private:
	std::atomic<Type> _type{Type::OneShot};
	std::atomic_ulong _delayMs = 0;
	Callback _callback;
	std::thread _thread;
	std::atomic_ulong _elapsedMs = 0;
	std::atomic_bool _aborting = false;
};
