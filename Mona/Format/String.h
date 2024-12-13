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
#include "Mona/Memory/Packet.h"
#include "Mona/Timing/Date.h"
#include <functional>

#undef max

namespace Mona {

// length because size() can easly overloads an other inheriting method
// no empty because empty() can easly overloads an other inheriting method
#define CONST_STRING(STRING)	operator const std::string&() const { return STRING;} \
								std::size_t length() const { return (STRING).length(); } \
								const char& operator[] (std::size_t pos) const { return (STRING)[pos]; } \
								const char& back() const { return (STRING).back(); } \
								const char& front() const { return (STRING).front(); } \
								const char* c_str() const { return (STRING).c_str(); }

struct Exception;

typedef uint8_t SPLIT_OPTIONS;
enum {
	SPLIT_IGNORE_EMPTY = 1, /// ignore empty tokens
	SPLIT_TRIM = 2,  /// remove leading and trailing whitespace from tokens
};


typedef uint8_t HEX_OPTIONS;
enum {
	HEX_CPP = 1,
	HEX_TRIM_LEFT = 2,
	HEX_UPPER_CASE = 4
};


/// Utility class for generation parse of strings
struct String : std::string {
	NULLABLE(empty())

	/*!
	Object formatable, must be iterable with key/value convertible in string */
	template<typename Type>
	struct Object : virtual Mona::Object {
		operator const Type&() const { return (const Type&)self; }
	protected:
		Object() { return; for (const auto& it : (const Type&)self) String(it.first, it.second); }; // trick to detect string iterability on build
	};

	template <typename ...Args>
	String(Args&&... args) {
		Assign<std::string>(*this, std::forward<Args>(args)...);
	}
	std::string& clear() { std::string::clear(); return *this; }

	static const std::string& Empty() { static std::string Empty; return Empty; }


	struct Comparator {
		bool operator()(const std::string& value1, const std::string& value2) const { return value1.compare(value2)<0; }
		bool operator()(const char* value1, const char* value2) const { return strcmp(value1, value2)<0; }
	};
	struct IComparator {
		bool operator()(const std::string& value1, const std::string& value2) const { return String::ICompare(value1, value2)<0; }
		bool operator()(const char* value1, const char* value2) const { return String::ICompare(value1, value2)<0; }
	};

	/*!
	Null terminate a string in a scoped place
	/!\ Can't work on a literal C++ declaration!
	/!\ When using by "data+size" way, address must be in data capacity! ( */
	struct Scoped {
		NULLABLE(_value == NULL);
		template<typename BufferType>
		Scoped(const char* value, uint32_t size, const BufferType& buffer) { init(value, size, buffer.data(), buffer.size()); }
		Scoped(const char* value, uint32_t size, const char* buffer, uint32_t capacity) { init(value, size, buffer, capacity); }
		Scoped(const Packet& packet) { init(EXP(packet), packet.data(), packet.buffer() ? packet.buffer()->size() : packet.size()); }
		~Scoped() { release(); }
		operator const char*() const { return _value; }
		const char* c_str() const { return _value; }
		const char* release();
	private:
		void init(const char* value, uint32_t size, const char* buffer, uint32_t capacity);
		char*			_value;
		std::size_t		_size;
		char			_c;
	};

	/*!
	Encode value to UTF8 when required, if the value was already UTF8 compatible returns true, else false */
	static bool ToUTF8(char value, char (&buffer)[2]);
	/*!
	Encode value to UTF8, onEncoded returns concatenate piece of string encoded (to allow no data copy) */
	typedef std::function<void(const char* value, std::size_t size)> OnEncoded;
	static void ToUTF8(const char* value, const String::OnEncoded& onEncoded) { ToUTF8(value, std::string::npos, onEncoded); }
	static void ToUTF8(const std::string& value, const String::OnEncoded& onEncoded) { ToUTF8(value.data(), value.size(), onEncoded); }
	static void ToUTF8(const char* value, std::size_t size, const String::OnEncoded& onEncoded);
	
