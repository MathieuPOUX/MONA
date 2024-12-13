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
#include "Mona/Util/Parameters.h"
#include "Mona/Format/Value.h"
#include "Mona/Util/Exceptions.h"
#include <vector>


namespace Mona {


struct XMLParser : virtual Object {
private:

	struct Tag {
		Tag() : full(false) {}
		const char*		name;
		uint32_t			size;
		bool			full;
	};

	//// TO OVERRIDE /////

	virtual bool onStartXMLDocument() { return true; }
	
	virtual bool onXMLInfos(const char* name, const Value& attributes) { return true; }

	virtual bool onStartXMLElement(const char* name, const Value& attributes) = 0;
	virtual bool onInnerXMLElement(const char* name, const Packet& inner) = 0;
	virtual bool onEndXMLElement(const char* name) = 0;

	virtual void onEndXMLDocument(const char* error) {}

	/////////////////////

public:

	enum RESULT {
		RESULT_DONE,
		RESULT_PAUSED,
		RESULT_ERROR
	};

	struct XMLState : virtual Object {
		NULLABLE(!_current)

		XMLState() : _current(NULL) {}
		void clear() { _current = NULL; }
	private:
		bool						_started;
		Exception					_ex;
		const char*					_current;
		std::vector<Tag>			_tags;

		friend struct XMLParser;
	};

	void reset();
	void reset(const XMLState& state);
	void save(XMLState& state);

	RESULT parse(Exception& ex);

protected:

	XMLParser(const Packet& packet) : _started(false),_reseted(false), _current(packet.data()),_running(false), _packet(packet) {}

private:

	RESULT		parse();
	const char*	parseXMLName(const char* endMarkers, uint32_t& size);

	
	Value						_attributes;
	bool						_running;
	bool						_reseted;
	Shared<Buffer>				_pInner;

	// state
	bool						_started;
	Exception					_ex;
	const char*					_current;
	std::vector<Tag>			_tags;

	// const
	Packet						_packet;
};


} // namespace Mona
