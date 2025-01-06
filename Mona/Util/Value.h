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
#include "Mona/Format/String.h"
#include "Mona/Format/Base64.h"
#include "Mona/Memory/Packet.h"
#include "Mona/Util/Event.h"
#include "Mona/Util/Iterator.h"
#include "Mona/Timing/Date.h"
#include <variant>
#include <list>


namespace Mona {

/**
 * Generic Value
 *
 * Object: fields in first, array in second!
 */
struct Value : virtual Object, Format<Value> {
	typedef Event<void(const Value& value)> OnUpdate; mutable OnUpdate onUpdate;

	NULLABLE(type() == Type::NONE);


	struct IteratorBase {
		IteratorBase(std::list<Value>& values);
		IteratorBase(std::list<Value>& values, std::list<Value>::iterator it);
		Value& item();
		void increment();
		void decrement();
		bool equal(const IteratorBase& other) const;
		std::list<Value>::iterator base() { return _it; }

	  private:
		void skipNulls() { skipNulls(end()); }
		void skipNulls(std::list<Value>::iterator end);
		std::list<Value>::iterator  end();
		std::list<Value>::iterator 	_it;
		std::list<Value>& 			_values;
	};
	MAKE_ITERABLE(IteratorBase, (_values), (_values, _values.end()));

	/**
	 * Type of Value
	 */
	enum Type {
		NONE = 0,
		BOOLEAN = 1,
		INTEGER = 2,
		DOUBLE = 3,
		DATE = 4,
		STRING = 5,
		BINARY = 6,
		OBJECT = 7
	};

	/**
	 * Construct a null value
	 */
	Value() : _value(nullptr), _pParent(NULL), _beginArray(_values.end()), index(-1) {}

	/**
	 * Construct from a Value copy, explicit because can be expensive
	 */
	explicit Value(const Value& other) : _value(nullptr), _pParent(NULL), _beginArray(_values.end()), index(-1)  {
		set(other);
	}

	/**
	 * Constructor with init value like parameter
	 */
	template <typename Type>
	Value(std::initializer_list<Type> list) : _pParent(NULL), _beginArray(_values.end()), index(-1) { set(std::move(list)); }

	/**
	 * Constructor with init value like parameter
	 */
	Value(std::initializer_list<std::pair<const char*, Value>> map) : _pParent(NULL), _beginArray(_values.end()), index(-1) { set(std::move(map)); }

	/**
	 * Constructor with init value like parameter
	 */
	template <typename ...Args>
	Value(Args&&... args) : _pParent(NULL), _beginArray(_values.end()), index(-1) { set(std::forward<Args>(args)...); }

	/**
	 * Index of the value when stored by index like an array
	 */
	const int32_t 		index;

	/**
	 * Name of the value when stored by field like an object
	 */
	const std::string 	name;

	/**
	 * Type of the value
	 */
	Type type() const { return Type(_value.index()); }

	/**
	 * Operator assignation
	 */
	template <typename Type>
	Value& operator=(Type&& value) {
		return set(std::forward<Type>(value));
	}

	/**
	 * Assignation by copy
	 */
	Value& operator=(const Value& other) { return set(other); }

	/**
	 * Reset value
	 */
	Value& set(nullptr_t) { return update(nullptr); }

	/**
	 * Reset value
	 */
	Value& reset() { return update(nullptr); }

	/**
	 * Set from other value by copy
	 */
	Value& set(const Value& other);

	/**
	 * Set value from number or boolean
	 */
	template <typename Number, typename = typename std::enable_if<std::is_arithmetic<Number>::value, Number>::type>
	Value& set(Number number) {
		return update(number);
	}

	/**
	 * Set value from date
	 */
	Value& set(const Date& date) { return update(date); }

	/**
	 * Set value from string
	 */
	Value& set(std::string value) { return update(std::move(value)); }

	/**
	 * Set value from string
	 */
	Value& set(const char* value) { return set(std::string(value)); }

	/**
	 * Set value from binary (without copy, if you prefer copy opt rather to the string version)
	 */
	Value& set(const Packet& value) { return update(Packet(value)); }

