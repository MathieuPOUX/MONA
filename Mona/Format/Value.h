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
#include "Mona/Memory/Packet.h"

#include "nlohmann/json.hpp"


namespace Mona {



	struct Value : nlohmann::json {
	/*
		enum Type {
			EM = 0,
			BOOLEAN,
			INTEGER,
			DOUBLE,
			STRING,
			BINARY,
			ARRAY,
			OBJECT
		};

	
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
					return String::tryNumber(pData->data(), pData->size(), value);
				}
				case Type::binary: {
					const binary_t* pData = nlohmann::json::get_ptr<const binary_t*>();
					return String::tryNumber(STR pData->data(), pData->size(), value);
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

	};


} // namespace Mona
