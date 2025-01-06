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
#include "Mona/Format/Format.h"
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


struct LowerCase : Format<LowerCase> {
	template <typename ...Args>
	LowerCase(Args&&... args) : _packet(std::forward<Args>(args)...) {}
	template<typename OutType>
	void stringify(OutType& out) const {
		for (std::size_t i = 0; i < _packet.size(); ++i) {
			char c = tolower(_packet[i]);
			out.append(&c, 1);
		}
	}
private:
	Packet _packet;
};

struct UpperCase : Format<UpperCase> {
	template <typename ...Args>
	UpperCase(Args&&... args) : _packet(std::forward<Args>(args)...) {}
	template<typename OutType>
	void stringify(OutType& out) const {
		for (std::size_t i = 0; i < _packet.size(); ++i) {
			char c = toupper(_packet[i]);
			out.append(&c, 1);
		}
	}
private:
	Packet _packet;
};


/// Utility class for generation parse of strings
struct String : std::string {
	NULLABLE(empty())

	/**
	Object formatable, must be iterable with key/value convertible in string */
	template<typename Type>
	struct Object : virtual Mona::Object {
		operator const Type&() const { return (const Type&)self; }
	protected:
		Object() { return; for (const auto& it : (const Type&)self) String(it.first, it.second); }; // trick to detect string iterability on build
	};

	template <typename ...Args>
	String(Args&&... args) {
		String::assign<std::string>(self, std::forward<Args>(args)...);
	}
	using std::string::append;
	using std::string::assign;
	std::string& clear() { std::string::clear(); return self; }

	static const std::string& Empty() { static std::string Empty; return Empty; }


	struct Comparator {
		bool operator()(const std::string& value1, const std::string& value2) const { return value1.compare(value2)<0; }
		bool operator()(const char* value1, const char* value2) const { return strcmp(value1, value2)<0; }
	};
	struct IComparator {
		bool operator()(const std::string& value1, const std::string& value2) const { return String::ICompare(value1, value2)<0; }
		bool operator()(const char* value1, const char* value2) const { return String::ICompare(value1, value2)<0; }
	};

	/**
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

	/**
	Encode value to UTF8 when required, if the value was already UTF8 compatible returns true, else false */
	static bool ToUTF8(char value, char (&buffer)[2]);
	/**
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
	/**
	/!\ Can't work on a literal C++ declaration! */
	static std::size_t Split(const char* value, const char* separators, const String::ForEach& forEach, SPLIT_OPTIONS options = 0) { return Split(value, std::string::npos, separators, forEach, options); }
	/**
	/!\ Can't work on a literal C++ declaration! */
	static std::size_t Split(const char* value, std::size_t size, const char* separators, const String::ForEach& forEach, SPLIT_OPTIONS options = 0);
	/**
	/!\ Can't work on a literal C++ declaration! */
	template<typename ListType, typename = typename std::enable_if<is_container<ListType>::value, ListType>::type>
	static ListType& Split(const char* value, const char* separators, ListType& values, SPLIT_OPTIONS options = 0) { return Split(value, std::string::npos, separators, values, options); }
	/**
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
	static bool tryNumber(Type& result, const std::string& value) { return tryNumber<Type, base>(result, value.data(), value.size()); }
	template<typename Type, uint8_t base=10>
	static bool tryNumber(Type& result, const char* value, std::size_t size = std::string::npos);
	template<typename Type, uint8_t base=10>
	static Type toNumber(const std::string& value, Type defaultValue = 0) { Type result; return tryNumber<Type, base>(result, value.data(), value.size()) ? result : defaultValue; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(const char* value, size_t size = std::string::npos) { Type result; return tryNumber<Type, base>(result, value, size) ? result : 0; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(const char* value, std::size_t size, Type defaultValue) { Type result; return tryNumber<Type, base>(result, value, size) ? result : defaultValue; }

	template<typename Type, uint8_t base=10>
	static bool tryNumber(Exception& ex, Type& result, const std::string& value) { return tryNumber<Type, base>(ex, result, value.data(), value.size()); }
	template<typename Type, uint8_t base=10>
	static bool tryNumber(Exception& ex, Type& result, const char* value, std::size_t size = std::string::npos);
	template<typename Type, uint8_t base=10>
	static Type toNumber(Exception& ex, const std::string& value, Type defaultValue = 0) { Type result; return tryNumber<Type, base>(ex, result, value.data(), value.size()) ? result : defaultValue; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(Exception& ex, const char* value, size_t size = std::string::npos) { Type result; return tryNumber<Type, base>(ex, result, value, size) ? result : 0; }
	template<typename Type, uint8_t base=10>
	static Type toNumber(Exception& ex, const char* value, std::size_t size, Type defaultValue) { Type result; return tryNumber<Type, base>(ex, result, value, size) ? result : defaultValue; }


	static bool IsTrue(const std::string& value) { return IsTrue(value.data(),value.size()); }
	static bool IsTrue(const char* value, std::size_t size=std::string::npos) { return IEqual(value, size, "1") || IEqual(value, size, "true") || IEqual(value, size, "yes") || IEqual(value, size, "on"); }
	static bool IsFalse(const std::string& value) { return IsFalse(value.data(),value.size()); }
	static bool IsFalse(const char* value, std::size_t size = std::string::npos) { return IEqual(value, size, "0") || IEqual(value, size, "false") || IEqual(value, size, "no") || IEqual(value, size, "off") || IEqual(value, size, "null"); }

	static std::string& replace(std::string& str, const std::string& what, const std::string& with);

	template <typename OutType, typename ...Args>
	static OutType& assign(OutType& out, Args&&... args) {
		out.clear();
		return append<OutType>(out, std::forward<Args>(args)...);
	}

	/// \brief match "std::string" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, const std::string& value, Args&&... args) {
        out.append(value.data(), value.size());
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/**
	const char* */
	template <typename OutType, typename STRType, typename ...Args>
	static typename std::enable_if<std::is_convertible<STRType, const char*>::value, OutType>::type&
	append(OutType& out, STRType value, Args&&... args) {
        out.append(STR value, strlen(value));
        return append<OutType>(out, std::forward<Args>(args)...);
    }
	/**
	String litteral (very fast, without strlen call) */
	template <typename OutType, std::size_t size, typename ...Args>
	static OutType& append(OutType& out, const char(&value)[size], Args&&... args) {
        out.append(STR value, size - 1);
        return append<OutType>(out, std::forward<Args>(args)...);
    }
	template <typename OutType, typename ...Args>
	static typename std::enable_if<std::is_convertible<OutType, std::string>::value, OutType>::type&
	append(OutType& out, std::string&& value, Args&&... args) {
		if (!out.size())
			out = std::move(value);
		else
			out.append(value.data(), value.size());
		return append<OutType>(out, std::forward<Args>(args)...);
	}


#if defined(_WIN32)
	/// \brief match "wstring" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, const std::wstring& value, Args&&... args) {
		return append<OutType>(value.c_str(), std::forward<Args>(args)...);
	}
	
	/// \brief match "const wchar_t*" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, const wchar_t* value, Args&&... args) {
		char buffer[PATH_MAX];
		ToUTF8(value, buffer);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }
