/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>

namespace Signal
{
	using Func = std::function<void(int)>;
}

template<typename ENUM_TYPE>
class SimpleSignalProvider
{
public:
	struct Subscriber
	{
		Subscriber() = delete;
		Subscriber(Signal::Func aCallback, ENUM_TYPE aEventId) :
			callback(aCallback),
			eventId(aEventId)
		{
		}

		Signal::Func callback;
		const ENUM_TYPE eventId;
	};

	using Subscription = std::shared_ptr<Subscriber>;

private:
	struct SubscriptionRef
	{
		SubscriptionRef() = delete;
		explicit SubscriptionRef(const Subscription& aSubscription) :
			subscriber(aSubscription)
		{
		}

		std::weak_ptr<Subscriber> subscriber;
	};

public:
	SimpleSignalProvider() = default;

public:
	[[nodiscard("Subscription dropped!")]] Subscription Subscribe(ENUM_TYPE eventId, Signal::Func callback)
	{
		std::shared_ptr<Subscriber> newSubscriber = std::make_shared<Subscriber>(callback, eventId);
		_subscribers.emplace_back(newSubscriber);
		return newSubscriber;
	}

	void Unsubscribe(const Subscriber& subscriber)
	{
		_subscribers.erase(
			std::remove_if(_subscribers.begin(), _subscribers.end(), [&subscriber](const SubscriptionRef& subRef)
			{
				if (auto weakSubscriber = subRef.subscriber.lock())
				{
					return weakSubscriber.get() == &subscriber;
				}

				return true; // This one is dead anyway since the lock failed.
			}),
   			_subscribers.end()
		);

		// Reminder: C++20 has std::erase_if so update this code perhaps in the future.
	}

protected:
	virtual void EmitSignal(ENUM_TYPE eventId, int param = 0)
	{
		CleanupDeadSubscribers();

		for (auto&& subRef : _subscribers)
		{
			if (auto weakSubscriber = subRef.subscriber.lock())
			{
				if (weakSubscriber->eventId == eventId)
				{
					Execute(*weakSubscriber, param);
				}
			}
			else
			{
	#ifndef NDEBUG
				throw std::runtime_error("There is a dead subscriber that hasn't been cleaned up!");
	#endif
			}
		}
	}

	virtual void Execute(Subscriber& subscriber, int param)
	{
		subscriber.callback(param);
	}

private:
	void CleanupDeadSubscribers()
	{
		_subscribers.erase(
			std::remove_if(_subscribers.begin(), _subscribers.end(), [](const SubscriptionRef& subRef)
			{
				return subRef.subscriber.expired();
			}),
			_subscribers.end()
		);

		// Reminder: C++20 has std::erase_if so update this code perhaps in the future.
	}

private:
	std::vector<SubscriptionRef> _subscribers;
};