	typedef std::function<bool(uint32_t index, const char* value)> ForEach; /// String::Split function type handler
	static std::size_t Split(const std::string& value, const char* separators, const String::ForEach& forEach, SPLIT_OPTIONS options = 0) {
		return Split(value.c_str(), std::string::npos, separators, forEach, options); // string::npos to avoid that Scoped instanciate a new string
	}
	template<typename ListType, typename = typename std::enable_if<is_container<ListType>::value, ListType>::type>
	static ListType& Split(const std::string& value, const char* separators, ListType& values, SPLIT_OPTIONS options = 0) {
		return Split(value.c_str(), std::string::npos, separators, values, options); // string::npos to avoid that Scoped instanciate a new string
	}
	/*!
	/!\ Can't work on a literal C++ declaration! */
	static std::size_t Split(const char* value, const char* separators, const String::ForEach& forEach, SPLIT_OPTIONS options = 0) { return Split(value, std::string::npos, separators, forEach, options); }
	/*!
	/!\ Can't work on a literal C++ declaration! */
	static std::size_t Split(const char* value, std::size_t size, const char* separators, const String::ForEach& forEach, SPLIT_OPTIONS options = 0);
	/*!
	/!\ Can't work on a literal C++ declaration! */
	template<typename ListType, typename = typename std::enable_if<is_container<ListType>::value, ListType>::type>
	static ListType& Split(const char* value, const char* separators, ListType& values, SPLIT_OPTIONS options = 0) { return Split(value, std::string::npos, separators, values, options); }
	/*!
	/!\ Can't work on a literal C++ declaration! */
	template<typename ListType, typename = typename std::enable_if<is_container<ListType>::value, ListType>::type>
	static ListType& Split(const char* value, std::size_t size, const char* separators, ListType& values, SPLIT_OPTIONS options = 0) {
		ForEach forEach([&values](uint32_t index, const char* value) {
			values.insert(values.end(), value);
			return true;
		});
		Split(value, size, separators, forEach, options);
		return values;
	}

	static size_t		TrimLeft(const char*& value, std::size_t size = std::string::npos);
	static std::string&	TrimLeft(std::string& value) { const char* data(value.data()); return value.erase(0, value.size()-TrimLeft(data, value.size())); }

	static size_t		TrimRight(const char* value, std::size_t size = std::string::npos);
	static std::string&	TrimRight(std::string& value) { value.resize(TrimRight(value.data(), value.size())); return value; }

	static size_t		Trim(const char*& value, std::size_t size = std::string::npos) { size = TrimLeft(value, size); return TrimRight(value, size); }
	static std::string&	Trim(std::string& value) { return TrimRight(TrimLeft(value)); }
	
	static std::string  toLower(std::string value) { for (char& c : value) c = tolower(c); return value; }
	static std::string	toUpper(std::string value) { for (char& c : value) c = toupper(c); return value; }

	static int ICompare(const char* data, const char* value, std::size_t count = std::string::npos) { return ICompare(data, std::string::npos, value, count); }
	static int ICompare(const char* data, std::size_t size, const char* value, std::size_t count = std::string::npos);
	static int ICompare(const std::string& data, const char* value, std::size_t count = std::string::npos) { return ICompare(data.c_str(), data.size(), value, count); }
	static int ICompare(const std::string& data, const std::string& value, std::size_t count = std::string::npos) { return ICompare(data.c_str(), data.size(), value.c_str(), count); }

	static bool IEqual(const char* data, const char* value, std::size_t count = std::string::npos) { return ICompare(data, std::string::npos, value, count) == 0; }
	static bool IEqual(const char* data, std::size_t size, const char* value, std::size_t count = std::string::npos) { return ICompare(data, size, value, count) == 0; }
	static bool IEqual(const std::string& data, const char* value, std::size_t count = std::string::npos) { return ICompare(data.c_str(), data.size(), value, count) == 0; }
	static bool IEqual(const std::string& data, const std::string& value, std::size_t count = std::string::npos) { return ICompare(data.c_str(), data.size(), value.c_str(), count) == 0; }

	static bool Equal(const char* data, const char* value, std::size_t count = std::string::npos) { return ICompare(data, std::string::npos, value, count) == 0; }
	static bool Equal(const char* data, std::size_t size, const char* value, std::size_t count = std::string::npos) { return ICompare(data, size, value, count) == 0; }
	static bool Equal(const std::string& data, const char* value, std::size_t count = std::string::npos) { return (count == std::string::npos ? data.compare(value) : data.compare(0, count, value, 0, count)) == 0; }
	static bool Equal(const std::string& data, const std::string& value, std::size_t count = std::string::npos) { return data.compare(0, count, value, 0, count) == 0; }

