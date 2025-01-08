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


struct BinaryReader : Bytes, virtual Object {
	NULLABLE(empty())

	BinaryReader(const char* data, size_t size, Bytes::Order byteOrder = Bytes::ORDER_NETWORK);

	char*			read(size_t size, char* buffer);
	template<typename BufferType, typename = typename std::enable_if<std::is_class<BufferType>::value, BufferType>::type>
	BufferType&		read(size_t size, BufferType& buffer) {
		buffer.resize(size);
		read(size,STR buffer.data());
		return buffer;
	}

	char			read() { return _current == _end ? 0 : *_current++; }

	std::string& 	readString(std::string& value);

	uint8_t			read8() { return _current==_end ? 0 : *_current++; }
	uint16_t		read16();
	uint32_t		read24();
	uint32_t		read32();
	uint64_t		read64();
	double			readDouble();
	float			readFloat();
	bool			readBool() { return _current==_end ? false : ((*_current++) != 0); }
	template<typename NumType>
	NumType			read7Bit();

	
	size_t			position() const { return _current-_data; }
	size_t			next(size_t count = 1);
	void			reset(size_t position = 0) { _current = _data+(position > _size ? _size : position); }
	size_t			shrink(size_t available);

	const char*		current() const { return _current; }
	uint32_t		available() const { return _end-_current; }

	// beware, data() can be null
	const char*		data() const override { return _data; }
	size_t			size() const override { return _size; }

	
	static BinaryReader Null;
private:
	
	bool			_flipBytes;
	const char*		_data;
	const char*		_end;
	const char*		_current;
	size_t			_size;
};


} // namespace Mona
