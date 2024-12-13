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

#include "Mona/Memory/Packet.h"
#include "Mona/Util/Exceptions.h"

using namespace std;


namespace Mona {

const Shared<const Bytes> Packet::_NullBytes;

uint32_t Packet::identicalBytes(const Packet& packet) const {
	uint32_t size = min(_size, packet._size);
	uint32_t i;
	for (i = 0; i < size; ++i) {
		if (_data[i] != packet._data[i])
			break;
	}
	return i;
}

Packet& Packet::operator+=(uint32_t offset) {
	if (offset>_size)
		offset = _size;
	_data += offset;
	_size -= offset;
	return self;
}

Packet& Packet::operator-=(uint32_t count) {
	if (count > _size)
		count = _size;
	_size -= count;
	return self;
}

Packet Packet::operator+(uint32_t offset) const {
	if (offset>_size)
		offset = _size;
	return Packet(self, _data + offset, _size - offset);
}

Packet Packet::operator-(uint32_t count) const {
	if (count>_size)
		count = _size;
	return Packet(self, _data, _size - count);
}

const Shared<const Bytes>& Packet::bufferize() const {
	if (_data && !*_ppBuffer) { // if has data, and _ppBuffer is empty (no bufferized) => bufferize!
		_reference = false;
		_ppBuffer = new Shared<const Bytes>(Shared<Buffer>(SET, _data, _size));
		_data = (*_ppBuffer)->data(); // fix new data address
	}
	return *_ppBuffer;
}

Packet& Packet::set(const Packet&& packet) {
	if (!packet) // if size==0 the normal behavior is required to get the same data address
		return set(NULL, 0);
	if (!_reference)
		delete _ppBuffer;
	else
		_reference = false;
	_ppBuffer = packet._ppBuffer ? new Shared<const Bytes>(packet.bufferize()) : nullptr;
	_data = packet._data;
	_size = packet._size;
	return self;
}

Packet& Packet::set(const Packet& packet) {
	_data = packet._data;
	_size = packet._size;
	if (!_reference) {
		if (packet._ppBuffer == _ppBuffer)
			return self; // packet is a reference to this, change just data aera!
		delete _ppBuffer;
		_reference = true;
	}
	_ppBuffer = packet._ppBuffer;
	return self;
}


Packet& Packet::set(const char* data, uint32_t size) {
	if (!_reference) {
		delete _ppBuffer;
		_reference = true;
	}
	_ppBuffer = &_NullBytes;
	_data = data;
	_size = size;
	return self;
}


Packet& Packet::set(const Packet& packet, const char* data, size_t size) {
	if (size == string::npos)
		return set(packet).setArea(data, packet.size() - (data - packet.data()));
	return set(packet).setArea(data, size);
}

Packet& Packet::set(const Packet&& packet, const char* data, size_t size) {
	if (size == string::npos)
		return set(move(packet)).setArea(data, packet.size() - (data - packet.data()));
	return set(move(packet)).setArea(data, size);
}

Packet& Packet::setArea(const char* data, uint32_t size) {
	if (_data) {
		// check aera
#if defined(_DEBUG)
		if (data<_data || (data + size)>(_data + _size))
			FATAL_ERROR("Area of data requested outside data referenced");
#else
		// In release fix the error, avoid a crash issue on user error for example
		if (data < _data)
			data = _data;
		if ((data + size)>(_data + _size)) {
			if (data > (_data + _size)) {
				data = _data + _size;
				size = 0;
			} else
				size = _data + _size - data;
		}
#endif
	}
	_data = data;
	_size = size;
	return self;
}


} // namespace Mona