	template<typename Type, uint8_t base=10>
	static bool tryNumber(const std::string& value, Type& result) { return tryNumber<Type, base>(value.data(), value.size(), result); }
	template<typename Type, uint8_t base=10>
	static bool tryNumber(const char* value, Type& result) { return tryNumber<Type, base>(value, std::string::npos, result); }
	template<typename Type, uint8_t base=10>
	static bool tryNumber(const char* value, std::size_t size, Type& result);
	template<typename Type, uint8_t base=10>
	static Type toNumber(const std::string& value, Type defaultValue = 0) { Type result; return tryNumber<Type, base>(value.data(), value.size(), result) ? result : defaultValue; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(const char* value, size_t size = std::string::npos) { Type result; return tryNumber<Type, base>(value, size, result) ? result : 0; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(const char* value, std::size_t size, Type defaultValue) { Type result; return tryNumber<Type, base>(value, size, result) ? result : defaultValue; }

	template<typename Type, uint8_t base=10>
	static bool tryNumber(Exception& ex, const std::string& value, Type& result) { return tryNumber<Type, base>(ex, value.data(), value.size(), result); }
	template<typename Type, uint8_t base=10>
	static bool tryNumber(Exception& ex, const char* value, Type& result) { return tryNumber<Type, base>(ex, value, std::string::npos, result); }
	template<typename Type, uint8_t base=10>
	static bool tryNumber(Exception& ex, const char* value, std::size_t size, Type& result);
	template<typename Type, uint8_t base=10>
	static Type toNumber(Exception& ex, const std::string& value, Type defaultValue = 0) { Type result; return tryNumber<Type, base>(ex, value.data(), value.size(), result) ? result : defaultValue; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(Exception& ex, const char* value, size_t size = std::string::npos) { Type result; return tryNumber<Type, base>(ex, value, size, result) ? result : 0; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(Exception& ex, const char* value, std::size_t size, Type defaultValue) { Type result; return tryNumber<Type, base>(ex, value, size, result) ? result : defaultValue; }


	static bool IsTrue(const std::string& value) { return IsTrue(value.data(),value.size()); }
	static bool IsTrue(const char* value, std::size_t size=std::string::npos) { return IEqual(value, size, "1") || IEqual(value, size, "true") || IEqual(value, size, "yes") || IEqual(value, size, "on"); }
	static bool IsFalse(const std::string& value) { return IsFalse(value.data(),value.size()); }
	static bool IsFalse(const char* value, std::size_t size = std::string::npos) { return IEqual(value, size, "0") || IEqual(value, size, "false") || IEqual(value, size, "no") || IEqual(value, size, "off") || IEqual(value, size, "null"); }

	template <typename BufferType>
	static bool FromURI(char hi, char lo, BufferType& buffer) {
		if (!isxdigit(hi) || !isxdigit(lo)) {
			String::Append(buffer, '%', hi, lo);
			return false;
		}
		hi = ((hi - (hi <= '9' ? '0' : '7')) << 4) | ((lo - (lo <= '9' ? '0' : '7')) & 0x0F);
		buffer.append(&hi, 1);
		return true;
	}
	template <typename BufferType>
	static bool FromURI(const std::string& value, BufferType& buffer) { return FromURI(value.data(), value.size(), buffer); }
	template <typename BufferType>
	static bool FromURI(const char* data, BufferType& buffer) { return FromURI(data, std::string::npos, buffer); }
	template <typename BufferType>
	static bool FromURI(const char* data, std::size_t size, BufferType& buffer) {
		bool oneDone = false;
		int8_t hi = 0;
		while (STR_AVAILABLE(data, size)) {
			if (*data == '%') {
				if (hi) {
					buffer.append(EXPC("%"));
					if (hi != -1)
						buffer.append(&hi, 1);
				}
				hi = -1;
			} else if (hi == -1 && *data != -1)
				hi = *data;
			else if (hi && FromURI(hi, *data, buffer))
				oneDone = true;
			STR_NEXT(data, size);
		};
		return oneDone;
	}


	template <typename BufferType>
	static BufferType& ToHex(BufferType& buffer, bool append = false) { return ToHex(buffer.data(), buffer.size(), buffer, append); }
	template <typename BufferType>
	static BufferType& ToHex(const std::string& value, BufferType& buffer, bool append = false) { return ToHex(value.data(), value.size(), buffer, append); }
	template <typename BufferType>
	static BufferType& ToHex(const char* value, std::size_t size, BufferType& buffer, bool append = false) {
		char* out;
		uint32_t count = size / 2;
		if (size & 1)
			++count;
		if (append) {
			buffer.resize(buffer.size() + count);
			out = (char*)buffer.data() + buffer.size() - count;
		} else {
			if (count>buffer.size())
				buffer.resize(count);
			out = (char*)buffer.data();
		}
		while (size-->0) {
			char left = toupper(*value++);
			char right = size-- ? toupper(*value++) : '0';
			*out++ = ((left - (left <= '9' ? '0' : '7')) << 4) | ((right - (right <= '9' ? '0' : '7')) & 0x0F);
		}
		if(!append)
			buffer.resize(count);
		return buffer;
	}

	template <typename OutType, typename ...Args>
	static OutType& Assign(OutType& out, Args&&... args) {
		out.clear();
		return Append<OutType>(out, std::forward<Args>(args)...);
	}

	/// \brief match "std::string" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const std::string& value, Args&&... args) {
		return Append<OutType>((OutType&)out.append(value.data(), value.size()), std::forward<Args>(args) ...);
	}

	/*!
	const char* */
	template <typename OutType, typename STRType, typename ...Args>
	static typename std::enable_if<std::is_convertible<STRType, const char*>::value, OutType>::type&
	Append(OutType& out, STRType value, Args&&... args) {
		return Append<OutType>((OutType&)out.append(value, strlen(value)), std::forward<Args>(args)...);
	}
	/*!
	String litteral (very fast, without strlen call) */
	template <typename OutType, typename CharType, uint32_t size, typename ...Args>
	static typename std::enable_if<std::is_same<CharType, const char>::value, OutType>::type&
	Append(OutType& out, CharType(&value)[size], Args&&... args) {
		return Append<OutType>((OutType&)out.append(value, size -1), std::forward<Args>(args)...);
	}
	template <typename OutType, typename ...Args>
	static typename std::enable_if<std::is_convertible<OutType, std::string>::value, OutType>::type&
	Append(OutType& out, std::string&& value, Args&&... args) {
		if (!out.size())
			out = std::move(value);
		else
			out.append(value.data(), value.size());
		return Append<OutType>(out, std::forward<Args>(args)...);
	}

	struct Lower : Packet { template<typename ...Args> explicit Lower(Args&&... args) : Packet(std::forward<Args>(args)...) {} };
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Lower& value, Args&&... args) {
		for (std::size_t i = 0; i < value.size(); ++i) {
			char c = tolower(value[i]);
			out.append(&c, 1);
		}
		return Append<OutType>(out, std::forward<Args>(args)...);
	}
	struct Upper : Packet { template<typename ...Args> explicit Upper(Args&&... args) : Packet(std::forward<Args>(args)...) {} };
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Upper& value, Args&&... args) {
		for (std::size_t i = 0; i < value.size(); ++i) {
			char c = toupper(value[i]);
			out.append(&c, 1);
		}
		return Append<OutType>(out, std::forward<Args>(args)...);
	}


#if defined(_WIN32)
	/// \brief match "wstring" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const std::wstring& value, Args&&... args) {
		return Append<OutType>(value.c_str(), std::forward<Args>(args)...);
	}
	
