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

#include "Mona/Format/XMLParser.h"

using namespace std;

namespace Mona {

void XMLParser::reset() {
	_pInner.reset();
	_reseted = true;
	_started=false;
	_ex = NULL;
	_current = _packet.data();
	_tags.clear();
}

void XMLParser::reset(const XMLState& state) {
	if (!state)
		return reset();
	_reseted = true;
	_started = state._started;
	_ex = state._ex;
	_current = state._current;
	_tags = state._tags;
}

void XMLParser::save(XMLState& state) {
	state._started = _started;
	state._current = _current;
	state._tags    = _tags;
	state._ex = _ex;
}

XMLParser::RESULT XMLParser::parse(Exception& ex) {
	if (_running) {
		ex.set<Ex::Intern>("Don't call recursively XMLParser::parse function");
		return RESULT_ERROR;
	}
	if (_ex) {
		ex = _ex;
		return RESULT_ERROR;
	}
	_running = true;
	RESULT result = parse();
	if (result!=RESULT_PAUSED)
		onEndXMLDocument(_ex ? (ex=_ex).c_str() : NULL);
	_running = false;
	return result;
}

XMLParser::RESULT XMLParser::parse() {

RESET:
	_reseted = false;
	if (_ex)
		return RESULT_ERROR;

	
	while (!_tags.empty()) {
		Tag& tag(_tags.back());
		if (!tag.full)
			break;
		String::Scoped scoped(tag.name, tag.size, _packet);
		_tags.pop_back();
		if(!onEndXMLElement(scoped))
			return  (_current == _packet.end() && _tags.empty()) ? RESULT_DONE : RESULT_PAUSED;
		if (_reseted)
			goto RESET;
	}

	uint32_t		 size(0);
	const char*  name(NULL);
	uint32_t		 innerSpaceRemovables = 0;
	
	while (_current<_packet.end()) {

		// start element or end element
		if (*_current == '<') {

			if (!_started) {
				_started = true; // before to allow a call to save
				if (!onStartXMLDocument())
					return RESULT_PAUSED;
				if (_reseted)
					goto RESET;
			}

			// process inner!
			if (_pInner && !((_packet.end() - _current) >= 9 && memcmp(_current, "<![CDATA[", 9) == 0)) {
	
				// skip end space if not CDATA + garante a capacity size+1 to be able to scope the packet in InnerXMLElement
				_pInner->resize(_pInner->size() - innerSpaceRemovables + 1);
				_pInner->resize(_pInner->size() - 1);

				Tag& tag(_tags.back());
				if(!onInnerXMLElement(String::Scoped(tag.name, tag.size, _packet), Packet(_pInner)))
					return RESULT_PAUSED;
				if (_reseted)
					goto RESET;
			}

			if (++_current == _packet.end()) {
				_ex.set<Ex::Format::Invalid>("XML element without tag name");
				return RESULT_ERROR;
			}

			// just after <

			bool next(true);
				
			if (*_current == '/') {
				//// END ELEMENT ////

				if (_tags.empty()) {
					_ex.set<Ex::Format::Invalid>("XML end element without starting before");
					return RESULT_ERROR;
				}

				Tag& tag(_tags.back());
				name = tag.name;
				size = tag.size;

				if ((_current+size) >= _packet.end() || memcmp(name,++_current,size)!=0) {
					_ex.set<Ex::Format::Invalid>("XML end element is not the '",Packet(name,size),"' expected");
					return RESULT_ERROR;
				}
				_current += size;

				// skip space
				while (_current < _packet.end() && isspace(*_current))
					++_current;
			
				if (_current == _packet.end() || *_current!='>') {
					_ex.set<Ex::Format::Invalid>("XML end '", Packet(name,size),"' element without > termination");
					return RESULT_ERROR;
				}
	
				// on '>'

				// skip space
				++_current;
				if (!_pInner)
					while (_current < _packet.end() && isspace(*_current))
						++_current;

				_tags.pop_back();
				next = onEndXMLElement(String::Scoped(name, size, _packet));

			} else if (*_current == '!') {
				//// COMMENT or CDATA ////

				if (++_current == _packet.end()) {
					_ex.set<Ex::Format::Invalid>("XML comment or CDATA malformed");
					return RESULT_ERROR;
				}

				if (_pInner || ((_packet.end() - _current) >= 7 && memcmp(_current, "[CDATA[", 7) == 0)) {
					/// CDATA ///

					if (_tags.empty()) {
						_ex.set<Ex::Format::Invalid>("No XML CDATA inner value possible without a XML parent element");
						return RESULT_ERROR;
					}

					_current += 7;
					// can be end
					if(!_pInner)
						_pInner.set();
					innerSpaceRemovables = 0;
					while (_current < _packet.end()) {
						if (*_current == ']' && ++_current < _packet.end() && *_current == ']' && ++_current < _packet.end() && *_current == '>')
							break;
						_pInner->append(_current++, 1);
					}
		
					if (_current == _packet.end()) {
						_ex.set<Ex::Format::Invalid>("XML CDATA without ]]> termination");
						return RESULT_ERROR;
					}
				
					// on '>'

				} else {
					/// COMMENT ///

					char delimiter1 = *_current;
					if (++_current == _packet.end()) {
						_ex.set<Ex::Format::Invalid>("XML comment malformed");
						return RESULT_ERROR;
					}
					char delimiter2 = *_current;

					while (++_current < _packet.end()) {
						if (*_current == delimiter1 && ++_current < _packet.end() && *_current == delimiter2 && ++_current < _packet.end() && *_current == '>')
							break;
					}

					if (_current == _packet.end()) {
						_ex.set<Ex::Format::Invalid>("XML comment without > termination");
						return RESULT_ERROR;
					}

					// on '>'
				}
				
				// skip space
				++_current;
				if (!_pInner)
					while (_current < _packet.end() && isspace(*_current))
						++_current;

			} else {

				//// START ELEMENT ////

				bool isInfos(*_current == '?');
				if (isInfos && ++_current == _packet.end()) {
					_ex.set<Ex::Format::Invalid>("XML info element without name");
					return RESULT_ERROR;
				}

				Tag tag;
				name = tag.name = parseXMLName(isInfos ? "?" : "/>", tag.size);
				if (!name)
					return RESULT_ERROR;
				size = tag.size;
		
				/// space or > or /

				/// read attributes
				_attributes.clear();
				while (_current < _packet.end()) {

					// skip space
					while (isspace(*_current)) {
							if (++_current==_packet.end())  {
							_ex.set<Ex::Format::Invalid>("XML element '", Packet(name,size),"' without termination");
							return RESULT_ERROR;
						}
					}
				
					if (isInfos && (*_current == '?')) {
						if (++_current == _packet.end() || *_current!='>') {
							_ex.set<Ex::Format::Invalid>("XML info '",Packet(name,size),"' without ?> termination");
							return RESULT_ERROR;
						}
						break;
					} else if (*_current == '/') {
						if (++_current == _packet.end() || *_current!='>') {
							_ex.set<Ex::Format::Invalid>("XML element '", Packet(name,size),"' without termination");
							return RESULT_ERROR;
						}
						tag.full = true;
						break;
					} else if (*_current == '>')
						break;
	
					// read name attribute
					uint32_t sizeKey(0);
					const char* key(parseXMLName("=", sizeKey));
					if (!key)
						return RESULT_ERROR;

					/// space or =

					// skip space
					while(isspace(*_current)) {
						if (++_current == _packet.end()) {
							_ex.set<Ex::Format::Invalid>("XML attribute '", Packet(key,sizeKey),"' without value");
							return RESULT_ERROR;
						}
						if (*_current == '=')
							break;
					}

					// =

					if (++_current==_packet.end()) {
						_ex.set<Ex::Format::Invalid>("XML attribute '", Packet(key,sizeKey),"' without value");
						return RESULT_ERROR;
					}

					// skip space
					while(isspace(*_current)) {
						if (++_current == _packet.end()) {
							_ex.set<Ex::Format::Invalid>("XML attribute '", Packet(key,sizeKey),"' without value");
							return RESULT_ERROR;
						}
					}

					/// just after = and space

					char delimiter(*_current);

					if ((delimiter != '"' && delimiter != '\'') || ++_current==_packet.end()) {
						_ex.set<Ex::Format::Invalid>("XML attribute '", Packet(key,sizeKey),"' without value");
						return RESULT_ERROR;
					}

			

					/// just after " start of attribute value

					// read value attribute
					uint32_t sizeValue(0);
					const char* value(_current);
					while (*_current != delimiter) {
						if (*_current== '\\') {
							if (++_current == _packet.end()) {
								_ex.set<Ex::Format::Invalid>("XML attribute '", Packet(key,sizeKey),"' with malformed value");
								return RESULT_ERROR;
							}
							++sizeValue;
						}
						if(++_current==_packet.end()) {
							_ex.set<Ex::Format::Invalid>("XML attribute '", Packet(key,sizeKey),"' with malformed value");
							return RESULT_ERROR;
						}
						++sizeValue;
					}
						
					++_current;

					/// just after " end of attribute value

					// store name=value
					_attributes.emplace(string(key, sizeKey), Packet(_packet, value, sizeValue));
				}



				if (_current==_packet.end())  {
					_ex.set<Ex::Format::Invalid>("XML element '", Packet(name,size),"' without > termination");
					return RESULT_ERROR;
				}
			
				// on '>' ('/>' or  '?>' or '>')

				// skip space
				++_current;
				if (!_pInner)
					while (_current < _packet.end() && isspace(*_current))
						++_current;

				String::Scoped scoped(name, size, _packet);
				if (isInfos)
					next = onXMLInfos(scoped, _attributes);
				else {
					_tags.emplace_back(tag); // before to allow a call to save
					next = onStartXMLElement(scoped, _attributes);
					if (next && tag.full) {
						_tags.pop_back();  // before to allow a call to save
						next = onEndXMLElement(scoped);
					}
				}
			}


			if (!next)
				return (_current == _packet.end() && _tags.empty()) ? RESULT_DONE : RESULT_PAUSED;
			if (_reseted)
				goto RESET;
	
			// after '>' (element end or element start) and after space, and can be _packet.end()

		} else {
			if (_pInner) { // inner value
				_pInner->append(_current, 1);
				if (isspace(*_current))
					++innerSpaceRemovables;
				else
					innerSpaceRemovables = 0;
			} else if (!isspace(*_current)) {
				if (_tags.empty()) {
					_ex.set<Ex::Format::Invalid>("No XML inner value possible without a XML parent element");
					return RESULT_ERROR;
				}
				_pInner.set().append(_current, 1);
				innerSpaceRemovables = 0;
			}	
			++_current;
		}
			
	}

	if (!_started) {
		_started = true;  // before to allow a call to save
		if (!onStartXMLDocument())
			return RESULT_PAUSED;
		if (_reseted)
			goto RESET;
	} else if (!_tags.empty()) {
		_ex.set<Ex::Format::Invalid>("No XML end '", Packet(_tags.back().name, _tags.back().size), "' element");
		return RESULT_ERROR;
	}

	return RESULT_DONE; // no more data!
}


const char* XMLParser::parseXMLName(const char* endMarkers, uint32_t& size) {

	if (!isalpha(*_current) && *_current!='_')  {
		_ex.set<Ex::Format::Invalid>("XML name must start with an alphabetic character");
		return NULL;
	}
	const char* name(_current++);
	while (_current < _packet.end() && isxml(*_current))
		++_current;
	size = (_current - name);
	if (_current==_packet.end() || (!strchr(endMarkers,*_current) && !isspace(*_current)))  {
		_ex.set<Ex::Format::Invalid>("XML name '", Packet(name,size),"' without termination");
		return NULL;
	}
	return name;
}

} // namespace Mona
