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


#include "Mona/Format/URL.h"


using namespace std;

namespace Mona {

const char* URL::Parse(const char* url, size_t& size, string& protocol, string& address) {
	protocol.clear();
	address.clear();

	// Fill protocol and address!
	/// level = 0 => protocol
	/// level = 1-3 => ://
	/// level = 3 => address
	const char* cur = url;
	size_t rest = size;
	uint8_t level(0);
	bool trimLeft = true;
	while(rest) {
		switch (*cur) {
			case '?':
				// query now
				if (level) {
					level = 4;
					break;
				}
				// just query!
			case 0:
				rest = 0;
				continue;
			case '\\':
			case '/':
				if (!level) {// just path!
					rest = 0;
					continue;
				}
				++level;
				break;
			case ':':
				if (!level) {
					address.clear(); // usefull if methods is using with the same string reference for protocol + address
					level = 1;
					break;
				}
			default: /// other char!
				if (level) {
					level = 3;
					address += *cur;
				} else {
					if (trimLeft) {
						if(isspace(*cur))
							break;
						trimLeft = false;
					}
					protocol += *cur;
				}
					
		}
		if (level > 3)
			break;
		++cur;
		if(rest !=string::npos)
			--rest;
	};

	if (level) { // => has gotten a protocol!
		// rest is request
		// cur must be a relative path excepting if file protocol
		url = cur;
		while (rest && (*cur == '/' || *cur == '\\')) {
			++cur;
			if (rest != string::npos)
				--rest;
		}
		if (url != cur && protocol == "file") {
			// always absolute if file!
			--cur;
			if (rest != string::npos)
				++rest;
		}
		size = rest;
		url = cur;
	} else { // url is just the request
		protocol.clear();
		address.clear();
	}
	if (size == string::npos)
		size = strlen(url);
	return url; // return request
}


const char* URL::ParseRequest(const char* request, size_t& size, string& path, REQUEST_OPTIONS options) {
	// Decode PATH

	bool relative;
	while (!(relative = !STR_AVAILABLE(request, size) || (*request != '/' && *request != '\\')) && (options & REQUEST_FORCE_RELATIVE))
		STR_NEXT(request, size);

	// allow to fix /c:/ to c:/ (and don't impact linux, if was already absolute it removes just the first slash)
	if (!relative && FileSystem::IsAbsolute(request+1))
		STR_NEXT(request, size);

	/// level = 0 => path => next char
	/// level = 1 => path => /
	/// level > 1 => path => . after /
	uint8_t level(0);
	vector<size_t> slashs;
	path.clear();
	uint8_t decoding = 0;
	while(STR_AVAILABLE(request, size)) {

		if (*request == '?')
			break; // query now!

				   // path
		if (*request == '/' || *request == '\\') {
			// level + 1 = . level
			if (level > 2) {
				// /../
				if (!slashs.empty()) {
					path.resize(slashs.back());
					slashs.pop_back();
				} else
					path.clear();
			}
			level = 1;
		} else do {
			if (level) {
				// level = 1 or 2
				if (level<3 && *request == '.') {
					++level;
					break;
				}
				slashs.emplace_back(path.size());
				path.append("/..", level);
				level = 0;
			}
			// Add current character
			if (*request == '%')
				decoding = 1; // signal that next 2 chars are uri encoded
			else if (decoding && ++decoding > 2) {
				char hi = path.back();
				path.resize(path.size() - 2); // remove %+hi
				String::FromURI(hi, *request, path);
				decoding = 0;
				break; // skip this value to decoded string!
			}
			path += *request;
		} while (false);

		STR_NEXT(request, size);
	};

	// Get size query!
	size = signed(size)>0 ? size : strlen(request);

	if(level) { // is folder
		if (level > 2) // /..
			path.resize(slashs.empty() ? 0 : slashs.back());
		path += '/';
	} else if(options & REQUEST_MAKE_FOLDER)
		path += '/';
	if (relative)
		MAKE_RELATIVE(path);
	return request; // returns query!
}

const char* URL::ParseRequest(const char* request, std::size_t& size, Path& path, REQUEST_OPTIONS options) {
	string strPath;
	request = ParseRequest(request, size, strPath, options);
	path = move(strPath);
	return request; // returns query!
}


uint32_t URL::ParseQuery(const Packet& query, const Parameters::OnItem& onItem) {
	Parameters::Parser Parser;
	Parser.uriChars = true;
	Parser.separator = '&';
	// skip first ?
	if (query.empty())
		return 0;
	if (query[0] == '?' && query.size()==1)
		return 0;
	return Parser(query, onItem);
}



} // namespace Mona
