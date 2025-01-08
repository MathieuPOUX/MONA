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
#include "Mona/Memory/Bytes.h"

namespace Mona {


struct BitReader : Bytes, virtual Object {

	BitReader(const char* data, uint32_t size) : _bit(0), _current(data), _end(data+size), _data(data), _size(size) {}

	virtual bool read();

	template<typename ResultType>
	ResultType read(uint8_t count = (sizeof(ResultType) * 8)) {
		typename std::make_unsigned<ResultType>::type result(0);
		while (_current != _end && count--) {
			result <<= 1;
			if (!read())
				continue;
			if (count >= (sizeof(ResultType) * 8))
				return std::numeric_limits<ResultType>::max(); // max reachs!
			result |= 1;
		}
		return result;
	}

	uint64_t	position() const { return (_current-_data)*8 + _bit; }
	virtual uint64_t	next(uint64_t count = 1);
	void		reset(uint64_t position = 0);
	uint64_t	shrink(uint64_t available);

	uint64_t	available() const { return (_end -_current)*8 - _bit; }

	// beware, data() can be null
	const char*		data() const override { return _data; }
	size_t			size() const override { return _size; }

	
	static BitReader Null;
protected:

	const char*		_data;
	const char*		_end;
	const char*		_current;
	size_t			_size;
	uint8_t			_bit;
};


} // namespace Mona
