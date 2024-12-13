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

#include "Mona/Format/BinaryWriter.h"
#include "Mona/Util/Util.h"


using namespace std;


namespace Mona {

BinaryWriter::BinaryWriter(char* buffer, uint32_t size, Bytes::Order byteOrder) :
	_offset(0), _pBuffer(new Buffer(size, buffer)), _reference(false), _flipBytes(byteOrder != Bytes::ORDER_NATIVE) {
	_pBuffer->resize(0);
}
BinaryWriter::BinaryWriter(Buffer& buffer, Bytes::Order byteOrder) :
	_offset(buffer.size()), _pBuffer(&buffer), _reference(true), _flipBytes(byteOrder != Bytes::ORDER_NATIVE) {
}

BinaryWriter::~BinaryWriter() {
	if (!_reference)
		delete _pBuffer;
}

char* BinaryWriter::buffer(uint32_t size) {
	uint32_t oldSize(_pBuffer->size());
	_pBuffer->resize(oldSize+size);
	return _pBuffer->data() + oldSize;
}

BinaryWriter& BinaryWriter::writeRandom(uint32_t count) {
	while(count--)
		write8(Util::Random<uint8_t>());
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
BinaryWriter& BinaryWriter::write7Bit(typename make_unsigned<ValueType>::type value, uint8_t bytes) {
	if (!bytes)
		return self;
	if (is_signed<ValueType>::value) {
		bool isNegative = typename make_signed<ValueType>::type(value) < 0;
		value <<= 1; // reserve one bit for sign
		if (isNegative) {
			value = -typename make_signed<ValueType>::type(value);
			value |= 1;
		}
	}
	uint8_t bits = (bytes - 1) * 7 + 1;
	if (!(value >> (bits - 1))) {
		bits -= 8;
		while (!(value >> bits) && (bits -= 7));
	}
	while (bits>1) {
		write8(0x80 | uint8_t(value >> bits));
		bits -= 7;
	}
	return write8(value & (bits ? 0xFF : 0x7F));
}
template BinaryWriter& BinaryWriter::write7Bit<uint16_t>(make_unsigned<uint16_t>::type value, uint8_t bytes);
template BinaryWriter& BinaryWriter::write7Bit<uint32_t>(make_unsigned<uint32_t>::type value, uint8_t bytes);
template BinaryWriter& BinaryWriter::write7Bit<uint64_t>(make_unsigned<uint64_t>::type value, uint8_t bytes);
template BinaryWriter& BinaryWriter::write7Bit<int16_t>(make_unsigned<int16_t>::type value, uint8_t bytes);
template BinaryWriter& BinaryWriter::write7Bit<int32_t>(make_unsigned<int32_t>::type value, uint8_t bytes);
template BinaryWriter& BinaryWriter::write7Bit<int64_t>(make_unsigned<int64_t>::type value, uint8_t bytes);


} // namespace Mona
