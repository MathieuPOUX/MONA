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

#include "Mona/Util/Util.h"
#if !defined(_WIN32)
#include <sys/times.h>
	#include <unistd.h>
	#include <sys/syscall.h>
	extern "C" char **environ;
#endif

using namespace std;

namespace Mona {

const uint8_t Util::UInt8Generators[] = {
	 0,   1,  1,  2,  1,  3,  5,  4,  5,  7,  7,  7,  7,  8,  9,  8,
	 11, 11, 11, 12, 11, 13, 15, 14, 17, 14, 15, 17, 17, 18, 19, 19,
	 21, 20, 21, 22, 23, 23, 23, 23, 27, 25, 25, 27, 27, 28, 27, 29,
	 31, 30, 31, 32, 31, 33, 31, 34, 37, 35, 37, 36, 37, 38, 37, 40,
	 41, 41, 41, 41, 41, 43, 43, 44, 43, 45, 47, 46, 47, 48, 47, 49,
	 49, 50, 51, 51, 53, 53, 53, 55, 53, 55, 53, 55, 57, 56, 57, 59,
	 59, 60, 61, 61, 63, 62, 61, 64, 63, 64, 67, 66, 67, 67, 69, 70,
	 69, 70, 71, 71, 73, 71, 73, 74, 73, 75, 75, 76, 77, 77, 79, 78,
	 79, 80, 79, 81, 83, 82, 83, 83, 83, 85, 85, 86, 87, 86, 89, 87,
	 89, 91, 89, 92, 91, 92, 91, 93, 93, 95, 95, 96, 95, 97, 99, 98,
	 99,100,101,101,101,103,103,103,103,103,103,106,105,107,109,108,
	109,109,109,111,109,112,111,113,113,114,115,116,115,118,117,118,
	119,119,121,121,121,122,125,123,123,124,125,125,125,127,127,128,
	129,129,131,130,131,133,131,133,133,134,135,134,137,137,137,138,
	137,139,141,140,139,142,141,142,143,144,145,144,147,146,149,148,
	149,149,151,149,151,151,151,153,153,154,153,155,157,156,157,158
};

const Parameters& Util::Environment() {
	static const struct Environment : Parameters, virtual Object {
		Environment() {
			const char* line(*environ);
			for (uint32_t i = 0; (line = *(environ + i)); ++i) {
				const char* value = strchr(line, '=');
				if (value) {
					setString(string(line, (value++) - line), value);
				} else
					setString(line, "");
			}
		}
	} Environment;
	return Environment;
}

uint64_t Util::Random() {
	// flip+process Id to make variate most signifiant bit on different computer AND on same machine
	static uint64_t A = Bytes::Flip64(Process::Id()) | Bytes::Flip64(Time::Now());
#if defined(_WIN32)
#if (_WIN32_WINNT >= 0x0600)
	static uint64_t B = Bytes::Flip64(Process::Id()) | Bytes::Flip64(GetTickCount64());
#else
	static uint64_t B = Bytes::Flip64(Process::Id()) | Bytes::Flip32(GetTickCount());
#endif
#else
	static tms TMS;
	static uint64_t B = Bytes::Flip64(Process::Id()) | Bytes::Flip32(times(&TMS));
#endif
	// xorshift128plus = faster algo able to pass BigCrush test
	uint64_t x = A;
	uint64_t const y = B;
	A = y; // not protect A and B to go more faster (and useless on random value...)
	x ^= x << 23; // a
	B = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c
	return B + y; // cast gives modulo here!
}

void Util::Dump(string& buffer, const char* data, uint32_t size) {
	uint8_t b;
	uint32_t c(0);
	buffer.resize((uint32_t)ceil((double)size / 16) * 67, false);

	const char * end = data + size;
	char*		 out = buffer.data();

	while (data < end) {
		c = 0;
		*out++ = '\t';
		while ((c < 16) && (data < end)) {
			b = *data++;
			snprintf(out, 4, "%X%X ", b >> 4, b & 0x0f);
			out += 3;
			++c;
		}
		data -= c;
		while (c++ < 16) {
			memcpy(out, "   \0", 4);
			out += 3;
		}
		*out++ = ' ';
		c = 0;
		while ((c < 16) && (data < end)) {
			b = *data++;
			if (b > 31)
				*out++ = b;
			else
				*out++ = '.';
			++c;
		}
		while (c++ < 16)
			*out++ = ' ';
		*out++ = '\n';
	}
}



} // namespace Mona
