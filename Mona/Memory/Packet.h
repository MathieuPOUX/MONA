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
/*!
Packet is a ingenious buffer tool which can be mainly used in two complement ways:
- As a signature method requirement to allow an area of data to be distributed/shared without copying data.
		void method(const Packet& packet)
	To allow a sharing buffer referenced (without data copy) Packet constructor will make data referenced immutable.
- As a way to hold a sharing buffer (in buffering it if not already done)

Example:
	Packet unbuffered("data",4);
	Packet buffered(pBuffer);
	...
	std::deque<Packet> packets;
	// Here Packet wraps a unbuffered area of data
	packets.push(std::move(unbuffered));
	// Here Packet wraps a already buffered area of data, after this call pBuffer is empty (became immutable), and no copying data will occur anymore
	packets.push(std::move(buffered)); */
struct Packet: Bytes, virtual Object {
	NULLABLE(!data())
	/*!
	Build an empty Packet */
	Packet(std::nullptr_t=nullptr) : _ppBuffer(&_NullBytes), _data(NULL), _size(0), _reference(true) {}
	/*!
	Reference an immutable area of unbuffered data */
	template<typename STRType, typename = typename std::enable_if<std::is_convertible<STRType, const char*>::value, STRType>::type>
	Packet(STRType data) : _data(data), _size(strlen(data)), _ppBuffer(&_NullBytes), _reference(true) {}
	/*!
	Reference an immutable area of unbuffered data */
	Packet(const char* data, uint32_t size) : _data(data), _size(size), _ppBuffer(&_NullBytes), _reference(true) {}
	/*!
	Reference an immutable area of unbuffered data */
	template<typename CharType, uint32_t size, typename = typename std::enable_if<std::is_convertible<CharType, const char>::value, CharType>::type>
	Packet(CharType(&data)[size]) : _reference(true) { set<CharType, size>(data); }
	/*!
	Reference an immutable area of unbuffered data */
	Packet(const Bytes& bytes) : _ppBuffer(&_NullBytes), _data(bytes.data()), _size(bytes.size()), _reference(true) {}
	/*!
	Reference an immutable area of unbuffered data */
	template<typename Type, typename = typename std::enable_if<std::is_class<Type>::value && std::is_convertible<Type, std::string>::value, Type>::type>
	Packet(const Type& value) : _reference(true) { set<std::string>(value); }
	/*!
	Create a copy from packet (explicit to not reference a temporary packet) */
	explicit Packet(const Packet& packet) : _reference(true) { set(packet); }
	/*!
	Create a copy from packet and move area of data referenced, area have to be include inside (explicit to not reference a temporary packet) */
	explicit Packet(const Packet& packet, const char* data, std::size_t size = std::string::npos) : _reference(true) { set(packet, data, size); }
	/*!
	Bufferize packet */
	Packet(const Packet&& packet) : _reference(true) { set(std::move(packet)); }
	/*!
	Bufferize packet and move area of data referenced, area have to be include inside */
	Packet(const Packet&& packet, const char* data, std::size_t size = std::string::npos) : _reference(true) { set(std::move(packet), data, size); }
	/*!
	Reference an immutable area of buffered data (explicit to not reference a temporary pBuffer) */
	template<typename ByteType>
	explicit Packet(const Shared<const ByteType>& pBuffer) : _reference(true) { set(pBuffer); }
	/*!
	Reference an immutable area of buffered data and move area of data referenced, area have to be include inside
	(explicit to not reference a temporary pBuffer) */
	template<typename ByteType>
	explicit Packet(const Shared<const ByteType>& pBuffer, const char* data) : _reference(true) { set(pBuffer, data); }
	template<typename ByteType>
	explicit Packet(const Shared<const ByteType>& pBuffer, const char* data, std::size_t size = std::string::npos) : _reference(true) { set(pBuffer, data, size); }
	/*!
	Capture the buffer passed in parameter, buffer become immutable and allows safe distribution
	(explicit to note that buffer is release) */
	explicit Packet(std::string& buffer) : _reference(true) { set(buffer); }
	Packet(std::string&& buffer) : _reference(true) { set(buffer); }
	/*!
	Capture the buffer passed in parameter and move area of data referenced, buffer become immutable and allows safe distribution, area have to be include inside */
	Packet(std::string&& buffer, const char* data, std::size_t size = std::string::npos) : _reference(true) { set(buffer, data, size); }
	/*!
	Capture the buffer passed in parameter, buffer become immutable and allows safe distribution
	(explicit to note that pBuffer is release) */
	template<typename ByteType, typename = typename std::enable_if<!std::is_const<ByteType>::value>::type>
	explicit Packet(Shared<ByteType>& pBuffer) : _reference(true) { set(pBuffer); }
	template<typename ByteType, typename = typename std::enable_if<!std::is_const<ByteType>::value>::type>
	Packet(Shared<ByteType>&& pBuffer) : _reference(true) { set(pBuffer); }
	/*!
	Capture the buffer passed in parameter and move area of data referenced, buffer become immutable and allows safe distribution, area have to be include inside */
	template<typename ByteType, typename = typename std::enable_if<!std::is_const<ByteType>::value>::type>
	Packet(Shared<ByteType>& pBuffer, const char* data, std::size_t size = std::string::npos) : _reference(true) { set(pBuffer, data, size); }
	/*!
	Release the referenced area of data */
	virtual ~Packet() { if (!_reference) delete _ppBuffer; }
	/*!
	Allow to compare data packet*/
	bool operator == (const Packet& packet) const { return _size == packet._size && (!_size || memcmp(_data, packet._data, _size)==0); }
	bool operator != (const Packet& packet) const { return !operator==(packet); }
	bool operator <  (const Packet& packet) const { return _size != packet._size ? _size < packet._size : (_size && memcmp(_data, packet._data, _size) < 0); }
	bool operator <= (const Packet& packet) const { return operator==(packet) || operator<(packet); }
	bool operator >  (const Packet& packet) const { return !operator<=(packet); }
	bool operator >= (const Packet& packet) const { return operator==(packet) || operator>(packet); }

