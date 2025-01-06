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
#include "Mona/Util/Parameters.h"
#include "Mona/Threading/Process.h"
#include "Mona/Disk/Path.h"

namespace Mona {

/**
 * @ingroup Util
 * @brief General utility functions
 */
struct Util : virtual Static {

	template<typename MapType, typename KeyType, typename ValueType>
	static ValueType get(const MapType& map, const KeyType& key, const ValueType& defaultValue) {
		const auto& it = map.find(key);
		return (it != map.end()) ? it->second : defaultValue;
	}

	static void Dump(const char* data, uint32_t size, Buffer& buffer);

	static const Parameters& Environment();

	static uint64_t	Random();
	template<typename Type>
	static Type		Random() { return Type(Random()); } // cast gives the modulo!
	static void		Random(char* data, uint32_t size) { for (uint32_t i = 0; i < size; ++i) data[i] = char(Random()); }

	static const uint8_t UInt8Generators[];

	template<typename Type>
	struct Scoped {
		Scoped(Type& value, const Type& tempValue) : _value(value), _oldValue(value) { value = tempValue; }
		~Scoped() { release(); }
		operator const Type&() const { return _value; }
		Scoped& release() { _value = _oldValue; return self; }
	private:
		const Type _oldValue;
		Type& _value;
	};


	template<typename Type, typename ResultType = typename std::make_signed<Type>::type>
	static ResultType Distance(Type pt1, Type pt2) {
		ResultType result(pt2 - pt1);
		return Mona::abs(result) > std::ceil(std::numeric_limits<typename std::make_unsigned<ResultType>::type>::max() / 2.0) ? (pt1 - pt2) : result;
	}

	template<typename Type, typename ResultType = typename std::make_signed<Type>::type>
	static ResultType Distance(Type pt1, Type pt2, Type max, Type min = 0) {
		DEBUG_ASSERT(min <= pt1 && pt1 <= max);
		DEBUG_ASSERT(min <= pt2 && pt2 <= max);
		ResultType result(pt2 - pt1);
		max -= min;
		if (Mona::abs(result) <= std::ceil(max / 2.0))
			return result;
		return result>0 ? (result - max - 1) : (result + max + 1);
	}
	
	template<typename Type, typename TypeD>
	static Type AddDistance(Type pt, TypeD distance, Type max, Type min = 0) {
		DEBUG_ASSERT(min <= pt && pt <= max);
		typename std::make_unsigned<Type>::type delta;
		if (distance >= 0) { // move on the right
			while (typename std::make_unsigned<TypeD>::type(distance) > (delta = (max - pt))) {
				pt = min;
				distance -= delta + 1;
			}
			return pt + distance;
		}
		// move on the left (distance<0), TypeD is necessary signed!
		distance = -typename std::make_signed<TypeD>::type(distance);
		while (typename std::make_unsigned<TypeD>::type(distance) > (delta = (pt - min))) {
			pt = max;
			distance -= delta + 1;
		}
		return pt - distance;
	}

};

} // namespace Mona