	/// \brief match "const wchar_t*" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const wchar_t* value, Args&&... args) {
		char buffer[PATH_MAX];
		ToUTF8(value, buffer);
		return Append<OutType>((OutType&)out.append(buffer, strlen(buffer)), std::forward<Args>(args)...);
	}
#endif

	// match le "char" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, char value, Args&&... args) {
		return Append<OutType>((OutType&)out.append(&value,1), std::forward<Args>(args)...);
	}

	// match le "signed char" cas
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, signed char value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hhd", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "short" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, short value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hd", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "int" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, int value, Args&&... args) {
		char buffer[16];
		sprintf(buffer, "%d", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "long" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%ld", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "unsigned char" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, unsigned char value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hhu", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "unsigned short" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, unsigned short value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hu", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "unsigned int" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, unsigned int value, Args&&... args) {
		char buffer[16];
		sprintf(buffer, "%u", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "unsigned long" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, unsigned long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%lu", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "int64_t" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, long long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%lld", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "uint64_t" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, unsigned long long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%llu", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "float" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, float value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%.8g", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "double" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, double value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%.16g", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	/// \brief match "bool" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, bool value, Args&&... args) {
		if (value)
			return Append((OutType&)out.append(EXPC("true")), std::forward<Args>(args)...);
		return Append<OutType>((OutType&)out.append(EXPC("false")), std::forward<Args>(args)...);
	}

	/// \brief match "null" case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, std::nullptr_t, Args&&... args) {
		return Append<OutType>((OutType&)out.append(EXPC("null")), std::forward<Args>(args)...);
	}

	/// \brief match pointer case
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const void* value, Args&&... args)	{
		char buffer[32];
		sprintf(buffer,"%p", value);
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	template<typename OutType>
	struct Writer : virtual Mona::Object {
		virtual bool write(OutType& out) = 0;
	};
	template <typename OutType, typename Type, typename ...Args>
	static OutType& Append(OutType& out, Writer<Type>& writer, Args&&... args) {
		while (writer.write(out));
		return Append<OutType>(out, std::forward<Args>(args)...);
	}
	

	/// \brief A usefull form which use snprintf to format out
	///
	/// \param out This is the std::string which to append text
	/// \param value A pair of format text associate with value (ex: pair<char*, double>("%.2f", 10))
	/// \param args Other arguments to append

	template<typename ValueType>
	struct Format : virtual Mona::Object {
		Format(const char* format, const ValueType& value) : value(value), format(format) {}
		const ValueType&	value;
		const char*			format;
	};
	template <typename OutType, typename Type, typename ...Args>
	static OutType& Append(OutType& out, const Format<Type>& custom, Args&&... args) {
		char buffer[64];
		try {
            snprintf(buffer, sizeof(buffer), custom.format, custom.value);
		}
		catch (...) {
			return Append<OutType>(out, std::forward<Args>(args)...);
		}
		return Append<OutType>((OutType&)out.append(buffer,strlen(buffer)), std::forward<Args>(args)...);
	}

	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Bytes& bytes, Args&&... args) {
		return Append<OutType>((OutType&)out.append(EXP(bytes)), std::forward<Args>(args)...);
	}

	struct Repeat : virtual Mona::Object {
		Repeat(uint32_t count, char value) : value(value), count(count) {}
		const char value;
		const uint32_t count;
	};
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Repeat& repeat, Args&&... args) {
		return Append<OutType>((OutType&)out.append(repeat.count, repeat.value), std::forward<Args>(args)...);
	}

	struct Date : virtual Mona::Object {
		Date(const Mona::Date& date, const char* format= Mona::Date::FORMAT_ISO8601) : format(format), _pDate(&date) {}
		Date(const char* format) : format(format), _pDate(NULL) {}
		const Mona::Date*	operator->() const { return _pDate; }
		const char*	const	format ;
	private:
		const Mona::Date* _pDate;
	};
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Date& date, Args&&... args) {
		const Mona::Date* pDate = date.operator->();
		if(pDate)
			return Append<OutType>((OutType&)pDate->format(date.format, out), std::forward<Args>(args)...);
		return Append<OutType>((OutType&)Mona::Date().format(date.format, out), std::forward<Args>(args)...);
	}
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Mona::Date& date, Args&&... args) {
		return Append<OutType>(out, Date(date), std::forward<Args>(args)...);
	}

	struct URI : virtual Mona::Object {
		URI(const char* value, std::size_t size = std::string::npos) : value(value), size(size) {}
		URI(const std::string& value) : value(value.data()), size(value.size()) {}
		const char* const value;
		const std::size_t size;
	};
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const URI& uri, Args&&... args) {
		std::size_t size = uri.size;
		const char* value = uri.value;
		while (STR_AVAILABLE(value, size)) {
			char c = *value;
			// https://en.wikipedia.org/wiki/Percent-encoding#Types_of_URI_characters
			if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
				out.append(&c, 1);
			else
				String::Append(out, '%', String::Format<uint8_t>("%2X", c)); // uint8_t to get a right value!
			STR_NEXT(value, size);
		}
		return Append<OutType>(out, std::forward<Args>(args)...);
	}

	struct Hex : virtual Mona::Object {
		Hex(const char* data, uint32_t size, HEX_OPTIONS options = 0) : data(data), size(size), options(options) {}
		const char*			data;
		const uint32_t		size;
		const HEX_OPTIONS	options;
	};
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Hex& hex, Args&&... args) {
		const char *end = hex.data + hex.size, *data = hex.data;
		bool skipLeft(false);
		if (hex.options&HEX_TRIM_LEFT) {
			while (data<end) {
				if ((U(*data) >> 4)>0)
					break;
				if (((*data) & 0x0F) > 0) {
					skipLeft = true;
					break;
				}
				++data;
			}
		}
		char ref = hex.options&HEX_UPPER_CASE ? '7' : 'W';
		char value;
		while (data<end) {
			if (hex.options&HEX_CPP)
				out.append(EXPC("\\x"));
			value = U(*data) >> 4;
			if (!skipLeft) {
				value += value > 9 ? ref : '0';
				out.append(&value, 1);
			} else
				skipLeft = false;
			value = (*data++) & 0x0F;
			value += value > 9 ? ref : '0';
			out.append(&value, 1);
		}
		return Append<OutType>(out, std::forward<Args>(args)...);
	}
	template <typename OutType, typename Type, typename ...Args>
	static OutType& Append(OutType& out, const Object<Type>& object, Args&&... args) {
		bool first = true;
		out.append(EXPC("{"));
		for (const auto& it : (const Type&)object) {
			if (!first)
				out.append(EXPC(", "));
			else
				first = false;
			Append<OutType>(out, it.first, ": ", it.second);
		}
		out.append(EXPC("}"));
		return Append<OutType>(out, std::forward<Args>(args)...);
	}
	struct Log : virtual Mona::Object {
		Log(const char* level, const std::string& file, long line, const std::string& message, uint32_t threadId = 0) : threadId(threadId), level(level), file(file), line(line), message(message) {}
		const char*			level;
		const std::string&	file;
		const long			line;
		const std::string&	message;
		const uint32_t		threadId;
	};
	template <typename OutType, typename ...Args>
	static OutType& Append(OutType& out, const Log& log, Args&&... args) {
		uint32_t size = Mona::Date().format("%d/%m %H:%M:%S.%c  ", out).size();
		out.append(7 - (Append<OutType>(out,log.level).size() - size), ' ');
		if (log.threadId) {
			Append<OutType>(out, log.threadId);
			size += 5; // tab to 60 (data including), otherwise 55!
		}
		size = Append<OutType>(out, ' ', ShortPath(log.file), '[', log.line, "] ").size() - size;
		if (size < 37)
			out.append(37 - size, ' ');
		return Append<OutType>(out, log.message, '\n', std::forward<Args>(args)...);
	}


	template <typename OutType>
	static OutType& Append(OutType& out) { return out; }

