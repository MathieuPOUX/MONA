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

#include "Mona/Util/Parameters.h"
#include "Mona/Disk/File.h"

using namespace std;

namespace Mona {

Parameters& Parameters::setParams(const Parameters& other) {
	// clear self!
	clear();
	// copy data
	if (other.count())
		_pMap.set() = *other._pMap;
	// onChange!
	for (auto& it : self)
		onParamChange(it.first, &it.second);
	return self;
}

Parameters& Parameters::setParams(Parameters&& other) {
	// clear self!
	clear();
	// move data
	if(other.count()) {
		_pMap = move(other._pMap);
		// clear other
		other.onParamClear();
	}	
	// onChange!
	for (auto& it : self)
		onParamChange(it.first, &it.second);
	return self;
}

Parameters::ForEach Parameters::range(const string& prefix) const {
	if(prefix.empty())
		return ForEach(begin(), end());
	string end(prefix);
	end.back() = prefix.back() + 1;
	return ForEach(lower_bound(prefix), lower_bound(end));
}

bool Parameters::getString(const string& key, string& value) const {
	const string* pValue = getParameter(key);
	if (!pValue)
		return false;
	value.assign(*pValue);
	return true;
}

std::string Parameters::get(const string& key, const string& defaultValue) const {
	const string* pValue = getParameter(key);
	return pValue ? *pValue : defaultValue;
}

bool Parameters::getBoolean(const string& key, bool& value) const {
	const string* pValue = getParameter(key);
	if (!pValue)
		return false;
	value = !String::IsFalse(*pValue); // otherwise considerate the value as true
	return true;
}

const string* Parameters::getParameter(const string& key) const {
	const auto& it = params().find(key);
	if (it != params().end())
		return &it->second;
	return onParamUnfound(key);
}

Parameters& Parameters::clear(const string& prefix) {
	if (!count())
		return self;
	if (prefix.empty()) {
		_pMap.reset();
		onParamClear();
	} else {
		string end(prefix);
		end.back() = prefix.back() + 1;
		erase(_pMap->lower_bound(prefix), _pMap->lower_bound(end));
	}
	return self;
}

bool Parameters::erase(const string& key) {
	// erase
	const auto& it(params().find(key));
	if (it != params().end()) {
		// move key because "key" parameter can be a "it->first" too, and must stay valid for onParamChange call!
		string key(move(it->first));
		_pMap->erase(it);
		if (_pMap->empty())
			clear();
		else
			onParamChange(key, NULL);
		return true;
	}
	return false;
}
Parameters::const_iterator Parameters::erase(const_iterator first, const_iterator last) {
	if (first == begin() && last == end()) {
		clear();
		return end();
	}
	const_iterator it;
	for (it = first; it != last;) {
		string key(move(it->first));
		it = _pMap->erase(it);
		onParamChange(key, NULL);
	}
	return it;
}


bool Parameters::ParseIniFile(const string& path, Parameters& parameters) {
	Exception ex;
	File file(path, File::MODE_READ);
	if (!file.load(ex))
		return false;
	uint32_t size = Mona::range<uint32_t>(file.size());
	if (size == 0)
		return true;
	Buffer buffer(size);
	if (file.read(ex, buffer.data(), size) < 0)
		return false;

	char* cur = buffer.data();
	const char* end = cur + size;
	const char* key, *value;
	Packet section;
	while (cur < end) {
		// skip space
		while (isspace(*cur) && ++cur < end);
		if (cur == end)
			break;

		// skip comments
		if (*cur == ';') {
			while (++cur < end && *cur != '\n');
			++cur;
			continue;
		}

		// line
		key = cur;
		value = NULL;
		size_t vSize(0), kSize(0);
		const char* quote = NULL;
		do {
			if (*cur == '\n')
				break;
			if (*cur == '\'' || *cur == '"') {
				if (quote) {
					if (*quote == *cur && quote < value) {
						// fix value!
						kSize += vSize + 1;
						value = NULL;
						vSize = 0;
					} // else was not a quote!
					quote = NULL;
				} else
					quote = cur;
			}
			if (value)
				++vSize;
			else if (*cur == '=')
				value = cur + 1;
			else
				++kSize;
		} while (++cur < end);

		if (vSize)
			vSize = String::Trim(value, vSize);

		bool isSection = false;
		if (kSize) {
			kSize = String::TrimRight(key, kSize);

			if (*key == '[' && ((vSize && value[vSize - 1] == ']') || (!value && key[kSize - 1] == ']'))) {
				// section
				// remove [
				--kSize;
				++key;
				// remove ]
				if (value)
					--vSize;
				else
					--kSize;
				isSection = true;
			}

			// remove quote on key
			if (kSize>1 && key[0] == key[kSize - 1] && (key[0] == '"' || key[0] == '\'')) {
				kSize -= 2;
				++key;
			}
		}

		if (vSize) {
			vSize = String::Trim(value, vSize);
			// remove quote on value
			if (vSize > 1 && value[0] == value[vSize - 1] && (value[0] == '"' || value[0] == '\'')) {
				vSize -= 2;
				++value;
			}
		}

		if (isSection) {
			parameters.setString(String(section.set(key, kSize)), value, vSize);
		} else
			parameters.setString(String(section, '.', Packet(key, kSize)), value, vSize);
	}

	return true;
}

Parameters::Key& Parameters::Key::operator=(Key&& key) {
	_pParts = NULL;
	_parts = move(key.parts());
	string::operator=(move(key));
	return self;
}

const map<uint32_t, Packet>& Parameters::Key::parts() const {
	uint32_t offset;
	if (_pParts) {
		for (uint32_t part : *_pParts) {
			if (part < size()) {
				auto it = _parts.emplace_hint(_parts.end(), SET, forward_as_tuple(part), forward_as_tuple());
				Packet& packet = it->second;
				offset = it == _parts.begin() ? 0 : ((--it)->first + 1);
				packet.set(data() + offset, part - offset);
			}
		}
		offset = _parts.empty() ? 0 : (_parts.rbegin()->first + 1);
		_pParts = NULL;
	} else {
		if (!_parts.empty() && _parts.rbegin()->first == size() && _parts.begin()->second.data() == data())
			return _parts; // no change, size is good, and data pointer un changed!
		offset = 0;
		auto it = _parts.begin();
		while (it != _parts.end() && it->first < size()) {
			offset += it->second.set(data() + offset, it->first - offset).size() + 1;
			++it;
		}
		_parts.erase(it, _parts.end());
	}
	_parts.emplace_hint(_parts.end(), SET, forward_as_tuple(size()), forward_as_tuple(data() + offset, size() - offset));
	return _parts;
}


static void DecodeURI(string& buffer, char lo) {
	char hi = buffer.back();
	buffer.resize(buffer.size() - 2); // remove %+hi
	String::FromURI(hi, lo, buffer);
}
uint32_t Parameters::Parser::operator()(const Packet& packet, const OnItem& onItem) {
	uint32_t count = 0;
	// Read pair key=value
	vector<uint32_t> keyParts;
	string key;
	const char* data = packet.data();
	const char* value = NULL;
	uint8_t decoding = 0;
	string decoded;
	do {
		if (data==packet.end() || *data == separator) {
			Packet v;
			if (!value) {
				if (key.empty())
					continue; // nothing to read!
			} else {
				if (!decoded.empty())
					v.set(String::TrimRight(decoded));
				else
					v.set(packet, value, String::TrimRight(value, data - value));
				value = NULL;
			}
			++count;
			Key k(keyParts, move(String::TrimRight(key)));
			if (!onItem(k, v))
				return count; // stopped by user!
			decoding = 0;
			key.clear();
			keyParts.clear();
			continue;
		}

		if (!isblank(*data)) {
			if (!value) {
				// parsing key!
				if (*data == equality)
					value = data + 1;
				else if (*data == subKey)
					keyParts.emplace_back(key.size());
			}

			if (uriChars) {
				if (*data == '%') {
					if(value && decoded.empty())
						decoded.assign(value, data - value);
					decoding = 1; // signal that next 2 chars are uri encoded
				} else if (decoding && ++decoding > 2) {
					if (value)
						DecodeURI(decoded, *data);
					else
						DecodeURI(key, *data);
					decoding = 0;
					continue; // skip this value to decoded string!
				}
			}
		} else if (decoding)
			decoding = 0;		

		if (value) {
			if (decoding || !decoded.empty())
				decoded += *data;
		} else
			key += *data;

	} while(data++ != packet.end());

	return count;
}



} // namespace Mona
