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

#include "Mona/Util/Value.h"
#include "Mona/Logs/Logs.h"


using namespace std;


namespace Mona {
	
static const Value Null;

Value::IteratorBase::IteratorBase(std::list<Value>& values) : 
	_values(values),
	_it(values.begin()) {
	skipNulls();
}

Value::IteratorBase::IteratorBase(std::list<Value>& values, std::list<Value>::iterator it) : 
	_values(values),
	_it(it) {
	skipNulls();
}


Value& Value::IteratorBase::item() {
	return *_it;
}

void Value::IteratorBase::increment() {
	auto end = _values.end();
	if (_it != end) {
		++_it;
		skipNulls(end);
	}
}

void Value::IteratorBase::decrement() {
	auto it = _it;
	// skip null item
	do {
		if (it == _values.begin()) {
			// no null before => don't decrement
			return;
		}
		--it;
	} while (!*it || _it->index < 0);
	_it = it;
}

bool Value::IteratorBase::equal(const IteratorBase& other) const {
	return _it == other._it;
}

void Value::IteratorBase::skipNulls(std::list<Value>::iterator end) {
	while (_it != end) {
		if(*_it && _it->index >= 0) {
			// not null or is inside the array part
			return;
		}
		++_it;
	};
}

std::list<Value>::iterator Value::IteratorBase::end() {
	// Search from the back the first no-null value
	auto it = _values.end();
	while (it != _values.begin()) {
		if(*--it) {
			return ++it;
		}
	}
	return it;
}

bool Value::operator==(const Value &other) const {
	if(type() != other.type()) {
		return false;
	}
	if(type() != OBJECT) {
		return _value == other._value;
	}

	auto it1 = begin(), end1 = end();
	auto it2 = other.begin(), end2 = other.end();

	while(it1 != end1) {
		if (it2 == end2 || *it1 != *it2) {
			return false;
		}
		++it1;
		++it2;
	}
	return it2 == end2;
}

Value& Value::set(const Value& other) {
	// Copy values, but preserve subscription!
	// Clear object part
	clearValues(_values.begin(), _beginArray);
	// Copy object/array
	auto it = _beginArray;
	auto itEndValid = it;
	auto itObj = _object.begin();
	for (const auto& item : other) {
		if(item.index<0) {
			// object
			if(item) {
				self[item.name] = item;
			}
			continue;
		}
		// Array
		if (it == _values.end()) {
			it = addItem(itObj);
		}
		*it = item;
		if(*it++) {
			itEndValid = it;
		}
	}
	// Clear following values!
	clearValues(itEndValid);
	// set final value
	return update(other._value);
}

Value& Value::updateValue(const Value& value) {
	if (this != &value && value) {
		// A children value becomes valid => Now must be an object!
		if (type() != Type::OBJECT) {
			_value = Object{};
		}
	}
	onUpdate(value);
	if (_pParent) {
		_pParent->updateValue(value);
	}
	return self;
}

void Value::clearValues(std::list<Value>::iterator from, std::list<Value>::iterator end) {
	if (from == end) {
		return;
	}
	if(from->index < 0) {
		// clear object part
		while (from != _beginArray) {
			// set to null
			*from = nullptr;
			// erase if no more onUpdate subscription
			if (!from->_object.size() && !from->onUpdate) {
				// erase!
				_object.erase(from->name.c_str());
				from = _values.erase(from);
				continue;
			}
			++from;
		}
	}

	// Now process Array part from the end to
	// - set to null all the items until from
	// - remove all the useless items without onUpdate subscription
	auto it = end;
	bool remove = true;
	while (from != it && it != _beginArray) {
		*--it = nullptr;
		if(!remove) {
			continue;
		}
		if(from->_object.size() || it->onUpdate) {
			remove = false;
			continue;
		}
		// erase if no more onUpdate subscription
		if(!it->index) {
			// no more elements!
			_beginArray = _values.end();
		}
		_object.erase(it->name.c_str());
		if(it == from) {
			// prevent from invalidation!
			++from;
		}
		it = _values.erase(it);
	}
}


const Value& Value::operator[](const char* name) const {
	auto it = _object.find(name);
	return it == _object.end() ? Null : *it->second;
}

Value& Value::operator[](const char* name) {
	// Search name
	auto itObject = _object.lower_bound(name);
	if (itObject != _object.end() && _object.key_comp()(itObject->first, name)) {
		// exists, returns it
		auto it = itObject->second;
		if(!*it) {
			// If null, move the value at the end like a new value to keep ordered the value by insertion!
			_values.splice(_beginArray, _values, it);
		}
		return *it;
	}

	// Create item
	auto it = _values.emplace(_beginArray);
	(string&)it->name = name;
	it->_pParent = this;
	_object.emplace_hint(itObject, it->name.c_str(), it);
	return *it;
}

const Value& Value::operator[](size_t index) const {
	auto it = find(index);
	return it == end() ? Null : *it;
}

Value::iterator Value::find(const char* name) {
	auto it = _object.find(name);
	return iterator(_values, it == _object.end() ? _values.end() : it->second);
}

Value::iterator Value::find(size_t index) {
	// Search index, or returns NULL value!
	auto it = _beginArray;
	while (it != _values.end() && index--) {
		++it;
	}
	return iterator(_values, it);
}

Value::iterator Value::erase(const_iterator it) {
	if(it != end()) {
		auto end = it.base();
		clearValues(it.base(), ++end);
	}
	
	return it;
}

list<Value>::iterator Value::position(size_t index) {
	auto it = _beginArray;
	if (index) {
		// Find or create the correct position!
		if(index == std::string::npos) {
			// search first null value!
			it = _values.end();
			while (it != _beginArray) {
				if(*--it) {
					++it;
					break;
				}
			}
		} else {
			auto itObj = _object.begin();
			while (index--) {
				if(it == _values.end()) {
					// create index!
					it = addItem(itObj);
				} else {
					++it;
				}
			}
		}
	}
	return it;
}

list<Value>::iterator Value::addItem(Objects::iterator& hint) {
	int32_t index = _values.empty() ? 0 : (_values.rbegin()->index+1);
	if(hint != _object.end() || (hint != _object.begin() && --hint != _object.end())) {
		// fix hint if need!
		while (hint->second->index < index && ++hint != _object.end());
	}
	auto it = _values.emplace(_values.end());
	if(!index) {
		_beginArray = it;
	}
	(int32_t&)it->index = index;
	(std::string&)it->name = String(index);
	it->_pParent = this;
	hint = _object.emplace_hint(hint, it->name.c_str(), it);
	return it;
}

bool Value::get(std::string& value) const {
	if(!self) {
        return false;
    }
    stringify(value);
    return true;
}

bool Value::get(Packet& value) const {
	if(!self) {
        return false;
    }
	if (type() == Type::BINARY) {
		value = std::get<Packet>(_value);
	} else {
		value = str();
	}
	return true;
}

/*Value& Value::set(Type type) {
	if (type == this->type())
		return self;
	// reset _integer to 0 if was string or binary!
	switch (type) {
		case Type::STRING: {
			switch (_type) {
				case Type::BOOLEAN:
					get<
				break;
			}
			break;
		}
	}
	switch (_type) {
		case ANY_STRING:
		case ANY_BINARY:
			_integer=0;
		default:;
	}
	// number!
	switch (_type = type) {
		case ANY_NUMBER: {
			if (_packet) {
				if (String::ToNumber(EXP(_packet), _integer))
					_type = String::ToNumber(EXP(_packet), _double) ? ANY_NUMBER : ANY_ARRAY;
				else
					_packet.reset();
			}
			return self;
		}
		case ANY_DATE: {
			if (_packet) {
				thread_local Date Date;
				thread_local Exception Ex;
				if (Date.update(Ex, EXP(_packet)))
					set(Date);
				else
					_packet.reset();
			}
			return self;
		}
		case ANY_BOOLEAN:; // BOOLEAN
			if(_packet && !String::IsFalse(EXP(_packet)))
				_integer = 1;
			return self;
		case ANY_NULL:
		case ANY_NONE:
			_packet.reset();
			return self;
		default:;
	}
	// String/Byte or complexe
	if (!_packet)
		_packet.set(EXPC(""));
	return self;
}*/

/*
static Value parse(const std::string& value) {
	return parse(EXP(value));
}
static Value parse(const char *data, size_t size = std::string::npos);

Value Value::parse(const char *data, size_t size) {
	int64_t number;
	Exception ex;
	if (String::tryNumber(ex, number, data, size)) {
		// Number: double or integer
		return ex.cast<Ex::Format::Incompatible>() ? String::toNumber<double>(data, size, number) : number;
	}
	if(String::IEqual(data, size, "true")) {
        return true;
    }
	if(String::IEqual(data, size, "false")) {
        return false;
    }
    if (_date.update(ex,value,_size))
		return DATE;
}*/

} // namespace Mona
