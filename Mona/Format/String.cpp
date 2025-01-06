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

string& String::replace(string& str, const string& what, const string& with) {
	size_t pos = 0;
	while ((pos = str.find(what, pos)) != std::string::npos) {
		str.replace(pos, what.length(), with);
		pos += with.length();
	}
	return str;
}

template<typename Type, uint8_t base>
bool String::tryNumber(Type& result, const char* value, size_t size)  {
	Exception ex;
	return tryNumber<Type, base>(ex, result, value, size);
}

template<typename Type, uint8_t base>
bool String::tryNumber(Exception& ex, Type& result, const char* value, size_t size) {
	STATIC_ASSERT(is_arithmetic<Type>::value);
	if (base > 36) {
		ex.set<Ex::Format::Invalid>(base, " is impossible to represent with ascii table, maximum base is 36");
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
				ex.set<Ex::Format::Invalid>(value, " is a partial number");
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
						ex.set<Ex::Format::Invalid>(value, " is a partial number");
						break;
					}
					continue;
				case '.':
				case ',':
					if (beginning || comma) {
						// accept a partial conversion!
						ex.set<Ex::Format::Invalid>(value, " is a partial number");
						break;
					}
					if (!std::is_floating_point<Type>::value) {
						// accept a partial conversion!
						ex.set<Ex::Format::Incompatible>(value, " is a floating number");
						break;
					}
					comma = 1;
					continue;
				default:;
				// stop conversion!
			}
			// stop but accept a partial conversion!
			ex.set<Ex::Format::Invalid>(value, " is a partial number");
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
		ex.set<Ex::Format::Invalid>(value, " is a partial number");
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
		ex.set<Ex::Format::Invalid>("Empty string is not a number");
		return false;
	}

	if (comma) {
		number /= comma;
	}

	if ((number - negative) > numeric_limits<Type>::max()) {
      // exceeds, choose to round to the max! is more accurate with an input-user parsing
	  ex.set<Ex::Format::Incompatible>(value, " exceeds a ", typeOf<Type>());
      number = numeric_limits<Type>::max() + negative;
    }

	if (negative) {
      if(is_unsigned<Type>::value) {
        // exceeds, choose to round to the min! is more accurate with an input-user parsing
		ex.set<Ex::Format::Incompatible>(value, " cannot be fully represented with a ", typeOf<Type>());
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


bool String::ToUTF8(char value, char (&buffer)[2]) {
	if (value >=0)
		return true;
	buffer[0] = ((U(value) >> 6) & 0x1F) | 0xC0;
	buffer[1] =(value & 0x3F) | 0x80;
	return false;
}

template bool  String::tryNumber(bool&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, bool&, const char*, size_t);
template bool  String::tryNumber(float&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, float&, const char*, size_t);
template bool  String::tryNumber(double&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, double&, const char*, size_t);
template bool  String::tryNumber(unsigned char&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, unsigned char&, const char*, size_t);
template bool  String::tryNumber(char&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, char&, const char*, size_t);
template bool  String::tryNumber(short&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, short&, const char*, size_t);
template bool  String::tryNumber(unsigned short&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, unsigned short&, const char*, size_t);
template bool  String::tryNumber(int&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, int&, const char*, size_t);
template bool  String::tryNumber(unsigned int&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, unsigned int&, const char*, size_t);
template bool  String::tryNumber(long&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, long&, const char*, size_t);
template bool  String::tryNumber(unsigned long&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, unsigned long&, const char*, size_t);
template bool  String::tryNumber(long long&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, long long&, const char*, size_t);
template bool  String::tryNumber(unsigned long long&, const char*, size_t);
template bool  String::tryNumber(Exception& ex, unsigned long long&, const char*, size_t);



} // namespace Mona
