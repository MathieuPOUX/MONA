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

#include "Mona/Format/BinaryReader.h"

using namespace std;

namespace Mona {

BinaryReader BinaryReader::Null(NULL,0);

BinaryReader::BinaryReader(const char* data, uint32_t size, Bytes::Order byteOrder) :
	_end(data+size),_data(data),_size(size),_current(data), _flipBytes(byteOrder != Bytes::ORDER_NATIVE) {
}

uint32_t BinaryReader::next(uint32_t count) {
	if (count > available())
		count = available();
	_current += count; 
	return count;
}

uint32_t BinaryReader::shrink(uint32_t available) {
	uint32_t rest(this->available());
	if (available > rest)
		return rest;
	_end = _current+ available;
	_size = _end-_data;
	return available;
}

char* BinaryReader::read(uint32_t size, char* value) {
	uint32_t available(this->available());
	if (size > available) {
		_current += available;
		return value; // must returns default value if no size available to read (to avoid to interpret a consufed read32() value as valid on 16 bits!)
	}
	memcpy(value, _current,size);
	_current += size;
	return value;
}


uint16_t BinaryReader::read16() {
	uint16_t value(0);
	read(sizeof(value),STR &value);
	if (_flipBytes)
		return Bytes::Flip16(value);
	return value;
}

uint32_t BinaryReader::read24() {
	uint32_t value(0);
	read(3, STR&value);
	if (_flipBytes)
		return Bytes::Flip24(value);
	return value;
}

uint32_t BinaryReader::read32() {
	uint32_t value(0);
	read(sizeof(value), STR&value);
	if (_flipBytes)
		return Bytes::Flip32(value);
	return value;
}


uint64_t BinaryReader::read64() {
	uint64_t value(0);
	read(sizeof(value), STR&value);
	if (_flipBytes)
		return Bytes::Flip64(value);
	return value;
}

double BinaryReader::readDouble() {
	double value(0);
	read(sizeof(value), STR&value);
	if (_flipBytes)
		return Bytes::Flip(value);
	return value;
}

float BinaryReader::readFloat() {
	float value(0);
	read(sizeof(value), STR&value);
	if (_flipBytes)
		return Bytes::Flip(value);
	return value;
}

template<typename ValueType>
ValueType BinaryReader::read7Bit(uint8_t bytes) {
	uint8_t byte;
	ValueType result = 0; // unsigned!
	do {
		byte = read8();
		if (!--bytes) {
			result = (result << 8) | byte; // Use all 8 bits from the 5th byte
			break;
		}
		result = (result << 7) | (byte & 0x7F);
	} while (byte & 0x80);
	if (!is_signed<ValueType>::value)
		return result;
	bool isNegative = result & 1;
	result >>= 1; // remove bit sign
	return isNegative ? -typename make_signed<ValueType>::type(result) : result;
}

template uint16_t BinaryReader::read7Bit(uint8_t bytes);
template uint32_t BinaryReader::read7Bit(uint8_t bytes);
template uint64_t BinaryReader::read7Bit(uint8_t bytes);
template int16_t BinaryReader::read7Bit(uint8_t bytes);
template int32_t BinaryReader::read7Bit(uint8_t bytes);
template int64_t BinaryReader::read7Bit(uint8_t bytes);

} // namespace Mona
