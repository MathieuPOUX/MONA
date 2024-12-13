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


#include "Mona/Net/SocketAddress.h"
#include "Mona/Format/String.h"


using namespace std;

namespace Mona {


const SocketAddress& SocketAddress::Wildcard(IPAddress::Family family) {
	if (family == IPv6) {
		static SocketAddress IPv6Wildcard(IPAddress::IPv6);
		return IPv6Wildcard;
	}
	static SocketAddress IPv4Wildcard(IPAddress::IPv4);
	return IPv4Wildcard;
}
uint16_t SocketAddress::SplitLiteral(const char* value, string& host) {
	uint16_t port(0);
	host.assign(value);
	const char* colon(strchr(value, ':'));
	if (colon && String::toNumber(colon + 1, port)) // check if it's really a marker port colon (could be a IPv6 colon)
		host.resize(colon - value);
	return port;
}

bool SocketAddress::setPort(Exception& ex, const char* port) {
	uint16_t number;
	if (!String::tryNumber(ex, port, number))
		return false;
	IPAddress::setPort(number);
	return true;
}

bool SocketAddress::set(Exception& ex, const IPAddress& host, const char* port) {
	if (!setPort(ex, port))
		return false;
	IPAddress::set(host);
	return true;
}

SocketAddress& SocketAddress::set(BinaryReader& reader, Family family) {
	IPAddress::set(reader, family);
	setPort(reader.read16());
	return self;
}

bool SocketAddress::setIntern(Exception& ex, const char* host, const char* port, bool resolveHost) {
	if(!port) // to solve the ambiguitis call between set(..., const char* port) and set(..., uint16_t port) when port = 0
		return setIntern(ex, host, uint16_t(0), resolveHost);
	uint16_t number;
	if (!String::tryNumber(port, number)) {
		ex.set<Ex::Net::Address::Port>("Invalid port number ", port);
		return false;
	}
	return setIntern(ex, host, number, resolveHost);
}

bool SocketAddress::setIntern(Exception& ex, const char* hostAndPort, bool resolveHost) {
	const char* colon(strrchr(hostAndPort,':'));
	if (colon)
		return setIntern(ex, string(hostAndPort, colon - hostAndPort).c_str(), colon+1, resolveHost);
	ex.set<Ex::Net::Address::Port>("Missing port number in ", hostAndPort);
	return false;
}

bool SocketAddress::operator < (const SocketAddress& address) const {
	if (family() != address.family())
		return family() < address.family();
	if (host() != address.host())
		return host() < address.host();
	return (port() < address.port());
}


}	// namespace Mona
