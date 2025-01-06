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
#include <thread>
#include <atomic>

namespace Mona {

#define BUFFER_RESET(BUFFER,SIZE) (BUFFER ? BUFFER->resize(SIZE, false) : BUFFER.set(SIZE))

/**
Buffer dynamic with clear/resize/data/size method (like std) */
struct Buffer : Bytes, virtual Object {
	NULLABLE(!_data);

	Buffer(uint32_t size = 0);
	Buffer(const char* data, uint32_t size);

	virtual ~Buffer();


	Buffer&			clip(uint32_t offset);
	Buffer&			append(const void* data, uint32_t size);
	Buffer&			append(uint32_t count, char value);
	Buffer&			resize(uint32_t size, bool preserveContent = true);
	Buffer&			clear() { resize(0, false); return *this; }

	uint8_t&			operator[](uint32_t i) { DEBUG_ASSERT(_data && i<_size) return (uint8_t&)_data[i]; }

	char*			data() { return _data; } // beware, data() can be null
	const char	*	data() const override { return _data; }
	uint32_t			size() const override { return _size; }

	uint32_t			capacity() const { return _capacity; }

	static Buffer&   Null() { static Buffer Null(0, nullptr); return Null; } // usefull for Writer Serializer for example (and can't be encapsulate in a Shared<Buffer>)


	struct Allocator : virtual Object {
		template<typename AllocatorType=Allocator, typename ...Args>
		static void   Set(Args&&... args) { Lock(); Get().set<AllocatorType>(std::forward<Args>(args)...); Unlock(); }
		static char*  Alloc(uint32_t& size);
		static void	  Free(char* buffer, uint32_t size);
	protected:
		virtual char*  alloc(uint32_t& capacity) { return new char[capacity]; }
		virtual void   free(char* buffer, uint32_t capacity) { delete[] buffer; }

		static void Lock() { while (!TryLock()) std::this_thread::yield(); }
		static void Unlock() { Mutex().clear(std::memory_order_release); }
	private:
		static bool TryLock() { return !Mutex().test_and_set(std::memory_order_acquire); }

		static Unique<Allocator>& Get() { static Unique<Allocator> PAllocator(SET); return PAllocator; }
		static std::atomic_flag&  Mutex() { static std::atomic_flag Mutex = ATOMIC_FLAG_INIT; return Mutex; }
		
	};
private:
	Buffer(uint32_t size, char* buffer);

	uint32_t				_offset;
	char*				_data;
	uint32_t				_size;
	uint32_t				_capacity;
	char*				_buffer;


	friend struct BinaryWriter;
};




} // namespace Mona