#endif

	// match le "char" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, char value, Args&&... args) {
        out.append(&value, 1);
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	// match le "signed char" cas
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, signed char value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hhd", value);
		out.append(STR buffer,strlen(buffer));
		return append<OutType>(out, std::forward<Args>(args)...);
	}

	/// \brief match "short" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, short value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hd", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "int" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, int value, Args&&... args) {
		char buffer[16];
		sprintf(buffer, "%d", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "long" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%ld", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "unsigned char" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, unsigned char value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hhu", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "unsigned short" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, unsigned short value, Args&&... args) {
		char buffer[8];
		sprintf(buffer, "%hu", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "unsigned int" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, unsigned int value, Args&&... args) {
		char buffer[16];
		sprintf(buffer, "%u", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "unsigned long" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, unsigned long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%lu", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "int64_t" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, long long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%lld", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "uint64_t" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, unsigned long long value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%llu", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "float" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, float value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%.8g", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "double" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, double value, Args&&... args) {
		char buffer[32];
		sprintf(buffer, "%.16g", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match "bool" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, bool value, Args&&... args) {
		if (value) {
            out.append(EXPC("true"));
        } else {
            out.append(EXPC("false"));
        }
        return append<OutType>(out, std::forward<Args>(args)...);
	}

	/// \brief match "null" case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, std::nullptr_t, Args&&... args) {
        out.append(EXPC("null"));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	/// \brief match pointer case
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, const void* value, Args&&... args)	{
		char buffer[32];
		sprintf(buffer,"%p", value);
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	template<typename OutType>
	struct Writer : virtual Mona::Object {
		virtual bool write(OutType& out) = 0;
	};
	template <typename OutType, typename Type, typename ...Args>
	static OutType& append(OutType& out, Writer<Type>& writer, Args&&... args) {
		while (writer.write(out));
		return append<OutType>(out, std::forward<Args>(args)...);
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
	static OutType& append(OutType& out, const Format<Type>& custom, Args&&... args) {
		char buffer[64];
		try {
            snprintf(buffer, sizeof(buffer), custom.format, custom.value);
		}
		catch (...) {
			return append<OutType>(out, std::forward<Args>(args)...);
		}
        out.append(STR buffer, strlen(buffer));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	template <typename OutType, typename Type, typename ...Args>
	static OutType& append(OutType& out, const Mona::Format<Type>& format, Args&&... args) {
        format.to(out);
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, const Bytes& bytes, Args&&... args) {
        out.append(EXP(bytes));
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	struct Repeat : virtual Mona::Object {
		Repeat(uint32_t count, char value) : value(value), count(count) {}
		const char value;
		const uint32_t count;
	};
	template <typename OutType, typename ...Args>
	static OutType& append(OutType& out, const Repeat& repeat, Args&&... args) {
        out.append(repeat.count, repeat.value);
        return append<OutType>(out, std::forward<Args>(args)...);
    }

	
	template <typename OutType, typename Type, typename ...Args>
	static OutType& append(OutType& out, const Object<Type>& object, Args&&... args) {
		bool first = true;
		out.append(EXPC("{"));
		for (const auto& it : (const Type&)object) {
			if (!first)
				out.append(EXPC(", "));
			else
				first = false;
			append<OutType>(out, it.first, ": ", it.second);
		}
		out.append(EXPC("}"));
		return append<OutType>(out, std::forward<Args>(args)...);
	}

	template <typename OutType>
	static OutType& append(OutType& out) { return out; }

private:
#if defined(_WIN32)
	static const char* ToUTF8(const wchar_t* value, char buffer[PATH_MAX]);
#endif
};


} // namespace Mona

