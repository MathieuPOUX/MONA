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
#include "Mona/Util/Exceptions.h"
#include OpenSSL(dh.h)


namespace Mona {

struct DiffieHellman : virtual Object {
	NULLABLE(!_pDH)

	enum { SIZE = 0x80 };

	DiffieHellman() : _pDH(NULL), _publicKeySize(0), _privateKeySize(0) {}
	~DiffieHellman() { if(_pDH) DH_free(_pDH);}

	bool	computeKeys(Exception& ex);
	uint8_t	computeSecret(Exception& ex, const char* farPubKey, uint32_t farPubKeySize, char* sharedSecret);

	uint8_t	publicKeySize() const { return _publicKeySize; }
	uint8_t	privateKeySize() const { return _privateKeySize; }

	char*	readPublicKey(char* key) const;
	char*	readPrivateKey(char* key) const;

private:
	char*	readKey(const BIGNUM *pKey, char* key) const { BN_bn2bin(pKey, BIN key); return key; }

	uint8_t	_publicKeySize;
	uint8_t	_privateKeySize;

	DH*		_pDH;
};


} // namespace Mona
