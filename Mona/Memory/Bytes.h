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

namespace Mona {

/**
 * @ingroup Memory
 * @brief Manipulate Bytes
 */
struct Bytes : virtual Object {
	bool				empty() const { return !size(); }

	virtual const char*	data() const = 0;
	virtual size_t		size() const = 0;
	const char*			end() const { return data() + size(); }

	/**
	Return value at index i in a uint8_t bitwise friendly format (and convertible to char without warning) */
	uint8_t				operator[](uint32_t i) const {
		DEBUG_ASSERT(data() && i<size())
			return data()[i];
	}

	/**
	Force a string conversion in building a std::string with this value in argument, keep if explicit to prefer data()/size() access less expensive */
	explicit operator std::string() const { return std::string(data(), size()); }

	 enum Order {
        ORDER_BIG_ENDIAN=1, // network order!
		ORDER_LITTLE_ENDIAN,
		ORDER_NETWORK=ORDER_BIG_ENDIAN,
#if (__BIG_ENDIAN__)
		ORDER_NATIVE=ORDER_BIG_ENDIAN
#else
		ORDER_NATIVE=ORDER_LITTLE_ENDIAN
#endif
	 };


	template<typename ValueType>
	static uint8_t Get7BitSize(typename std::make_unsigned<ValueType>::type value, uint8_t bytes = sizeof(ValueType) + 1) {
		uint8_t result(1);
		while ((value >>= 7) && result++<bytes);
		return result-(value ? 1 : 0); // 8th bit
	}

	
	static uint16_t Flip16(uint16_t value) { return ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00); }
	static uint32_t Flip24(uint32_t value) { return ((value >> 16) & 0x000000FF) | (value & 0x0000FF00) | ((value << 16) & 0x00FF0000); }
	static uint32_t Flip32(uint32_t value) { return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000); }
	static uint64_t Flip64(uint64_t value) { uint32_t hi = uint32_t(value >> 32); uint32_t lo = uint32_t(value & 0xFFFFFFFF); return uint64_t(Flip32(hi)) | (uint64_t(Flip32(lo)) << 32); }
	template<typename NumType>
	static NumType&& Flip(NumType&& value) { uint8_t *lo = (uint8_t*)&value; uint8_t *hi = (uint8_t*)&value + sizeof(value) - 1; uint8_t swap; while (lo < hi) { swap = *lo; *lo++ = *hi; *hi-- = swap; } return value; }

	static uint16_t From16Network(uint16_t value) { return From16BigEndian(value); }
	static uint32_t From24Network(uint32_t value) { return From24BigEndian(value); }
	static uint32_t From32Network(uint32_t value) { return From32BigEndian(value); }
	static uint64_t From64Network(uint64_t value) { return From64BigEndian(value); }
	template<typename NumType>
	static NumType&& FromNetwork(NumType&& value) { return FromBigEndian(value); }

	static uint16_t To16Network(uint16_t value) { return From16BigEndian(value); }
	static uint32_t To24Network(uint32_t value) { return From24BigEndian(value); }
	static uint32_t To32Network(uint32_t value) { return From32BigEndian(value); }
	static uint64_t To64Network(uint64_t value) { return From64BigEndian(value); }
	template<typename NumType>
	static NumType&& ToNetwork(NumType&& value) { return FromBigEndian(value); }

	static uint16_t From16BigEndian(uint16_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip16(value); }
	static uint32_t From24BigEndian(uint32_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip24(value); }
	static uint32_t From32BigEndian(uint32_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip32(value); }
	static uint64_t From64BigEndian(uint64_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip64(value); }
	template<typename NumType>
	static NumType&& FromBigEndian(NumType&& value) { return ORDER_NATIVE == ORDER_BIG_ENDIAN ? value : Flip(value); }

	static uint16_t To16BigEndian(uint16_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip16(value); }
	static uint32_t To24BigEndian(uint32_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip24(value); }
	static uint32_t To32BigEndian(uint32_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip32(value); }
	static uint64_t To64BigEndian(uint64_t value) { return ORDER_NATIVE==ORDER_BIG_ENDIAN ? value : Flip64(value); }
	template<typename NumType>
	static NumType&& ToBigEndian(NumType&& value) { return ORDER_NATIVE == ORDER_BIG_ENDIAN ? value : Flip(value); }

	static uint16_t From16LittleEndian(uint16_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip16(value); }
	static uint32_t From24LittleEndian(uint32_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip24(value); }
	static uint32_t From32LittleEndian(uint32_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip32(value); }
	static uint64_t From64LittleEndian(uint64_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip64(value); }
	template<typename NumType>
	static NumType&& FromLittleEndian(NumType&& value) { return ORDER_NATIVE == ORDER_LITTLE_ENDIAN ? value : Flip(value); }

	static uint16_t To16LittleEndian(uint16_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip16(value); }
	static uint32_t To24LittleEndian(uint32_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip24(value); }
	static uint32_t To32LittleEndian(uint32_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip32(value); }
	static uint64_t To64LittleEndian(uint64_t value) { return ORDER_NATIVE==ORDER_LITTLE_ENDIAN ? value : Flip64(value); }
	template<typename NumType>
	static NumType&& ToLittleEndian(NumType&& value) { return ORDER_NATIVE == ORDER_LITTLE_ENDIAN ? value : Flip(value); }
};


} // namespace Mona
