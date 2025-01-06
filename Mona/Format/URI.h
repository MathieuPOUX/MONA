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

struct URI : Format<URI>, Packet {
	template <typename ...Args>
	URI(Args&&... args) : Packet(std::forward<Args>(args)...) {
	}
	
	template<typename OutType>
	void stringify(OutType& out) const {
		std::size_t size = this->size();
		const char* value = this->data();
		while (STR_AVAILABLE(value, size)) {
			char c = *value;
			// https://en.wikipedia.org/wiki/Percent-encoding#Types_of_URI_characters
			if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
				out.append(&c, 1);
			else
				String::append(out, '%', String::Format<uint8_t>("%2X", c)); // uint8_t to get a right value!
			STR_NEXT(value, size);
		}
	}

	
	template <typename BufferType>
	static bool parse(char hi, char lo, BufferType& buffer) {
		if (!isxdigit(hi) || !isxdigit(lo)) {
			String::append(buffer, '%', hi, lo);
			return false;
		}
		hi = ((hi - (hi <= '9' ? '0' : '7')) << 4) | ((lo - (lo <= '9' ? '0' : '7')) & 0x0F);
		buffer.append(&hi, 1);
		return true;
	}
	template <typename BufferType>
	static bool parse(const std::string& value, BufferType& buffer) { return parse(value.data(), value.size(), buffer); }
	template <typename BufferType>
	static bool parse(const char* data, BufferType& buffer) { return parse(data, std::string::npos, buffer); }
	template <typename BufferType>
	static bool parse(const char* data, std::size_t size, BufferType& buffer) {
		bool oneDone = false;
		int8_t hi = 0;
		while (STR_AVAILABLE(data, size)) {
			if (*data == '%') {
				if (hi) {
					buffer.append(EXPC("%"));
					if (hi != -1)
						buffer.append(&hi, 1);
				}
				hi = -1;
			} else if (hi == -1 && *data != -1)
				hi = *data;
			else if (hi && parse(hi, *data, buffer))
				oneDone = true;
			STR_NEXT(data, size);
		};
		return oneDone;
	}

};


} // namespace Mona
