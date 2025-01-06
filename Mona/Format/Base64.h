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

const char _B64Table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char _B64urlTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

const char _ReverseB64Table[128] = {
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};


struct Base64 : Format<Base64> {
	Base64(const Packet& packet, bool toURL = false) :
		packet(packet),
		toURL(toURL) {
	}
	const Packet	packet;
	const bool 		toURL;

	template<typename OutType>
	void stringify(OutType& out) const {
		uint32_t accumulator(out.size()), bits(0);

		out.resize(accumulator+uint32_t(std::ceil(packet.size()/3.0)*4));

		char* current(static_cast<char*>(out.data()));
		if (!current) {
			// to expect null writer
            return;
        }
        const char*	end(current+out.size());
		current += accumulator;

		const char* data = packet.data();
		const char* endData = data + packet.size();
		const auto& table = toURL ? _B64urlTable : _B64Table;

		accumulator = 0;
		while(data<endData) {
			accumulator = (accumulator << 8) | (*data++ & 0xFFu);
			bits += 8;
			while (bits >= 6) {
				bits -= 6;
				*current++ = table[(accumulator >> bits) & 0x3Fu];
			}
		}
		if (bits > 0) { // Any trailing bits that are missing.
			accumulator <<= 6 - bits;
			*current++ = table[accumulator & 0x3Fu];
		}
		// padding with '='
		while (current<end) { 
			*current++ = '=';
		}
	}


	template <typename BufferType>
	static bool parse(BufferType& buffer) { return parse(buffer, buffer.data(), buffer.size()); }
	template <typename BufferType>
	static bool parse(BufferType& buffer, const std::string& value, bool append=false) { return parse(buffer, value.data(), value.size(), append); }
	template <typename BufferType>
	static bool parse(BufferType& buffer, const char* data, uint32_t size, bool append=false) {
		if (!buffer.data())
			return false; // to expect null writer 

		uint32_t bits(0), oldSize(append ? buffer.size() : 0);
		uint32_t accumulator(oldSize + uint32_t(ceil(size / 4.0) * 3));
		const char* end = data+size;

		if (buffer.size()<accumulator)
			buffer.resize(accumulator); // maximum size!
		char* out =  (char*)buffer.data() + oldSize;

		accumulator = size = 0;

		while(data<end) {
			uint8_t c = *data++;
			if (isspace(c) || c == '=')
				continue;

			if ((c > 127) || (_ReverseB64Table[c] > 63)) {
				// reset the oldSize
				buffer.resize(oldSize);
				return false;
			}
		
			accumulator = (accumulator << 6) | _ReverseB64Table[c];
			bits += 6;
			if (bits >= 8) {
				bits -= 8;
				size++;
				*out++ = ((accumulator >> bits) & 0xFFu);
			}
		}
		buffer.resize(oldSize+size);
		return true;
	}
};


} // namespace Mona