	/**
	 * Set value from map
	 */
	template<typename Map, typename std::enable_if_t<is_map<Map>::value, int> = 0>
    Value& set(const Map& map) {
		// clear all values, we cannot reuse any values here
		clearValues();
		return fill(map);
	}

	/**
	 * Set value from array
	 */
	template<typename List, typename std::enable_if_t<is_list<List>::value, int> = 0>
	Value& set(const List& list) {
		// Clear object part
		clearValues(_values.begin(), _beginArray);
		return fill(list, 0);
	}

	/**
	 * Set value from an initializer_list array
	 */
	template<typename Type>
	Value& set(std::initializer_list<Type> list) {
		return set<std::initializer_list<Type>>(list);
	}

	/**
	 * Set value from an initializer_list map
	 */
	Value& set(std::initializer_list<std::pair<const char*, Value>> map) {
		// clear all values, we cannot reuse any values here
		clearValues();
		return fill(std::move(map));
	}

	/**
	 * Fill object properties from the provided initializer_list
	 */
    Value& fill(std::initializer_list<std::pair<const char*, Value>> map) {
		for (const auto& [key, value] : map) {
			self[key] = value;
		}
		return update(Object{});
	}

	/**
	 * Fill object properties from the provided map
	 */
	template<typename Map, typename std::enable_if_t<is_map<Map>::value, int> = 0>
    Value& fillMap(const Map& map) {
		for (const auto& [key, value] : map) {
			self[key] = value;
		}
		return update(Object{});
	}

