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
#include "Mona/Format/BinaryReader.h"
#include <mutex>
#include <vector>
#include <map>

#if defined(_WIN32)
struct _SYSTEMTIME;
#endif

namespace Mona {

struct Date;
struct Timezone : virtual Object {
	enum Type {
		GMT = (int32_t)0x7FFFFFFF, /// Special value for offset (int32_t minimum)
		LOCAL = (int32_t)0x80000000 /// Special value for offset(int32_t maximum)
	};

	// Local
	static int32_t				Offset(bool dst=false) { return dst ? GetTimezone()._dstOffset : GetTimezone()._offset; } // timezone in ms
	static int32_t				Offset(const Date& date, bool& isDST) { return GetTimezone().offset(date,isDST); }
	static const std::string&	Name()   { return GetTimezone()._name; }

	// Others
	static int32_t		Offset(const std::string& code) { bool isDST(false);  return Offset(code.data(), isDST); }
	static int32_t		Offset(const char* code) { bool isDST(false);  return Offset(code, isDST); }
	static int32_t		Offset(const std::string& code,bool& isDST)  { return Offset(code.data(), isDST); }
	static int32_t		Offset(const char* code, bool& isDST);

	template<typename OutType>
	static OutType& Format(int32_t offset, OutType& out, bool bISO=true) {
		if (offset == GMT)
			return (OutType&)(bISO ? out.append(EXPC("Z")) : out.append(EXPC("GMT")));
		char buffer[32];
		out.append((offset < 0) ? "-" : "+", 1);
		uint32_t value = abs(offset);
		out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", value / 3600000));
		if (bISO)
			out.append(EXPC(":"));
		return (OutType&)out.append(buffer, snprintf(buffer, sizeof(buffer), "%02d", (value % 3600000) / 60000));
	}
private:
	static Timezone& GetTimezone() { static Timezone TZ; return TZ; }
	struct TransitionRule {
		enum Type {
			TYPE_NONE,
			TYPE_ABSOLUTE,
			TYPE_JULIAN,
			TYPE_RELATIVE
		};
		TransitionRule() : type(TYPE_NONE),month(0),day(0),week(0),clock(0) {}
		uint8_t	month; // 1 to 12
		uint8_t	week; // 1 to 5
		uint16_t	day; // 0 to 6 if type==RELATIVE, 0 to 365 if type==ABSOLUTE (leap counted), 1 to 365 if type==JULIAN (no leap counted)
		uint32_t	clock; // clock in ms
		Type	type;
		operator bool() const { return type != TYPE_NONE; }
	};

	struct Transition : public virtual Object {
		Transition() : offset(0),isDST(false) {}
		Transition& set(const Transition& other) { (int32_t&)this->offset = other.offset; (bool&)this->isDST = other.isDST; return *this; }
		Transition& set(int32_t offset, bool isDST) { (int32_t&)this->offset = offset; (bool&)this->isDST = isDST; return *this; }
		const int32_t  offset;
		const bool   isDST;
	};

	
	Timezone();
	
	int32_t  offset(const Date& date, bool& isDST);
	int64_t  ruleToTime(int32_t year, bool isDST, const TransitionRule& rule);

#if defined(_WIN32)
	void  fillDefaultTransitionRule(const _SYSTEMTIME& date, bool isDST,Timezone::TransitionRule& rule);
#endif

	int32_t						_offset;
	int32_t						_dstOffset;
	std::string					_name;
	TransitionRule				_startDST;
	TransitionRule				_endDST;
	std::map<int64_t, Transition>	_transitions;


	/// TZ Database
	static bool  ParseTZRule(std::string::const_iterator& it, const std::string::const_iterator& end, TransitionRule& rule);
	static int32_t ParseTZOffset(std::string::const_iterator& it, const std::string::const_iterator& end, int32_t defaultValue);


	bool  readTZDatabase(const std::string& path);
	
	template <typename Type>
	bool readTZData(BinaryReader& reader) {
		if (reader.available() < 39)
			return false;

		reader.next(15); // skip header

		uint32_t gmtSize(reader.read32());
		uint32_t stdSize(reader.read32());
		uint32_t leapSize(reader.read32());
		uint32_t timeSize(reader.read32());
		uint32_t typeSize(reader.read32());
		uint32_t charSize(reader.read32());
		uint32_t i(0);

		uint32_t position(reader.position());
		reader.next(timeSize*(sizeof(Type)+1));

		// Types
		struct TransitionType {
			TransitionType() : offset(7200000), isDST(false) {}
			int32_t offset;
			bool isDST;
		};

		if (reader.available() < (typeSize*6))
			return false;

		std::vector<TransitionType> types(typeSize);
		for (i = 0; i < typeSize; ++i) {
			types[i].offset = reader.read32();
			types[i].isDST = reader.read8()!=0;
			reader.next(1); // skip abbr (pointer to name)
		}

		 // Skip charSize
		 reader.next(charSize);
	 
		// Times (sorted in ascending time order)
		uint32_t end(reader.position());
		reader.reset(position);
		const char* type = reader.current() + (timeSize*sizeof(Type));
		while (timeSize--) {
			uint8_t id(*type++);
			if (id >= types.size()) {
				reader.next(sizeof(Type));
				continue;
			}
			int64_t time(sizeof(Type) == 4 ? (int32_t)reader.read32() : reader.read64());

			// no "emplace" to override a possible transition existing with this time
			_transitions[time*1000].set(types[id].offset*1000,types[id].isDST);
		}
		reader.reset(end);

		// Skip leap seconds
		reader.next(leapSize*2*sizeof(Type));

		// Skip stdSize and gmtSize
		reader.next(stdSize);
		reader.next(gmtSize);

		/*
		// display result
		for (const Transition& transition : _transitions) {
			Date date;
			TimeToDate(transition, date);
			printf("%d/%hhu/%hhu @ %hhu:%hhu:%hhu,%hu =>",date.year,date.month,date.day,date.hour,date.minute,date.second,date.millisecond);
			printf(" %d | %s\n", transition.gmtOffset,transition.isDST ? "DST" : "STD");
		}*/
		return true;
	}

};


} // namespace Mona

