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
#include <chrono>

namespace Mona {

struct Time : virtual Object {

	struct Elapsed : virtual Object {
		Elapsed() : _now(Time::Now()) {}
		int64_t operator()() const { return Time::Now() - _now; }
		int64_t operator()(int64_t time) const { return _now - time; }
		bool operator()(int64_t time, int64_t duration) const { return (_now - time) > duration; }
	private:
		int64_t _now;
	};

	/// \brief Construct a Time instance with current time value
	Time() : _time(Now()) {}

	Time(const Time& other) : _time(other._time) {}

	/// \brief Construct a Time instance with defined time value
	Time(int64_t time) : _time(time) {}

	/// \brief Copy Constructor
	virtual Time& operator=(const Time& other) { _time=other._time; return *this; }

	/// \brief Time in Microseconds since epoch
	operator int64_t() const { return time(); }

	/// \brief Return true if time (in msec) is elapsed since creation
	bool isElapsed(int64_t duration) const {return elapsed()>duration;}

	/// \brief Return time elapsed since creation (in �sec)
	int64_t elapsed() const { return Now() - time(); }

	/// \brief Update the time object with current time
	virtual Time& update() { _time = Now(); return *this; }

	/// \brief Update the time object with time parameter (in msec)
	virtual Time& update(int64_t time) { _time = time; return *this; }
	virtual Time& operator=(int64_t time) { _time = time; return *this; }

	/// \brief add �sec to the Mona time instance
	virtual Time& operator+= (int64_t time) { _time += time; return *this; }
	
	/// \brief reduce the Mona time instance (in �sec)
	virtual Time& operator-= (int64_t time) { _time -= time; return *this; }

	static int64_t Now() {
#if (_WIN32_WINNT >= 0x0600)
		static int64_t Ref = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - GetTickCount64();
		return Ref + GetTickCount64(); // GetTickCount64 performance x10 comparing with std::chrono::steady_clock::now()
#else
		static int64_t Ref = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		return Ref + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
	}

protected:
	virtual int64_t time() const { return _time; }
private:

	int64_t		 _time; // time en milliseconds with as reference 1/1/1970

};

} // namespace Mona

