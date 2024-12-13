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
#include "Mona/Format/BinaryReader.h"
#include "Mona/Util/Exceptions.h"
#include OpenSSL(hmac.h)
#include OpenSSL(err.h)

namespace Mona {


typedef uint8_t ROTATE_OPTIONS;
enum {
	ROTATE_INPUT = 1,
	ROTATE_OUTPUT = 2
};

struct Crypto : virtual Static {
	enum {
		MD5_SIZE = 16,
		SHA1_SIZE = 20,
		SHA256_SIZE = 32
	}; 

	static unsigned long LastError() { return ERR_get_error(); }
	static const char*   ErrorToMessage(unsigned long error) {
		if (!error)
			return "No error";
		const char* reason(ERR_reason_error_string(error));
		return reason ? reason : "Unknown error";
	}
	static const char*   LastErrorMessage() { return ErrorToMessage(LastError()); }

	static uint8_t  Rotate8(uint8_t value);
	static uint16_t Rotate16(uint16_t value);
	static uint32_t Rotate24(uint32_t value);
	static uint32_t Rotate32(uint32_t value);
	static uint64_t Rotate64(uint64_t value);

	static uint16_t ComputeChecksum(BinaryReader& reader);

	static uint32_t ComputeCRC32(const char* data, uint32_t size, ROTATE_OPTIONS options =0);


	struct Hash : virtual Static {
		static char* MD5(char* value, uint32_t size) { return Compute(EVP_md5(), value, size, value); }
		static char* MD5(const char* data, uint32_t size, char* value) { return Compute(EVP_md5(), data, size, value); }
		static char* SHA1(char* value, uint32_t size) { return Compute(EVP_sha1(), value, size, value); }
		static char* SHA1(const char* data, uint32_t size, char* value) { return Compute(EVP_sha1(), data, size, value); }
		static char* SHA256(char* value, uint32_t size) { return Compute(EVP_sha256(), value, size, value); }
		static char* SHA256(const char* data, uint32_t size, char* value) { return Compute(EVP_sha256(), data, size, value); }

		static char* Compute(const EVP_MD* evp, const char* data, uint32_t size, char* value);
	};

	struct HMAC : virtual Static {
		static char* MD5(const char* key, int keySize, char* value, uint32_t size) { return Compute(EVP_md5(), key, keySize, value, size, value); }
		static char* MD5(const char* key, int keySize, const char* data, uint32_t size, char* value) { return Compute(EVP_md5(), key, keySize, data, size, value); }
		static char* SHA1(const char* key, int keySize, char* value, uint32_t size) { return Compute(EVP_sha1(), key, keySize, value, size, value); }
		static char* SHA1(const char* key, int keySize, const char* data, uint32_t size, char* value) { return Compute(EVP_sha1(), key, keySize, data, size, value); }
		static char* SHA256(const char* key, int keySize, char* value, uint32_t size) { return Compute(EVP_sha256(), key, keySize, value, size, value); }
		static char* SHA256(const char* key, int keySize, const char* data, uint32_t size, char* value) { return Compute(EVP_sha256(), key, keySize, data, size, value); }

		static char* Compute(const EVP_MD* evp, const char* key, int keySize, const char* data, uint32_t size, char* value);
	};

};



} // namespace Mona
