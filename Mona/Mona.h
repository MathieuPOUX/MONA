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


#include <stdio.h>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <cstring>
#include <limits>
#include <memory>
#include <functional>
#include <cmath> // C++ version of math.h to solve abs ambiguity over linux
#include "assert.h"


/////  Usefull macros and patchs   //////

#define BIN (uint8_t*)
#define STR (char*)

#define self    (*this)
#define NULLABLE(CONDITION) explicit operator bool() const { return CONDITION ? false : true; }

// usefull to parse string with 0 null char OR relating a size given
#define	STR_AVAILABLE(DATA, SIZE)	((signed(SIZE) < 0 ? *(DATA) : SIZE) ? true : false)
#define STR_NEXT(DATA, SIZE)		(--SIZE, ++DATA)

#define SET		std::piecewise_construct
#define SET_T	std::piecewise_construct_t
#define EXPC(VALUE)	VALUE"",(sizeof(VALUE)-1) // "" concatenation is here to check that it's a valid const string is not a pointer of char*
#define EXP(VALUE)	(VALUE).data(), (VALUE).size()

#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x
#define LINE_STRING STRINGIFY(__LINE__)


#if defined(_WIN32)
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#define NOMINMAX
#include "windows.h"
#define sprintf sprintf_s
#define snprintf sprintf_s
#define PATH_MAX 4096 // to match Linux!
#define __BIG_ENDIAN__ 0 // windows is always little endian!
#elif defined(__FreeBSD__) || defined(__APPLE__) || defined(__TOS_MACOS__) || defined(__NetBSD__) || defined(__OpenBSD__)
#define _BSD 1 // Detect BSD system
#endif


#if defined(_WIN64)
#define OpenSSL(FILE) <openssl64/FILE>
#else
#define OpenSSL(FILE) <openssl/FILE>
#endif

//
// Automatically link Base library.
//
#if defined(_MSC_VER)
// disable the "throw noexception" warning because Mona has its own exception and can use everywhere std throw on FATAL problem (unexpected behavior)
#pragma warning(disable: 4297)

#pragma comment(lib, "crypt32.lib")
#if defined(_DEBUG)
#pragma comment(lib, "libcryptod.lib")
#pragma comment(lib, "libssld.lib")
#else
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#endif

#endif


namespace Mona {

/*!
Use a U(value) conversion to make bitshit and bitright operator safe with signed number */
template<typename Type>
static typename std::make_unsigned<Type>::type U(Type value) { return value; }

enum Any : uint8_t {
	ANY_NONE = 0, // keep equal to 0!
	ANY_NULL,
	ANY_BOOLEAN,
	ANY_NUMBER,
	ANY_DATE,
	ANY_STRING, // < ANY_STRING => number!
	ANY_BINARY,
	ANY_OBJECT, // first complex type! (>= ANY_OBJECT => complexType)
	ANY_ARRAY,
	ANY_MAP
};


#if defined(_DEBUG) && defined(_WIN32)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
void DetectMemoryLeak();
#else
void DetectMemoryLeak();
#endif


#define		STATIC_ASSERT(...)				{ static_assert(__VA_ARGS__, #__VA_ARGS__); }

#if defined(_DEBUG)
#if defined(_WIN32)
#define		DEBUG_ASSERT(ASSERT)			{ _ASSERTE(ASSERT); }
#else
#define		DEBUG_ASSERT(ASSERT)			{ assert(ASSERT); }
#endif
#define 	CHECK(ASSERT)		DEBUG_ASSERT(ASSERT)
#else
#define		DEBUG_ASSERT(ASSERT)		{}
#define 	CHECK(ASSERT)		{ if(!(ASSERT)) throw std::runtime_error( #ASSERT " assertion, " __FILE__ "[" LINE_STRING "]"); }
#endif

///// TYPES /////
/*!
unique_ptr on the mode of shared and which forbid custom deleter (too complicated, use a event on object deletion rather) */
template<typename Type>
struct Unique : std::unique_ptr<Type> {
	Unique() : std::unique_ptr<Type>() {}
	template<typename ArgType, typename = typename std::enable_if<std::is_constructible<std::unique_ptr<Type>, ArgType>::value>::type>
	Unique(ArgType&& arg) : std::unique_ptr<Type>(std::forward<ArgType>(arg)) {}
	template<typename ...Args>
	Unique(SET_T, Args&&... args) : std::unique_ptr<Type>(new Type(std::forward<Args>(args)...)) {}

