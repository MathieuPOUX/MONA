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
#include "Mona/Util/Event.h"
#include <set>


namespace Mona {

struct Parameters : String::Object<Parameters> {
	typedef Event<void(const std::string& key, const std::string* pValue)> ON(Change);
	typedef Event<void()>												   ON(Clear);
	typedef Event<const std::string*(const std::string& key)>			   ON(Unfound);
	
	static bool ParseIniFile(const std::string& path, Parameters& parameters);

	struct Key : String, virtual Object {
		template<typename ...Args>
		Key(std::vector<uint32_t>& parts, Args&&... args) : _pParts(&parts), String(std::forward<Args>(args)...) {}
		template<typename ...Args>
		Key(Args&&... args) : _pParts(NULL), String(std::forward<Args>(args)...) {}
		Key& operator=(Key&& key);
		
		const char*						name() const { return parts().rbegin()->second.data(); }
		const std::map<uint32_t, Packet>& parts() const;

	private:
		mutable std::map<uint32_t, Packet>	_parts;
		mutable std::vector<uint32_t>*		_pParts;
	};

	typedef std::function<bool(Parameters::Key&, const Packet&)> OnItem;
	struct Parser : virtual Object {
		char separator = ';';
		char equality = '=';
		char subKey = '.';
		bool uriChars = false;

		uint32_t	 operator()(const Packet& value, const OnItem& onItem);

		template<typename MapType, typename = typename std::enable_if<is_container<MapType>::value, MapType>::type>
		MapType& operator()(const Packet& value, MapType& map) {
			self(value, [&map](Parameters::Key& key, const Packet& value) {
				map.emplace(std::move(key), value);
				return true;
			});
			return map;
		}
	};
	
	typedef std::map<std::string, std::string, String::IComparator>::const_iterator const_iterator;
	typedef std::map<std::string, std::string, String::IComparator>::key_compare key_compare;

	struct ForEach {
		ForEach(const const_iterator& begin, const const_iterator& end) : _begin(begin), _end(end) {}
		const_iterator		begin() const { return _begin; }
		const_iterator		end() const { return _end; }
	private:
		const_iterator  _begin;
		const_iterator  _end;
	};

	Parameters() {}
	Parameters(Parameters&& other) { setParams(std::move(other));  }
	Parameters& setParams(Parameters&& other);

	const Parameters& parameters() const { return self; }

	const_iterator	begin() const { return params().begin(); }
	const_iterator	end() const { return params().end(); }
	const_iterator  lower_bound(const std::string& key) const { return params().lower_bound(key); }
	const_iterator  find(const std::string& key) const { return params().find(key); }
	ForEach			from(const std::string& prefix) const { return ForEach(params().lower_bound(prefix), params().end()); }
	ForEach			range(const std::string& prefix) const;
	uint32_t			count() const { return params().size(); }


	const Time&		timeChanged() const { return _timeChanged; }


	Parameters&		clear(const std::string& prefix = String::Empty());

	/*!
	Return false if key doesn't exist (and don't change 'value'), otherwise return true and assign string 'value' */
	bool		getString(const std::string& key, std::string& value) const;
	/*!
	Return false if key doesn't exist or if it's not a numeric type, otherwise return true and assign numeric 'value' */
	template<typename Type>
	bool getNumber(const std::string& key, Type& value) const {
		STATIC_ASSERT(std::is_arithmetic<Type>::value); const std::string* pValue = getParameter(key);
		return pValue && String::tryNumber(*pValue, value);
	}
	/*!
	Return false if key doesn't exist or if it's not a boolean type, otherwise return true and assign boolean 'value' */
	bool getBoolean(const std::string& key, bool& value) const;

	/*!
	A typed return of get */
	template<typename NumberType, typename = typename std::enable_if<std::is_arithmetic<NumberType>::value>::type>
    NumberType get(const std::string& key, NumberType defaultValue = 0) const {
		NumberType result((NumberType)defaultValue);
		getNumber(key, result);
		return result;
	}
	/*!
	A short version of getString with default argument to get value by returned result */
	std::string get(const std::string& key, const std::string& defaultValue = "") const;

	bool hasKey(const std::string& key) const { return getParameter(key) != NULL; }

	bool erase(const std::string& key);
	const_iterator erase(const_iterator first, const_iterator last);
	const_iterator erase(const_iterator it) { return erase(it, ++it); }

	template<typename KeyType, typename ...Args>
	const std::string& setString(KeyType&& key, Args&&... args) { return setParameter(std::forward<KeyType>(key), std::forward<Args>(args) ...); }
	template<typename KeyType, typename Type>
	Type setNumber(KeyType&& key, Type value) { STATIC_ASSERT(std::is_arithmetic<Type>::value); emplace(std::forward<KeyType>(key), String(value)); return value; }
	template<typename KeyType>
	bool setBoolean(KeyType&& key, bool value) { emplace(std::forward<KeyType>(key), value ? "true" : "false");  return value; }

	const std::string* getParameter(const std::string& key) const;
	template<typename KeyType, typename ...Args>
	const std::string& setParameter(KeyType&& key, Args&&... args) { return emplace(std::forward<KeyType>(key), std::string(std::forward<Args>(args) ...)).first->second; }
	/*!
	Just to match STD container (see MapWriter) */
	template<typename KeyType, typename ...Args>
	std::pair<const_iterator, bool> emplace(KeyType&& key, Args&&... args) { return emplace(std::forward<KeyType>(key), std::string(std::forward<Args>(args) ...)); }
	template<typename KeyType>
	std::pair<const_iterator, bool> emplace(KeyType&& key, std::string&& value) {
		if (!_pMap)
			_pMap.set();
		const auto& it = _pMap->emplace(std::forward<KeyType>(key), std::string());
		//std::string value(std::forward<Args>(args) ...);
		if (it.second || value != it.first->second) {
			it.first->second = std::move(value);
			onParamChange(it.first->first, &it.first->second);
		}
		return it;
	}


	static const Parameters& Null() { static Parameters Null(nullptr); return Null; }

protected:
	Parameters(const Parameters& other) { setParams(other); }
	Parameters& setParams(const Parameters& other);

	virtual void onParamChange(const std::string& key, const std::string* pValue) { _timeChanged.update(); onChange(key, pValue); }
	virtual void onParamClear() { _timeChanged.update(); onClear(); }

private:
	virtual const std::string* onParamUnfound(const std::string& key) const { return onUnfound(key); }
	virtual void onParamInit() {}

	const std::map<std::string, std::string, String::IComparator>& params() const { if (!_pMap) ((Parameters&)self).onParamInit(); return _pMap ? *_pMap : *Null()._pMap; }


	Parameters(std::nullptr_t) : _pMap(SET) {} // Null()

	Time _timeChanged;

	// shared because a lot more faster than using st::map move constructor!
	// Also build _pMap just if required, and then not erase it but clear it (more faster that reset the shared)
	Shared<std::map<std::string, std::string, String::IComparator>>	_pMap;
};



} // namespace Mona

