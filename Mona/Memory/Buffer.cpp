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

#include "Mona/Memory/Buffer.h"
#include "Mona/Util/Exceptions.h"

using namespace std;

namespace Mona {

static char _Empty;

char* Buffer::Allocator::Alloc(uint32_t& size) {
	if (size<=16) { // at minimum allocate 16 bytes!
		if (!size) {
			Get(); // just access to Allocator to create it, to avoid a crash on static Buffer destruction with a Allocator access after deletion (see SerialisersTest for example)
			return &_Empty;
		}
		size = 16;
	} else { 
		 // fast compute the closer upper power of two for new capacity
		uint32_t capacity = size - 1; // at minimum allocate 16 bytes!
		capacity |= capacity >> 1;
		capacity |= capacity >> 2;
		capacity |= capacity >> 4;
		capacity |= capacity >> 8;
		capacity |= capacity >> 16;
		if (++capacity) // if = 0 exceeds uint32_t, keep original size required
			size = capacity;
		if (size>0x80000000)
			return new char[size];
	}
	if (!TryLock())
		return new char[size];
	char* buffer = Get()->alloc(size);
	Unlock();
	return buffer;
}
void Buffer::Allocator::Free(char* buffer, uint32_t size) {
	if (!size) {
		if (buffer != &_Empty)
			delete[] buffer;
		return;
	}
	if ((size & (size - 1)) || !TryLock())  // check than we have a size create with Alloc (capacity log2) + TryLock working!
		return delete[] buffer;
	Get()->free(buffer, size);
	Unlock();
}


Buffer::Buffer(uint32_t size) : _offset(0), _size(size), _capacity(size) {
	_data = _buffer = Allocator::Alloc(_capacity);
}
Buffer::Buffer(const char* data, uint32_t size) : _offset(0), _size(size), _capacity(size) {
	memcpy(_data = _buffer = Allocator::Alloc(_capacity), data, size);
}

Buffer::Buffer(uint32_t size, char* buffer) : _offset(0), _data(buffer), _size(size), _capacity(size), _buffer(NULL) {}

Buffer::~Buffer() {
	if (_buffer)
		Allocator::Free(_buffer, _capacity);
}

Buffer& Buffer::append(const void* data, uint32_t size) {
	if (!_data) // to expect null Buffer 
		return self;
	uint32_t oldSize(_size);
	resize(_size + size);
	memcpy(_data + oldSize, data, size);
	return self;
}
Buffer& Buffer::append(uint32_t count, char value) {
	if (!_data) // to expect null Buffer 
		return self;
	uint32_t oldSize(_size);
	resize(_size + count);
	memset(_data + oldSize, value, count);
	return self;
}

Buffer& Buffer::clip(uint32_t offset) {
	if (offset >= _size)
		return clear();
	if (offset) {
		_offset += offset;
		_data += offset;
		_size -= offset;
		_capacity -= offset;
	}
	return *this;
}

Buffer& Buffer::resize(uint32_t size, bool preserveData) {
	if (size <= _capacity) {
		if (_offset && !preserveData) {
			// fix possible clip
			_capacity += _offset;
			_data -= _offset;
			_offset = 0;
		}
		_size = size;
		return *this;
	}

	// here size > capacity, so size > _size

	char* oldData(_data);

	// try without offset
	if (_offset) {
		_capacity += _offset;
		_data -= _offset;
		_offset = 0;
		if (size <= _capacity) {
			if (preserveData)
				memmove(_data, oldData, _size);
			_size = size;
			return *this;
		}
	}

	if (!_buffer)
		FATAL_ERROR("Static buffer exceeds maximum ",_capacity," bytes capacity");

	// allocate
	uint32_t oldCapacity(_capacity);
	_data = Allocator::Alloc(_capacity=size);

	 // copy data
	if (preserveData)
		memcpy(_data, oldData, _size);


	// deallocate if was allocated
	if (oldCapacity)
		Allocator::Free(_buffer, oldCapacity);

	_size = size;
	_buffer=_data;
	return *this;
}



} // namespace Mona
