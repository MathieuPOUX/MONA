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
#include "Mona/Timing/Timezone.h"
#include "Mona/Format/Format.h"
#include "inttypes.h"

namespace Mona {

struct Exception;

struct Date : Time, virtual Object {
	struct Format : Mona::Format<Format> {
		Format(const Date& date, const char* fmt = Date::FORMAT_ISO8601) :
			fmt(fmt), date(date) {
		}
		const char* const fmt;
		const Date& date;

		template<typename OutType>
		void stringify(OutType& out) const {

			char buffer[32];
			uint32_t formatSize = strlen(fmt);
			uint32_t iFormat(0);

			while (iFormat < formatSize) {
				char c(fmt[iFormat++]);
				if (c != '%') {
					if (c != '[' && c != ']')
						out.append(&c, 1);
					continue;
				}

				if (iFormat == formatSize)
					break;

				switch (c = fmt[iFormat++]) {
				case 'w': out.append(_WeekDayNames[date.weekDay()], 3); break;
				case 'W': { const char* day(_WeekDayNames[date.weekDay()]); out.append(day, strlen(day)); break; }
				case 'b': out.append(_MonthNames[date.month() - 1], 3); break;
				case 'B': { const char* month(_MonthNames[date.month() - 1]); out.append(month, strlen(month)); break; }
				case 'd': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", date.day())); break;
				case 'e': out.append(buffer, snprintf(buffer, sizeof(buffer), "%u", date.day())); break;
				case 'f': out.append(buffer, snprintf(buffer, sizeof(buffer), "%2d", date.day())); break;
				case 'm': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", date.month())); break;
				case 'n': out.append(buffer, snprintf(buffer, sizeof(buffer), "%u", date.month())); break;
				case 'o': out.append(buffer, snprintf(buffer, sizeof(buffer), "%2d", date.month())); break;
				case 'y': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", date.year() % 100)); break;
				case 'Y': out.append(buffer, snprintf(buffer, sizeof(buffer), "%04d", date.year())); break;
				case 'H': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", date.hour())); break;
				case 'h': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", (date.hour() < 1 ? 12 : (date.hour() > 12 ? (date.hour() - 12) : date.hour())))); break;
				case 'a': out.append((date.hour() < 12) ? "am" : "pm", 2); break;
				case 'A': out.append((date.hour() < 12) ? "AM" : "PM", 2); break;
				case 'M': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", date.minute())); break;
				case 'S': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", date.second())); break;
				case 's': out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", date.second()));
					out.append(EXPC("."));
				case 'F':
				case 'i': out.append(buffer, snprintf(buffer, sizeof(buffer), "%03d", date.millisecond())); break;
				case 'c': out.append(buffer, snprintf(buffer, sizeof(buffer), "%u", date.millisecond() / 100)); break;
				case 'z': Timezone::Format(date.isGMT() ? Timezone::GMT : date.offset(), out); break;
				case 'Z': Timezone::Format(date.isGMT() ? Timezone::GMT : date.offset(), out, false); break;
				case 't':
				case 'T': {
					if (iFormat == formatSize)
						break;
					uint32_t factor(1);
					switch (tolower(fmt[iFormat++])) {
					case 'h':
						factor = 3600000;
						break;
					case 'm':
						factor = 60000;
						break;
					case 's':
						factor = 1000;
						break;
					}
					out.append(buffer, snprintf(buffer, sizeof(buffer), "%02" PRIu64, uint64_t(date.time() / factor)));
					break;
				}
				default: out.append(&c, 1);
				}
			}
		}
	};

	static const char* FORMAT_ISO8601; 				/// 2005-01-01T12:00:00+01:00 | 2005-01-01T11:00:00Z
	static const char* FORMAT_ISO8601_FRAC;			/// 2005-01-01T12:00:00.000000+01:00 | 2005-01-01T11:00:00.000000Z
	static const char* FORMAT_ISO8601_SHORT; 		/// 20050101T120000+01:00 | 20050101T110000Z
	static const char* FORMAT_ISO8601_SHORT_FRAC;	/// 20050101T120000.000000+01:00 | 20050101T110000.000000Z
	static const char* FORMAT_RFC822;				/// Sat, 1 Jan 05 12:00:00 +0100 | Sat, 1 Jan 05 11:00:00 GMT
	static const char* FORMAT_RFC1123;				/// Sat, 1 Jan 2005 12:00:00 +0100 | Sat, 1 Jan 2005 11:00:00 GMT
	static const char* FORMAT_HTTP;					/// Sat, 01 Jan 2005 12:00:00 +0100 | Sat, 01 Jan 2005 11:00:00 GMT
	static const char* FORMAT_RFC850;				/// Saturday, 1-Jan-05 12:00:00 +0100 | Saturday, 1-Jan-05 11:00:00 GMT
	static const char* FORMAT_RFC1036;				/// Saturday, 1 Jan 05 12:00:00 +0100 | Saturday, 1 Jan 05 11:00:00 GMT
	static const char* FORMAT_ASCTIME;				/// Sat Jan  1 12:00:00 2005
	static const char* FORMAT_SORTABLE;				/// 2005-01-01 12:00:00

	static bool  IsLeapYear(int32_t year) { return (year % 400 == 0) || (!(year & 3) && year % 100); }

	// build a NOW date, not initialized (is null)
	// /!\ Keep 'Type' to avoid confusion with "build from time" constructor, if a explicit int32_t offset is to set, use Date::setOffset or "build from time" contructor
	Date(Timezone::Type offset=Timezone::LOCAL) : _isDST(false),_year(0), _month(0), _day(0), _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(false), _offset((int32_t)offset),_isLocal(true) {}
	
