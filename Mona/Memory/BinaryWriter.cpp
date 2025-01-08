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

#include "Mona/Memory/BinaryWriter.h"
#include "Mona/Util/Util.h"


using namespace std;


namespace Mona {

BinaryWriter::BinaryWriter(char* buffer, size_t size, Bytes::Order byteOrder) :
	_pos(0), _view(buffer, size), _flipBytes(byteOrder != Bytes::ORDER_NATIVE) {
}
BinaryWriter::BinaryWriter(string& buffer, Bytes::Order byteOrder) :
	_pos(buffer.size()), _pBuffer(&buffer), _flipBytes(byteOrder != Bytes::ORDER_NATIVE) {
}

BinaryWriter& BinaryWriter::resize(size_t size) {
	if (_pBuffer) {
		_pBuffer->resize(_pos + size);
	} else {
		if(size>_view.size()) {
			FATAL_ERROR("Static buffer exceeds maximum ", _view.size(), " bytes capacity");
		}
		_pos = size;
	}
	return self;
}

BinaryWriter& BinaryWriter::append(const void* data, size_t size) {
	memcpy(buffer(size), data, size);
	return self;
}

BinaryWriter& BinaryWriter::append(size_t count, char value) {
	memset(buffer(count), value, count);
	return self;
}

char* BinaryWriter::buffer(size_t size) {
	uint32_t oldSize(this->size());
	resize(oldSize+size);
	return STR data() + oldSize;
}

BinaryWriter& BinaryWriter::writeRandom(size_t count) {
	while(count--) {
		write8(Util::Random<uint8_t>());
	}
	return *this;
}

BinaryWriter& BinaryWriter::write16(uint16_t value) {
	if (_flipBytes)
		value = Bytes::Flip16(value);
	return append(&value, sizeof(value));
}

BinaryWriter& BinaryWriter::write24(uint32_t value) {
	if (_flipBytes)
		value = Bytes::Flip24(value);
	return append(&value, 3);
}

BinaryWriter& BinaryWriter::write32(uint32_t value) {
	if (_flipBytes)
		value = Bytes::Flip32(value);
	return append(&value, sizeof(value));
}

BinaryWriter& BinaryWriter::write64(uint64_t value) {
	if (_flipBytes)
		value = Bytes::Flip64(value);
	return append(&value, sizeof(value));
}

BinaryWriter& BinaryWriter::writeDouble(double value) {
	if (_flipBytes)
		value = Bytes::Flip(value);
	return append(&value, sizeof(value));
}
BinaryWriter& BinaryWriter::writeFloat(float value) {
	if (_flipBytes)
		value = Bytes::Flip(value);
	return append(&value, sizeof(value));
}

template<typename ValueType>
BinaryWriter& BinaryWriter::write7Bit(typename make_unsigned<ValueType>::type value) {
	if (is_signed<ValueType>::value) {
		bool isNegative = typename make_signed<ValueType>::type(value) < 0;
		value <<= 1; // reserve one bit for sign
		if (isNegative) {
			value = -typename make_signed<ValueType>::type(value);
			value |= 1;
		}
	}
	char byte = value & 0x7F;
    while (value >>= 7) {
      write8(0x80 | byte);
      byte = value & 0x7F;
    }
    return write8(byte);
}


template BinaryWriter& BinaryWriter::write7Bit<uint16_t>(make_unsigned<uint16_t>::type value);
template BinaryWriter& BinaryWriter::write7Bit<uint32_t>(make_unsigned<uint32_t>::type value);
template BinaryWriter& BinaryWriter::write7Bit<uint64_t>(make_unsigned<uint64_t>::type value);
template BinaryWriter& BinaryWriter::write7Bit<int16_t>(make_unsigned<int16_t>::type value);
template BinaryWriter& BinaryWriter::write7Bit<int32_t>(make_unsigned<int32_t>::type value);
template BinaryWriter& BinaryWriter::write7Bit<int64_t>(make_unsigned<int64_t>::type value);


} // namespace Mona
