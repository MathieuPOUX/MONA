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
#include "Mona/Format/Format.h"
#include "Mona/Memory/Packet.h"

namespace Mona {

typedef uint8_t HEXA_OPTIONS;
enum {
	HEXA_CPP = 1,
	HEXA_TRIM_LEFT = 2,
	HEXA_UPPER_CASE = 4
};

struct Hexa : Format<Hexa> {
	Hexa(const Packet& packet, HEXA_OPTIONS options = 0) : packet(packet), options(options) {

	}
	const Packet		packet;
	const HEXA_OPTIONS	options;

	template<typename OutType>
	void stringify(OutType& out) const {
		const char* end = packet.data() + packet.size();
		const char* data = packet.data();
		bool skipLeft(false);
		if (options&HEXA_TRIM_LEFT) {
			while (data<end) {
				if ((U(*data) >> 4)>0)
					break;
				if (((*data) & 0x0F) > 0) {
					skipLeft = true;
					break;
				}
				++data;
			}
		}
		char ref = options&HEXA_UPPER_CASE ? '7' : 'W';
		char value;
		while (data<end) {
			if (options & HEXA_CPP) {
				out.append(EXPC("\\x"));
			}
			value = U(*data) >> 4;
			if (!skipLeft) {
				value += value > 9 ? ref : '0';
				out.append(&value, 1);
			} else {
				skipLeft = false;
			}
			value = (*data++) & 0x0F;
			value += value > 9 ? ref : '0';
			out.append(&value, 1);
		}
	}

	

	template <typename BufferType>
	static BufferType& parse(BufferType& buffer, bool append = false) { return parse(buffer.data(), buffer.size(), buffer, append); }
	template <typename BufferType>
	static BufferType& parse(const std::string& value, BufferType& buffer, bool append = false) { return parse(value.data(), value.size(), buffer, append); }
	template <typename BufferType>
	static BufferType& parse(const char* value, std::size_t size, BufferType& buffer, bool append = false) {
		char* out;
		uint32_t count = size / 2;
		if (size & 1)
			++count;
		if (append) {
			buffer.resize(buffer.size() + count);
			out = (char*)buffer.data() + buffer.size() - count;
		} else {
			if (count>buffer.size())
				buffer.resize(count);
			out = (char*)buffer.data();
		}
		while (size-->0) {
			char left = toupper(*value++);
			char right = size-- ? toupper(*value++) : '0';
			*out++ = ((left - (left <= '9' ? '0' : '7')) << 4) | ((right - (right <= '9' ? '0' : '7')) & 0x0F);
		}
		if(!append)
			buffer.resize(count);
		return buffer;
	}
};


} // namespace Mona