	/**
	 * Fill array elements from the provided list to the end of the container by default,
	 * or fill them at the specified position (override existing).
	 */
	template<typename List, typename std::enable_if_t<is_list<List>::value, int> = 0>
	Value& fill(const List& list, size_t position = std::string::npos) {
		auto it = this->position(position);
		auto itEndValid = it;
		auto itObj = _object.begin();
		for (const auto& item : list) {
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
		return update(Object{});
	}

	/**
	 * Search or create a object property from its name
	 */
	Value& operator[](const char* name);

	/**
	 * Search or create a object property from its name
	 */
	Value& operator[](const std::string& name) { return self[name.c_str()]; }

	/**
	 * Search a object property from its name, returns a Null Value if unfound
	 */
	const Value& operator[](const char* name) const;

	/**
	 * Search a object property from its name, returns a Null Value if unfound
	 */
	const Value& operator[](const std::string& name) const { return self[name.c_str()]; }

	/**
	 * Search or create a object property from its index
	 */
	Value& operator[](size_t index) { return *this->position(index); }

	/**
	 * Search or create a object property from its index
	 */
	const Value& operator[](size_t index) const;

	/**
	 * Search an object property from its name
	 */
	const_iterator find(const std::string& name) const { return find(name.c_str()); }
	/**
	 * Search an object property from its name
	 */
	iterator find(const std::string& name) { return find(name.c_str()); }

	/**
	 * Search an object property from its name
	 */
	const_iterator find(const char* name) const { return ((Value*)this)->find(name); }
	/**
	 * Search an object property from its name
	 */
	iterator find(const char* name);

	/**
	 * Search an array element from its index
	 */
	const_iterator find(size_t index) const  { return ((Value*)this)->find(index); }
	/**
	 * Search an array element from its index
	 */
	iterator find(size_t index);

	/**
	 * Erase an object property from its name
	 */
	size_t erase(const std::string& name) { return erase(name.c_str()); }

	/**
	 * Erase an object property from its name
	 */
	size_t erase(const char* name) { return erase(find(name))!=end() ? 1 : 0; }

	/**
	 * Erase an array element from its index
	 */
	size_t erase(size_t index) { return erase(find(index))!=end() ? 1 : 0; }

	/**
	 * Erase an array element from its position
	 */
	iterator erase(const_iterator it);

	/**
	 * Operator of conversion
	 */
	template<typename ValueType>
	operator ValueType() const {
		return get<ValueType>();
	}

	/**
	 * Operator of comparison
	 */
	bool operator==(const Value& other) const;

	/**
	 * Operator of comparison
	 */
	bool operator!=(const Value& other) const { return !(operator==(other)); }

	/**
	 * Read the value as a string, returns true on success
	 */
	bool get(std::string& value) const;

	/**
	 * Read and returns the value as a string, or return defaultValue if it fails
	 */
	template<typename ValueType, typename = typename std::enable_if<std::is_same<ValueType, std::string>::value>::type>
	ValueType get(const std::string_view& defaultValue = "") const {
		std::string value;
		if(get(value)) {
			return value; // move
		}
		return std::string(defaultValue); // copy
	}
	
	/**
	 * Read and returns a const char* (without copy) only if value is a string, return defaultValue otherwise
	 */
	template<typename ValueType, typename = typename std::enable_if<std::is_same<ValueType, const char*>::value>::type>
	ValueType get(const char* defaultValue = NULL) const {
		const char* value = std::get_if<std::string>(&_value);
		return value ? value : defaultValue;
	}

	/**
	 * Read the value as a binary, returns true on success
	 */
	bool get(Packet& value) const;

	/**
	 * Read and returns the value as a binary, or return defaultValue if it fails
	 */
	template <typename ValueType, typename = typename std::enable_if<std::is_same<ValueType, Packet>::value>::type>
	ValueType get(const Packet& defaultValue = "") const {
		Packet value;
		if(get(value)) {
			return value; // move
		}
		return defaultValue; // copy
	}


	template<typename OutType>
	void stringify(OutType& out) const {
        switch (type()) {
            case NONE:
                return;
            case BOOLEAN:
				String::append(out, std::get<bool>(_value));
                return;
			case INTEGER:
                String::append(out, std::get<int64_t>(_value));
                return;
			case DOUBLE:
                String::append(out, std::get<double>(_value));
                return;
			case DATE:
                String::append(out, std::get<Date>(_value));
                return;
			case STRING:
                String::append(out, std::get<std::string>(_value));
                return;
			case BINARY:
                String::append(out, Base64(std::get<Packet>(_value)));
                return;
			case OBJECT:
                // TODO : JSON by default!
                return;
        }
    }

  private:
	using Objects = std::map<const char*, std::list<Value>::iterator, String::IComparator>;
	
	std::list<Value>::iterator addItem(Objects::iterator& hint);

	std::list<Value>::iterator position(size_t index);

	template <typename Type>
	Value& update(Type&& value) {
		bool wasObject = type() == OBJECT;
		_value = std::forward<Type>(value);
		if(wasObject && type() != OBJECT) {
			// object to primitive => clear object/array!
			clearValues();
		}
		return updateValue(self);
	}
	Value& updateValue(const Value& value);

	void clearValues() { clearValues(_values.begin(), _values.end()); }
	void clearValues(std::list<Value>::iterator from) { clearValues(from, _values.end()); }
	void clearValues(std::list<Value>::iterator from, std::list<Value>::iterator end);


	struct Object {
		// dummy comparator, a more complexe comparison is done with _values/_object
		bool operator==(const Object& other) const { return true; }
	};
	std::variant <
		std::nullptr_t,                // NONE
		bool,                          // BOOLEAN
		int64_t,                       // INTEGER
		double,                        // DOUBLE
		Date,						   // DATE
		std::string,                   // STRING
		Packet,						   // BINARY
		Object						   // Object/Array into _items/_map
	> 								_value;
	Value* 							_pParent;
	std::list<Value> 				_values;
	std::list<Value>::iterator 		_beginArray;
	Objects 						_object;
};

