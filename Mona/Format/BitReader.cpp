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

#include "Mona/Format/BitReader.h"

using namespace std;

namespace Mona {

BitReader BitReader::Null(NULL,0);

bool BitReader::read() {
	if (_current == _end)
		return false;
	bool result = (*_current & (0x80u >> _bit++)) ? true : false;
	if (_bit == 8) {
		_bit = 0;
		++_current;
	}
	return result;
}


uint64_t BitReader::next(uint64_t count) {
	uint64_t gotten(0);
	while (_current != _end && count--) {
		++gotten;
		if (++_bit == 8) {
			_bit = 0;
			++_current;
		}
	}
	return gotten;
}

void BitReader::reset(uint64_t position) {
	uint32_t bytes = uint32_t(position / 8);
	_current = _data + (bytes>_size ? _size : bytes);
	_bit = position % 8;
}

uint64_t BitReader::shrink(uint64_t available) {
	uint64_t rest = this->available();
	if (available > rest)
		return rest;
	_end = _current + (available / 8);
	_size = _end - _data;
	_bit = available % 8;
	return available;
}

} // namespace Mona
