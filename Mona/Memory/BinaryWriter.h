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
#include "Mona/Memory/Buffer.h"

namespace Mona {

struct BinaryWriter : Bytes, virtual Object {
	NULLABLE(empty())

	BinaryWriter(char* buffer, uint32_t size, Bytes::Order byteOrder = Bytes::ORDER_NETWORK);
	BinaryWriter(Buffer& buffer, Bytes::Order byteOrder = Bytes::ORDER_NETWORK);
	virtual ~BinaryWriter();

	BinaryWriter& append(const void* data, uint32_t size) { _pBuffer->append(data, size); return self; }
	BinaryWriter& append(uint32_t count, char value) { _pBuffer->append(count, value); return self; }
	BinaryWriter& write(const void* data, uint32_t size) { return append(data, size); }
	template<typename CharType, uint32_t size, typename = typename std::enable_if<std::is_convertible<CharType, const char>::value, CharType>::type>
	BinaryWriter& write(CharType(&data)[size]) { return append(data, size-1); }
	template<typename STRType, typename = typename std::enable_if<std::is_convertible<STRType, const char*>::value, STRType>::type>
	BinaryWriter& write(STRType data) { return append(data, strlen(data)); }
	BinaryWriter& write(const Bytes& bytes) { return append(EXP(bytes)); }
	BinaryWriter& write(const std::string& value) { return append(EXP(value)); }
	BinaryWriter& write(char value) { return append(&value, sizeof(value)); }

	BinaryWriter& write8(uint8_t value) { return append(&value, sizeof(value)); }
	BinaryWriter& write16(uint16_t value);
	BinaryWriter& write24(uint32_t value);
	BinaryWriter& write32(uint32_t value);
	BinaryWriter& write64(uint64_t value);
	BinaryWriter& writeDouble(double value);
	BinaryWriter& writeFloat(float value);
	BinaryWriter& writeBool(bool value) { return write8(value ? 1 : 0); }
	
	/**
	Write a 7Bit value, supports signed value in adding a first sign low bit*/
	template<typename ValueType>
	BinaryWriter& write7Bit(typename std::make_unsigned<ValueType>::type value);
	BinaryWriter& writeString(const std::string& value) {
		return write(value.data(), value.size());
	}
	BinaryWriter& writeString(const char* value, std::size_t size = std::string::npos) {
		return write(value, size == std::string::npos ? strlen(value) : size).write8(0);
	}

	BinaryWriter& writeRandom(uint32_t count=1);

	BinaryWriter& next(uint32_t count = 1) { return resize(size() + count); }
	BinaryWriter& clear(uint32_t size = 0) { return resize(size); }
	BinaryWriter& resize(uint32_t size) { if (_pBuffer->data()) _pBuffer->resize(_offset + size,true); return self; }

	// method doesn't supported by BinaryWriter::Null or BinaryWriter static, raise an exception
	char*	buffer(uint32_t size);

	// beware, data() can be null in the BinaryWriter::Null case
	const char*   data() const override { return _pBuffer->data() + _offset; }
	uint32_t		  size() const override { return _pBuffer->size() - _offset; }
	Buffer&		  buffer() { return *_pBuffer; }

	static BinaryWriter& Null() { static BinaryWriter Null(Buffer::Null()); return Null; }

private:

	bool			_flipBytes;
	mutable Buffer*	_pBuffer;
	bool			_reference;
	uint32_t			_offset;
};



} // namespace Mona
