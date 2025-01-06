/*
This file is a part of MonaSolutions Copyright 2017
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or
modify it under the terms of the the Mozilla Public License v2.0.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License v. 2.0 received along this program for more
details (or else see http://mozilla.org/MPL/2.0/).

*/

#pragma once

#include "Mona/Mona.h"
#include "Mona/Timing/Time.h"
#include "Mona/Util/Exceptions.h"
#include <set>
#include <map>

namespace Mona {


struct Timer : virtual Object {
	Timer() : _count(0) {}
	~Timer();

/**
	OnTimer is a function which returns the timeout in ms of next call, or 0 to stop the timer.
	"count" parameter informs on the number of raised time */
	struct OnTimer : std::function<uint32_t(uint32_t delay)>, virtual Object {
		NULLABLE(!_nextRaising)

		OnTimer() : _nextRaising(0), count(0) {}
		// explicit to forbid to pass in "const OnTimer" parameter directly a lambda function
		template<typename FunctionType>
		explicit OnTimer(FunctionType&& function) : _nextRaising(0), count(0), std::function<uint32_t(uint32_t)>(std::move(function)) {}

		~OnTimer() { if (_nextRaising) FATAL_ERROR("OnTimer function deleting while running"); }

		const Time& nextRaising() const { return _nextRaising; }
	
		template<typename FunctionType>
		OnTimer& operator=(FunctionType&& function) {
			std::function<uint32_t(uint32_t)>::operator=(std::move(function));
			return *this;
		}

		/**
		Call the timer, also allow to call the timer on set => timer.set(onTimer, onTimer()); */
		uint32_t operator()(uint32_t delay = 0) const { ++(uint32_t&)count; return std::function<uint32_t(uint32_t)>::operator()(delay); }

		const uint32_t count;
	private:
		mutable Time	_nextRaising;

		friend struct Timer;
	};

	uint32_t count() const { return _count; }

/**
	Set timer, timeout is the first raising timeout
	If timeout is 0 it removes the timer! */
	const Timer::OnTimer& set(const Timer::OnTimer& onTimer, uint32_t timeout) const;

/**
	Return the time in ms to wait before to call one time again raise method (>0), or 0 if there is no more timer! */
	uint32_t raise();

private:
	void add(const OnTimer& onTimer,  uint32_t timeout) const;
	bool remove(const OnTimer& onTimer, Shared<std::set<const OnTimer*>>& pMove) const;

	mutable	uint32_t												_count;
	mutable std::map<int64_t, Shared<std::set<const OnTimer*>>>	_timers;
};



} // namespace Mona
