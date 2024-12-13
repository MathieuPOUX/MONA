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

#include "Mona/Format/String.h"
#include "Mona/Util/Exceptions.h"
#include <limits>
#include <cctype>

using namespace std;

namespace Mona {

#if defined(_WIN32) && (_MSC_VER < 1900)
	 // for setting number of exponent digits to 2
	static int output_exp_old_format(_set_output_format(_TWO_DIGIT_EXPONENT));
#endif

const char* String::Scoped::release() {
	char* value;
	if ((value = _value)) {
		_value = NULL;
		if (_size == string::npos) {
			delete[] value;
			value = NULL;
		} else
			value[_size] = _c;
	}
	return value;
}

void String::Scoped::init(const char* value, uint32_t size, const char* buffer, uint32_t capacity) {
	if (!value) {
		_value = NULL;
		return;
	}
	if ((value + size) < (buffer + capacity)) {
		_size = size;
		_value = (char*)value;
		_c = value[size];
	} else {
		_size = string::npos;
		_value = new char[size + 1]();
		memcpy(_value, value, size);
	}
	_value[size] = 0;
}

size_t String::Split(const char* value, size_t size, const char* separators, const String::ForEach& forEach, SPLIT_OPTIONS options) {

	const char* it(NULL);
	const char* itEnd(NULL);
	size_t count(0);

	for(;;) {
		if (options & SPLIT_TRIM)
			while (STR_AVAILABLE(value, size) && isspace(*value))
				STR_NEXT(value, size);
		it = value;
		bool available;
		while ((available=STR_AVAILABLE(it, size)) && !strchr(separators, *it))
			STR_NEXT(it, size);
			
		itEnd = it;
		if (options & SPLIT_TRIM) {
			while (itEnd-- != value && isspace(*itEnd));
			++itEnd;
		}
		if (!(options&SPLIT_IGNORE_EMPTY) || itEnd != value) {
			if(!forEach(count++, String::Scoped(value, itEnd - value, value, it - value + (available || signed(size) < 0))))
				return string::npos;
		}
		if (!available)
			break;
		value = STR_NEXT(it, size);
	}

	return count;
}

int String::ICompare(const char* data, size_t size, const char* value, size_t count) {
	if (data == value)
		return 0;
	if (!data)
		return -1;
	if (!value)
		return 1;

	int d, v;
	do {
		if (!count--)
			return 0; // no difference until here!
		if (((v = (unsigned char)(*(value++))) >= 'A') && (v <= 'Z'))
			v -= 'A' - 'a';
		if (!size--)
			return -v;
		if (((d = (unsigned char)(*(data++))) >= 'A') && (d <= 'Z'))
			d -= 'A' - 'a';
	} while (d && (d == v));
	return d - v;
}

size_t String::TrimLeft(const char*& value, size_t size) {
	if (size == string::npos)
		size = strlen(value);
	while (size && isspace(*value)) {
		++value;
		--size;
	}
	return size;
}
size_t String::TrimRight(const char* value, size_t size) {
	const char* begin(value);
	if (size == string::npos)
		size = strlen(begin);
	value += size;
	while (value != begin && isspace(*--value))
		--size;
	return size;
}

template<typename Type, uint8_t base>
bool String::tryNumber(const char* value, size_t size, Type& result)  {
	Exception ex;
	return tryNumber<Type, base>(ex, value, size, result);
}

template<typename Type, uint8_t base>
bool String::tryNumber(Exception& ex, const char* value, size_t size, Type& result) {
	STATIC_ASSERT(is_arithmetic<Type>::value);
	if (base > 36) {
		ex.set<Ex::Format>(base, " is impossible to represent with ascii table, maximum base is 36");
		return false;
	}
	bool beginning = true;
    uint8_t negative = 0;
    long double number(0);
    uint64_t comma(0);

    const char* current(value);
    while(*current && size-->0) {

		if (iscntrl(*current) || *current==' ') {
			if (!beginning) {
				// accept a partial conversion!
				ex.set<Ex::Format>(value, " is a partial number");
				break;
			}
			// trim beginning
			++current;
			continue;
      	}

		if(ispunct(*current)) {
			switch (*current++) {
				case '-':
					if (beginning) {
						negative = negative ? 0 : 1; // double -- = +
					}
				case '+':
					if (!beginning) {
						// accept a partial conversion!
						ex.set<Ex::Format>(value, " is a partial number");
						break;
					}
					continue;
				case '.':
				case ',':
					if (beginning || comma) {
						// accept a partial conversion!
						ex.set<Ex::Format>(value, " is a partial number");
						break;
					}
					comma = 1;
					continue;
				default:;
				// stop conversion!
			}
			// stop but accept a partial conversion!
			ex.set<Ex::Format>(value, " is a partial number");
			break; 
		}

      int8_t value = *current - '0';
      if (value > 9) {
        // is letter!
        if (value >= 49)
          value -= 39; // is lower letter
        else
          value -= 7; // is upper letter
      }
      if (value>=base) {
		// stop but accept a partial conversion!
		ex.set<Ex::Format>(value, " is a partial number");
        break;
	  }
   
      if (beginning) {
        beginning = false;
      }

      number = number * base + value;
      comma *= base;
      ++current;
    }

	if (beginning) {
		ex.set<Ex::Format>("Empty string is not a number");
		return false;
	}

	if (comma) {
		number /= comma;
	}

	if ((number - negative) > numeric_limits<Type>::max()) {
      // exceeds, choose to round to the max! is more accurate with an input-user parsing
      number = numeric_limits<Type>::max() + negative;
    }

	if (negative) {
      if(is_unsigned<Type>::value) {
        // exceeds, choose to round to the min! is more accurate with an input-user parsing
        number = 0;
      } else {
        number *= -1;
      }
    }

	result = (Type)number;
	return true;
}


#if defined(_WIN32)
const char* String::ToUTF8(const wchar_t* value,char buffer[PATH_MAX]) {
	WideCharToMultiByte(CP_UTF8, 0, value, -1, buffer, PATH_MAX, NULL, NULL);
	return buffer;
}
#endif


void String::ToUTF8(const char* value, size_t size, const String::OnEncoded& onEncoded) {
	const char* begin(value);
	char newValue[2];
	while(STR_AVAILABLE(value, size)) {
		if (ToUTF8(*value, newValue)) {
			STR_NEXT(value, size);
			continue;
		}

		if (value > begin)
			onEncoded(begin, value - begin);
		onEncoded(newValue, 2);

		STR_NEXT(value, size);
		begin = value;
	}

	if (value > begin)
		onEncoded(begin, value - begin);
}

const char* String::ShortPath(const string& path) {
	const char* cur(path.c_str() + path.size());
	const char* name = NULL;
	while (cur-- > path.c_str()) {
		if (*cur == '/' || *cur == '\\') {
			if (name) // end!
				break;
			name = cur;
		}
	}
	++cur;
	if (name && (IEqual(cur, name - cur, "sources") || IEqual(cur, name - cur, "mona")))
		return name + 1;
	return cur;
}

bool String::ToUTF8(char value, char (&buffer)[2]) {
	if (value >=0)
		return true;
	buffer[0] = ((U(value) >> 6) & 0x1F) | 0xC0;
	buffer[1] =(value & 0x3F) | 0x80;
	return false;
}

template bool  String::tryNumber(const char*, size_t, bool&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, bool&);
template bool  String::tryNumber(const char*, size_t, float&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, float&);
template bool  String::tryNumber(const char*, size_t, double&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, double&);
template bool  String::tryNumber(const char*, size_t, unsigned char&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, unsigned char&);
template bool  String::tryNumber(const char*, size_t, char&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, char&);
template bool  String::tryNumber(const char*, size_t, short&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, short&);
template bool  String::tryNumber(const char*, size_t, unsigned short&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, unsigned short&);
template bool  String::tryNumber(const char*, size_t, int&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, int&);
template bool  String::tryNumber(const char*, size_t, unsigned int&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, unsigned int&);
template bool  String::tryNumber(const char*, size_t, long&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, long&);
template bool  String::tryNumber(const char*, size_t, unsigned long&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, unsigned long&);
template bool  String::tryNumber(const char*, size_t, long long&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, long long&);
template bool  String::tryNumber(const char*, size_t, unsigned long long&);
template bool  String::tryNumber(Exception& ex, const char*, size_t, unsigned long long&);

} // namespace Mona