	template<typename NewType = Type, typename ...Args>
	NewType& set(Args&&... args) { std::unique_ptr<Type>::reset(new NewType(std::forward<Args>(args)...)); return (NewType&)*self; }
	Unique& reset() { std::unique_ptr<Type>::reset(); return self; }
	template<typename ArgType>
	Unique& operator=(ArgType&& arg) { std::unique_ptr<Type>::operator=(std::forward<ArgType>(arg)); return self; };
	template<typename NewType>
	Unique& operator=(NewType* pType) { std::unique_ptr<Type>::reset(pType); return self; };
};
/*!
shared_ptr which forbid pointer/new construction (too slow) and custom deleter (too complicated, use a event on object deletion rather) */
template<typename Type>
struct Shared : std::shared_ptr<Type> {
	Shared() : std::shared_ptr<Type>() {}
	template<typename ArgType, typename = typename std::enable_if<!std::is_pointer<ArgType>::value && std::is_constructible<std::shared_ptr<Type>, ArgType>::value>::type>
	Shared(ArgType&& arg) : std::shared_ptr<Type>(std::forward<ArgType>(arg)) {}
	template<typename ArgType>
	Shared(const ArgType& arg, Type* pObj) : std::shared_ptr<Type>(arg, pObj) {}
	template<typename ...Args>
	Shared(SET_T, Args&&... args) : std::shared_ptr<Type>(std::make_shared<Type>(std::forward<Args>(args)...)) {}

	template<typename NewType = Type, typename ...Args>
	NewType& set(Args&&... args) { return *(NewType*)std::shared_ptr<Type>::operator=(std::make_shared<NewType>(std::forward<Args>(args)...)).get(); }
	Shared& reset() { std::shared_ptr<Type>::reset(); return self; }
	template<typename ArgType>
	Shared& operator=(ArgType&& arg) { std::shared_ptr<Type>::operator=(std::forward<ArgType>(arg)); return self; };
	template<typename NewType>
	Shared& operator=(NewType* pType) { std::shared_ptr<Type>::reset(pType); return self; };
};
template<typename Type>
using Weak = std::weak_ptr<Type>;

template< typename T, typename U >
Shared<T> const_pointer_cast(const Shared<U>& r) noexcept { return Shared<T>(r, const_cast<T*>(r.get())); }
template< typename T, typename U >
Shared<T> static_pointer_cast(const Shared<U>& r) noexcept { return Shared<T>(r, static_cast<T*>(r.get())); }
template< typename T, typename U >
Shared<T> reinterpret_pointer_cast(const Shared<U>& r) noexcept { return Shared<T>(r, reinterpret_cast<T*>(r.get())); }
template< typename T, typename U >
Shared<T> dynamic_pointer_cast(const Shared<U>& r) noexcept {
	if (auto p = dynamic_cast<T*>(r.get()))
		return Shared<T>(r, p);
	return Shared<T>();
}

//////  No copy, no move, objet nullable  //////


struct Static {
private:
	Static() = delete;
	Static(const Static& other) = delete;
	Static& operator=(const Static& other) = delete;
	Static(Static&& other) = delete;
	Static& operator=(Static&& other) = delete;
};
 
struct Object {
	Object() {}
	virtual ~Object() {};
private:
	Object(const Object& other) = delete;
	Object& operator=(const Object& other) = delete;
	Object(Object&& other) = delete;
	Object& operator=(Object&& other) = delete;
};

////// ASCII ////////

struct ASCII : virtual Static {
	enum Type {
		CONTROL  = 0x0001,
		BLANK    = 0x0002,
		SPACE    = 0x0004,
		PUNCT    = 0x0008,
		DIGIT    = 0x0010,
		HEXDIGIT = 0x0020,
		ALPHA    = 0x0040,
		LOWER    = 0x0080,
		UPPER    = 0x0100,
		GRAPH    = 0x0200,
		PRINT    = 0x0400,
		XML		 = 0x0800
	};

	static uint8_t toLower(char value) { return Is(value, UPPER) ? (value + 32) : value; }
	static uint8_t toUpper(char value) { return Is(value, LOWER) ? (value - 32) : value; }