	/*!
	Return number of identical bytes */
	uint32_t identicalBytes(const Packet& packet) const;
	/*!
	Return buffer */
	const Shared<const Bytes>&	buffer() const { return _ppBuffer ? *_ppBuffer : _NullBytes; }
	/*!
	Return data */
	const char*					data() const override { return _data; }
	/*!
	Return size */
	uint32_t						size() const override { return _size; }

	/*!
	Move the area of data referenced */
	Packet& operator+=(uint32_t offset);
	/*!
	Move the area of data referenced */
	Packet& operator++() { return operator+=(1); }
	/*!
	Get a new area of data referenced (move) in a new packet */
	Packet  operator+(uint32_t offset) const;
	/*!
	[1] Get a new area of data referenced (resize) in a new packet */
	Packet  operator-(uint32_t count) const;
	/*!
	Resize the area of data referenced */
	Packet& operator-=(uint32_t count);
	/*!
	Resize the area of data referenced */
	Packet& operator--() { return operator-=(1); }
	/*!
	= operator, redirection to set */
	template<typename Type>
	Packet& operator=(Type&& value) { return set(std::forward<Type>(value)); }
	/*!
	Reference a packet */
	Packet& operator=(const Packet& packet) { return set(packet); }
	/*!
	Release packet */
	Packet& operator=(std::nullptr_t) { return set(NULL, 0); }
	/*!
	Release packet */
	Packet& reset() { return set(NULL, 0); }
	/*!
	Shrink packet */
	Packet& shrink(uint32_t available) { return setArea(_data, available); }
	/*!
	Move the area of data referenced */
	Packet& clip(uint32_t offset) { return operator+=(offset); }
	/*!
	Reference an immutable area of unbuffered data */
	template<typename STRType, typename = typename std::enable_if<std::is_convertible<STRType, const char*>::value, STRType>::type>
	Packet& set(STRType data) { return set(data, strlen(data)); }
	/*!
	Reference an immutable area of unbuffered data */
	Packet& set(const char* data, uint32_t size);
	/*!
	Reference an immutable area of unbuffered data */
	template<typename CharType, uint32_t size, typename = typename std::enable_if<std::is_convertible<CharType, const char>::value, CharType>::type>
	Packet& set(CharType(&data)[size]) {
		if (!_reference) {
			delete _ppBuffer;
			_reference = true;
		}
		_ppBuffer = NULL;
		_data = data;
		_size = size-1;
		return self;
	}
	/*!
	Reference an immutable area of unbuffered data */
	Packet& set(const Bytes& bytes) { return set(EXP(bytes)); }
	/*!
	Reference an immutable area of unbuffered data */
	template<typename Type, typename = typename std::enable_if<std::is_class<Type>::value && std::is_convertible<Type, std::string>::value, Type>::type>
	Packet& set(const Type& value) { const std::string& val(value); return set(EXP(val)); }
	/*!
	Reference a packet */
	Packet& set(const Packet& packet);
	/*!
	Reference a packet and move area of data referenced, area have to be include inside */
	Packet& set(const Packet& packet, const char* data, std::size_t size = std::string::npos);
	/*!
	Bufferize packet */
	Packet& set(const Packet&& packet);
	/*!
	Bufferize packet and move area of data referenced, area have to be include inside */
	Packet& set(const Packet&& packet, const char* data, std::size_t size = std::string::npos);
	/*!
	Reference an immutable area of buffered data */
	template<typename ByteType>
	Packet& set(const Shared<const ByteType>& pBuffer) {
		if (!pBuffer || !pBuffer->data())  // if pBuffer->size==0 the normal behavior is required to get the same data address
			return set(NULL, 0);
		if (!_reference)
			delete _ppBuffer;
		else
			_reference = false;
		_data = pBuffer->data();
		_size = pBuffer->size();
		_ppBuffer = new Shared<const Bytes>(pBuffer);
		return self;
	}
	/*!
	Reference an immutable area of buffered data and move area of data referenced, area have to be include inside */
	template<typename ByteType>
	Packet& set(const Shared<const ByteType>& pBuffer, const char* data, std::size_t size = std::string::npos) {
		if (size == std::string::npos)
			return set(pBuffer).setArea(data, pBuffer->size() - (data - pBuffer->data()));
		return set(pBuffer).setArea(data, size);
	}
	/*!
	Capture the buffer passed in parameter, buffer become immutable and allows safe distribution */
	Packet& set(std::string& buffer) {
		struct SBytes : Bytes, std::string {
			SBytes(std::string& buffer) : std::string(std::move(buffer)) {}
			const char*	data() const override { return std::string::data(); }
			uint32_t size() const override { return std::string::size(); }
		};
		return set(Shared<SBytes>(SET, buffer));
	}
	/*!
	Capture the buffer passed in parameter and move area of data referenced, buffer become immutable and allows safe distribution, area have to be include inside */
	Packet& set(std::string& buffer, const char* data, std::size_t size = std::string::npos) {
		if (size == std::string::npos)
			size = buffer.size() - (data - buffer.data());
		return set(buffer).setArea(data, size);
	}
	/*!
	Capture the buffer passed in parameter, buffer become immutable and allows safe distribution */
	Packet& set(std::string&& buffer) { return set(buffer); }
	/*!
	Capture the buffer passed in parameter, buffer become immutable and allows safe distribution */
	template<typename ByteType, typename = typename std::enable_if<!std::is_const<ByteType>::value>::type>
	Packet& set(Shared<ByteType>& pBuffer) {
		if (!pBuffer || !pBuffer->data()) // if size==0 the normal behavior is required to get the same data address
			return set(NULL, 0); // no need here to capture pBuffer (no holder on)
		if (!_reference)
			delete _ppBuffer;
		else
			_reference = false;
		_data = pBuffer->data();
		_size = pBuffer->size();
		_ppBuffer = new Shared<const Bytes>(move(pBuffer)); // forbid now all changes by caller!
		return self;
	}
	template<typename ByteType, typename = typename std::enable_if<!std::is_const<ByteType>::value>::type>
	Packet& set(Shared<ByteType>&& pBuffer) { return set(pBuffer); }
	/*!
	Capture the buffer passed in parameter and move area of data referenced, buffer become immutable and allows safe distribution, area have to be include inside */
	template<typename ByteType, typename = typename std::enable_if<!std::is_const<ByteType>::value>::type>
	Packet& set(Shared<ByteType>& pBuffer, const char* data, std::size_t size = std::string::npos) {
		if (size == std::string::npos)
			size = pBuffer->size() - (data - pBuffer->data());
		return set(pBuffer).setArea(data, size);
	}

private:
	Packet& setArea(const char* data, uint32_t size);

	const Shared<const Bytes>&	bufferize() const;

	mutable const Shared<const Bytes>*	_ppBuffer; // if NULL means no bufferization required (static char*)
	mutable const char*					_data;
	mutable bool						_reference;
	uint32_t								_size;
	static const Shared<const Bytes>	_NullBytes;
};


} // namespace Mona