	// build from time
	Date(int64_t time, int32_t offset= Timezone::LOCAL) : _isDST(false),_year(0), _month(0), _day(0),  _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(false), _offset(offset),_isLocal(true), Time(time) {}

	// build from other  date
	explicit Date(const Date& other) : Time((Time&)other), _isDST(other._isDST),_year(other._year), _month(other._month), _day(other._day),  _weekDay(other._weekDay),_hour(other._hour), _minute(other._minute), _second(other._second), _millisecond(other._millisecond), _changed(other._changed), _offset(other._offset),_isLocal(other._isLocal) {
	}
	
	// build from date
	explicit Date(int32_t year, uint8_t month, uint8_t day, int32_t offset= Timezone::LOCAL) : _isDST(false),_year(0), _month(0), _day(0), _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(true), _offset(0),_isLocal(true), Time(0) {
		update(year,month,day,offset);
	}

	// build from clock
	explicit Date(uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond=0) : _isDST(false),_year(0), _month(1), _day(1), _weekDay(4),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(false), _offset(0),_isLocal(false), Time(0) {
		setClock(hour,minute,second,millisecond);
	}

	// build from date+clock
	explicit Date(int32_t year, uint8_t month, uint8_t day, uint8_t hour=0, uint8_t minute=0, uint8_t second=0, uint16_t millisecond=0, int32_t offset= Timezone::LOCAL) : _isDST(false),_year(0), _month(0), _day(0), _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(true), _offset(0),_isLocal(true), Time(0) {
		update(year,month,day,hour,minute,second,millisecond,offset);
	}

	 // now
	Date& update() { return update(Time::Now()); }
	// /!\ Keep 'Type' to avoid confusion with 'update(int64_t time)'
	Date& update(Timezone::Type offset) { return update(Time::Now(),offset); }

	// from other date
	Date& update(const Date& date);

	// from time
	Date& update(int64_t time) { return update(time, _isLocal ? int32_t(Timezone::LOCAL) : _offset); }
	Date& update(int64_t time,int32_t offset);

	// from date
	Date& update(int32_t year, uint8_t month, uint8_t day);
	Date& update(int32_t year, uint8_t month, uint8_t day, int32_t offset) { update(year, month, day); setOffset(offset); return *this; }

	// from date+clock
	Date& update(int32_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond = 0);
	Date& update(int32_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond, int32_t offset) { update(year, month, day, hour, minute, second,millisecond); setOffset(offset); return *this; }

	/// from string
	bool update(Exception& ex, const char* value, const char* format = NULL) { return update(ex, value, std::string::npos, format); }
	bool update(Exception& ex, const char* value, std::size_t count, const char* format = NULL);
	bool update(Exception& ex, const std::string& value, const char* format = NULL) { return update(ex, value.data(), format); }

	Date& operator=(int64_t time) { update(time); return *this; }
	Date& operator=(const Date& date) { update(date); return *this; }
	Date& operator+= (int64_t time) { update(this->time()+time); return *this; }
	Date& operator-= (int64_t time) { update(this->time()-time); return *this; }

	// to time
	int64_t time() const;

	/// GETTERS
	// date
	int32_t	year() const			{ if (_day == 0) init(); return _year; }
	uint8_t	month() const			{ if (_day == 0) init(); return _month; }
	uint8_t	day() const				{ if (_day == 0) init(); return _day; }
	uint8_t	weekDay() const;
	uint16_t	yearDay() const;
	// clock
	uint32_t  clock() const			{ if (_day == 0) init(); return _hour*3600000L + _minute*60000L + _second*1000L + _millisecond; }
	uint8_t	hour() const			{ if (_day == 0) init(); return _hour; }
	uint8_t	minute() const			{ if (_day == 0) init(); return _minute; }
	uint8_t	second() const			{ if (_day == 0) init(); return _second; }
	uint16_t	millisecond() const		{ if (_day == 0) init(); return _millisecond; }
	// offset
	int32_t	offset() const;
	bool	isGMT() const			{ if (_day == 0) init(); return offset()==0 && !_isLocal; }
	bool	isDST() const			{ offset(); /* <= allow to refresh _isDST */ return _isDST; }

	/// SETTERS
	// date
	void	setYear(int32_t year);
	void	setMonth(uint8_t month);
	void	setDay(uint8_t day);
	void	setWeekDay(uint8_t weekDay);
	void	setYearDay(uint16_t yearDay);

	// clock
	void	setClock(uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond=0);
	void	setClock(uint32_t clock);
	void	setHour(uint8_t hour);
	void	setMinute(uint8_t minute);
	void	setSecond(uint8_t second);
	void	setMillisecond(uint16_t millisecond);
	// offset
	void	setOffset(int32_t offset);


private:
	void  init() const { _day = 1; ((Date*)this)->update(Time::time(), _offset); }
	void  computeWeekDay(int64_t days);
	bool  parseAuto(Exception& ex, const char* data, std::size_t count);

	int32_t			_year;
	uint8_t			_month; // 1 to 12
	mutable uint8_t	_day; // 1 to 31
	mutable uint8_t	_weekDay; // 0 to 6 (sunday=0, monday=1) + 7 (unknown)
	uint8_t			_hour; // 0 to 23
	uint8_t			_minute;  // 0 to 59
	uint8_t			_second;	// 0 to 59
	uint16_t			_millisecond; // 0 to 999
	mutable int32_t	_offset; // gmt offset
	mutable bool	_isDST; // means that the offset is a Daylight Saving Time offset
	mutable bool	_isLocal; // just used when offset is on the special Local value!

	mutable bool	_changed; // indicate that date information has changed, we have to refresh time value

	static const char*    _WeekDayNames[];
	static const char*    _MonthNames[];
};


} // namespace Mona