		/*
	
		/// helper type for initializer lists of basic_json values
    	using InitializerList = std::initializer_list<nlohmann::detail::json_ref<Value>>;

		template <typename ...Args>
		Value(Args&&... args) : nlohmann::json(std::forward<Args>(args)...) {
			int i = 0;
		}

		Value(InitializerList init) {
			bool is_an_object = std::all_of(init.begin(), init.end(), [](const auto& element_ref) {
				// The cast is to ensure op[size_type] is called, bearing in mind size_type may not be int;
				// (many string types can be constructed from 0 via its null-pointer guise, so we get a
				// broken call to op[key_type], the wrong semantics and a 4804 warning on Windows)
				return element_ref->is_array() && element_ref->size() == 2 && (*element_ref)[static_cast<size_type>(0)].is_string();
			});

			if (is_an_object) {
				// the initializer list is a list of pairs -> create object
				m_data.m_type = value_t::object;
				m_data.m_value = value_t::object;

				for (auto& element_ref : init)
				{
					auto element = element_ref.moved_or_copied();
					m_data.m_value.object->emplace(
						std::move(*((*element.m_data.m_value.array)[0].m_data.m_value.string)),
						std::move((*element.m_data.m_value.array)[1]));
				}
			}
			else
			{
				// the initializer list describes an array -> create array
				m_data.m_type = value_t::array;
				m_data.m_value.array = std::vector<nlohmann::json_abi_v3_11_3::json, std::allocator<nlohmann::json_abi_v3_11_3::json>>(init.begin(), init.end());
			}

		}

		void push_back(InitializerList init) {
			if (is_object() && init.size() == 2 && (*init.begin())->is_string()){
				Value&& key = init.begin()->moved_or_copied();
				nlohmann::json::push_back(typename object_t::value_type(
							std::move(key.get_ref<string_t&>()), (init.begin() + 1)->moved_or_copied()));
			}
			else {
				nlohmann::json::push_back(Value(init));
			}
		}


		template<typename ValueType>
		operator ValueType() const {
			return get<ValueType>();
		}

		// Read a string
		bool get(std::string& value) const {
			return getData(value);
		}
		template<typename ValueType, typename = typename std::enable_if<std::is_same<ValueType, std::string>::value>::type>
		ValueType get(const std::string& defaultValue = "") const {
			std::string value;
			return get(value) ? value : defaultValue;
		}

		// Read a binary
		bool get(Packet& value) const {
			return getData(value);
		}
		template<typename ValueType, typename = typename std::enable_if<std::is_same<ValueType, Packet>::value>::type>
		ValueType get(const Packet& defaultValue = "") const {
			Packet value = Packet(defaultValue);
			get(value);
			return value;
		}
		

		// Read a number
		template<typename ValueType, typename = typename std::enable_if<std::is_arithmetic<ValueType>::value, ValueType>::type>
		bool get(ValueType& value) const {
			switch (type()) {
				case Type::boolean:
					value = nlohmann::json::get<bool>() ? 1 : 0;
					return true;
				case Type::number_integer:
					value = range<ValueType>(nlohmann::json::get<int64_t>());
					return true;
				case Type::number_unsigned:
					value = range<ValueType>(nlohmann::json::get<uint64_t>());
					return true;
				case Type::number_float:
					value = range<ValueType>(nlohmann::json::get<double>());
					return true;
				case Type::string: {
					const string_t* pData = nlohmann::json::get_ptr<const string_t*>();
					return String::tryNumber(value, pData->data(), pData->size());
				}
				case Type::binary: {
					const binary_t* pData = nlohmann::json::get_ptr<const binary_t*>();
					return String::tryNumber(value, STR pData->data(), pData->size());
				}
				default:;
			}
			// Cannot convert = no value
			// Type::object, Type::array, Type::discarded, Type::null
			return false;
		}
		template<typename ValueType, typename DefaultValueType = ValueType, typename = typename std::enable_if<std::is_arithmetic<ValueType>::value, ValueType>::type>
		ValueType get(DefaultValueType defaultValue = 0) const {
			get(defaultValue);
			return defaultValue;
		}

	private:

		template<typename DataType>
		bool getData(DataType& data) const {
			switch (type()) {
				case Type::boolean:
					data = nlohmann::json::get<bool>() ? "true" : "false";
					return true;
				case Type::number_integer:
					data = String(nlohmann::json::get<int64_t>());
					return true;
				case Type::number_unsigned:
					data = String(nlohmann::json::get<uint64_t>());
					return true;
				case Type::number_float:
					data = String(nlohmann::json::get<double>());
					return true;
				case Type::binary: {
					const auto& value = nlohmann::json::get_binary();
					data.assign(STR value.data(), value.size());
					return true;
				}
				case Type::string:
					data = nlohmann::json::get<std::string>();
					return true;
				default: break;
			}
			// Cannot convert = no value
			// Type::object, Type::array, Type::discarded, Type::null
			return false;
		}*/

		//Type _type;


} // namespace Mona
