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

#include "SimpleTimer.h"

#include <chrono>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

SimpleTimer::SimpleTimer(Type type, unsigned long delayMs, Callback callback) :
	_type(type),
	_delayMs(delayMs),
	_callback(callback)
{
	Restart();
}

SimpleTimer::~SimpleTimer()
{
	Abort();
}

bool SimpleTimer::IsRunning() const
{
	return _thread.joinable();
}

unsigned long SimpleTimer::GetElapsed() const
{
	return _elapsedMs;
}

unsigned long SimpleTimer::GetDelay() const
{
	return _delayMs;
}

void SimpleTimer::Restart()
{
	constexpr int RESOLUTION_MS = 10; // milliseconds

	Abort();
	_elapsedMs = 0;

	_thread = std::thread([this]()
	{
		while ((_elapsedMs < _delayMs) && !_aborting)
		{
			const auto start = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
			Sleep(RESOLUTION_MS);
#else
			usleep(RESOLUTION_MS * 1000); // POSIX function (takes microseconds). There is also a more modern nanosleep() function but is more ugly to use.
#endif
			const auto end = std::chrono::high_resolution_clock::now();
			const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

			_elapsedMs += duration;
		}

		if (!_aborting)
		{
			_callback();
			if (_type == Type::Repeating)
			{
				Restart();
			}
		}
	});
}

void SimpleTimer::Abort()
{
	if (IsRunning())
	{
		_aborting = true;

		try
		{
			_thread.join();
		}
		catch (const std::system_error& /*e*/)
		{
			// Just in case the timer thread expired in the meantime.
		}

		_aborting = false;
	}
}