	static bool Is(char value,uint16_t type) {return value&0x80 ? 0 : ((_CharacterTypes[int(value)]&type) != 0);}
private:
	static const uint16_t _CharacterTypes[128];
};

inline constexpr uint32_t FourCC(uint8_t a, uint8_t b, uint8_t c, uint8_t d) { return (((((a << 8) | b) << 8) | c) << 8) | d; }

template <typename Type>
class is_smart_pointer : virtual Static {
	template<typename S> static char(&G(typename std::enable_if<
		std::is_same<decltype(static_cast<typename S::element_type*(S::*)() const>(&S::get)), typename S::element_type*(S::*)() const>::value,
		void
	>::type*))[1];
	template<typename S> static char(&G(...))[2];
public:
	static bool const value = sizeof(G<Type>(0)) == 1;
};

template<typename Type, typename = typename std::enable_if<std::is_pointer<Type>::value>::type>
Type ToPointer(Type pType) { return pType; }
template<typename Type, typename = typename std::enable_if<!std::is_pointer<Type>::value && !is_smart_pointer<Type>::value>::type>
Type* ToPointer(Type& type) { return &type; }
template<typename Type>
Type* ToPointer(const std::shared_ptr<Type>& type) { return type.get(); }
template<typename Type>
Type* ToPointer(const std::unique_ptr<Type>& type) { return type.get(); }
template<typename K, typename V>
auto ToPointer(std::pair<K, V>& type) { return ToPointer(type.second); }

struct PtrComparator {
	template<typename Type>
	bool operator() (Type&& ptr1, Type&& ptr2) const { return ToPointer(ptr1) < ToPointer(ptr2); }
};

template<typename ListType, typename FType, typename ...Args>
inline auto Calls(ListType& objects, FType&& pF, Args&&... args) {
	typedef typename ListType::value_type ObjType;
	auto result = std::result_of<decltype(pF)(ObjType, Args...)>::type();
	for (ObjType& obj : objects)
		result += (ToPointer(obj)->*pF)(std::forward<Args>(args) ...);
	return result;
}

inline uint64_t abs(double value) { return (uint64_t)std::abs(value); }
inline uint64_t abs(float value) { return (uint64_t)std::abs(value); }
inline uint64_t abs(int64_t value) { return (uint64_t)std::abs(value); }
inline uint32_t abs(int32_t value) { return (uint32_t)std::abs(value); }
inline uint16_t abs(int16_t value) { return (uint16_t)std::abs(value); }
inline uint8_t abs(int8_t value) { return (uint8_t)std::abs(value); }

template <typename T>
inline int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

inline bool isalnum(char value) { return ASCII::Is(value, ASCII::ALPHA | ASCII::DIGIT); }
inline bool isalpha(char value) { return ASCII::Is(value,ASCII::ALPHA); }
inline bool isblank(char value) { return ASCII::Is(value,ASCII::BLANK); }
inline bool iscntrl(char value) { return ASCII::Is(value,ASCII::CONTROL); }
inline bool isdigit(char value) { return ASCII::Is(value,ASCII::DIGIT); }
inline bool isgraph(char value) { return ASCII::Is(value,ASCII::GRAPH); }
inline bool islower(char value) { return ASCII::Is(value,ASCII::LOWER); }
inline bool isprint(char value) { return ASCII::Is(value,ASCII::PRINT); }
inline bool ispunct(char value) { return ASCII::Is(value,ASCII::PUNCT); }
inline bool isspace(char value) { return  ASCII::Is(value,ASCII::SPACE); }
inline bool isupper(char value) { return ASCII::Is(value,ASCII::UPPER); }
inline bool isxdigit(char value) { return ASCII::Is(value,ASCII::HEXDIGIT); }
inline bool isxml(char value) { return ASCII::Is(value,ASCII::XML); }
inline char tolower(char value) { return ASCII::toLower(value); }
inline char toupper(char value) { return ASCII::toUpper(value); }


const char* strrpbrk(const char* value, const char* markers);
const char *strrstr(const char* where, const char* what);

template<typename Type>
inline Type min(Type value) { return value; }
template<typename Type1, typename Type2, typename ...Args>
inline typename std::conditional<sizeof(Type1) >= sizeof(Type2), Type1, Type2>::type
				  min(Type1 value1, Type2 value2, Args&&... args) { return value1 < value2 ? min(value1, args ...) : min(value2, args ...); }

template<typename Type>
inline Type max(Type value) { return value; }
template<typename Type1, typename Type2, typename ...Args>
inline typename std::conditional<sizeof(Type1) >= sizeof(Type2), Type1, Type2>::type
				  max(Type1 value1, Type2 value2, Args&&... args) { return value1 > value2 ? max(value1, args ...) : max(value2, args ...); }

template<typename RangeType, typename Type>
inline RangeType range(Type value) {
	RangeType result = static_cast<RangeType>(value);
	if (result < 0)
		return value > 0 ? std::numeric_limits<RangeType>::max() : result; // result signed and value unsigned => result max
	return value < 0 ? 0 : result; // result unsigned and value signed => 0
}
const std::string& typeOf(const std::type_info& info);
template<typename ObjectType>
inline const std::string& typeOf(const ObjectType& object) { return typeOf(typeid(object)); }
/*!
Try to prefer this template typeOf version, because is the more faster*/
template<typename ObjectType>
inline const std::string& typeOf() {
	static const std::string& Type(typeOf(typeid(ObjectType)));
	return Type;
}

template <typename T, typename = void>
struct is_iterable : std::false_type {};
template <typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))> > : std::true_type {};

template <typename T, typename = void>
struct is_container : std::false_type {};
template <typename T>
struct is_container<T, std::void_t<decltype(std::declval<T>().emplace(std::begin(std::declval<T>())))>> : std::true_type {};



} // namespace Mona
