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
#include "Mona/Threading/Thread.h"

namespace Mona {

struct BufferPool : Buffer::Allocator, private Thread, virtual Object {

	BufferPool() { start(Thread::PRIORITY_LOWEST); }
	~BufferPool() { stop(); }

private:
	char* alloc(uint32_t& capacity) override {
		char* buffer = _buffers[computeIndex(capacity)].pop();
		return buffer ? buffer : new char[capacity];
	}
	void free(char* buffer, uint32_t capacity) override { _buffers[computeIndex(capacity)].push(buffer); }

	bool run(Exception& ex, const volatile bool& requestStop);
	uint8_t computeIndex(uint32_t capacity);

	struct Buffers : private std::vector<char*>, virtual Object {
		Buffers() : _minSize(0), _maxSize(0) {}
		~Buffers() { for (char* buffer : self) delete[] buffer; }
		char*	pop();
		void   push(char* buffer);
		void	manage(std::vector<char*>& gc);
	private:
		uint32_t _minSize;
		uint32_t _maxSize;
	};
	Buffers			 _buffers[28];
};


} // namespace Mona