private:
	static const char* ShortPath(const std::string& path);
#if defined(_WIN32)
	static const char* ToUTF8(const wchar_t* value, char buffer[PATH_MAX]);
#endif
};

inline bool operator==(const std::string& left, const char* right) { return left.compare(right) == 0; }
inline bool operator==(const std::string& left, const std::string& right) { return left.compare(right) == 0; }
inline bool operator==(const char* left, const std::string& right) { return right.compare(left) == 0; }
inline bool operator!=(const std::string& left, const char* right) { return left.compare(right) != 0; }
inline bool operator!=(const std::string& left, const std::string& right) { return left.compare(right) != 0; }
inline bool operator!=(const char* left, const std::string& right) { return right.compare(left) != 0; }
inline bool operator<(const std::string& left, const char* right) { return left.compare(right) < 0; }
inline bool operator<(const std::string& left, const std::string& right) { return left.compare(right) < 0; }
inline bool operator<(const char* left, const std::string& right) { return right.compare(left) > 0; }
inline bool operator<=(const std::string& left, const char* right) { return left.compare(right) <= 0; }
inline bool operator<=(const std::string& left, const std::string& right) { return left.compare(right) <= 0; }
inline bool operator<=(const char* left, const std::string& right) { return right.compare(left) >= 0; }
inline bool operator>(const std::string& left, const char* right) { return left.compare(right) > 0; }
inline bool operator>(const std::string& left, const std::string& right) { return left.compare(right) > 0; }
inline bool operator>(const char* left, const std::string& right) { return right.compare(left) < 0; }
inline bool operator>=(const std::string& left, const char* right) { return left.compare(right) >= 0; }
inline bool operator>=(const std::string& left, const std::string& right) { return left.compare(right) >= 0; }
inline bool operator>=(const char* left, const std::string& right) { return right.compare(left) <= 0; }


} // namespace Mona

