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

unsigned long SimpleTimer::GetDelay() const
{
	return _delayMs;
}

void SimpleTimer::Restart()
{
	Abort();

	const auto chronoDelay = std::chrono::milliseconds(_delayMs);
	_thread = std::thread([this, chronoDelay]()
	{
		auto next = std::chrono::steady_clock::now() + chronoDelay;
		while (!_aborting)
		{
			std::this_thread::sleep_until(next);
			if (_aborting) break;

			_callback();

			if (_type != Type::Repeating) break;
			next += chronoDelay;
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
