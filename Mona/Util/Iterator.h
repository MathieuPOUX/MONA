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
#include <iterator>

namespace Mona {

/**
 * Class to help to build a custom iterator
 *
 * Implements a base for your Iterator with the following signature:
```
	struct IteratorBase {
		const value_type& item() const;
		void increment();
		void decrement();
		bool equal(const IteratorBase& other) const;
	};
```
 *	Then you can declare your iterator and const_iterator like this:
```
	using iterator = Iterator<ITERATOR_BASE, false>;
	using const_iterator = Iterator<ITERATOR_BASE, true>;
```
 *	Or make all your object iterable with macro MAKE_ITERABLE.
 *  Ex:
```
	struct Iterable {
		struct IteratorBase {
			long num = 0;
			IteratorBase(long _num) : num(_num) {}
			const long& item() {
				return num;
			}
			void increment() {
				if (num <= 100) {
					++num;
				}
			}
			void decrement() {
				if (num > 0) {
					--num;
				}
			}
			bool equal(const IteratorBase& other) const {
				return num == other.num;
			}
		};
		MAKE_ITERABLE(IteratorBase, 0, 100)
	};

	// Now iterate and displays item
	Iterable iterable;
	for(auto it : iterable) {
		cout << it; // displays 0 to 100
	}

```
 */
template<typename IteratorBase, bool isConst>
struct Iterator : IteratorBase {
	using iterator_category = std::bidirectional_iterator_tag;
	// value_type must be non-const and remove reference!
	using value_type = typename std::remove_const<typename std::remove_reference<decltype(std::declval<IteratorBase>().item())>::type>::type;
	using difference_type = std::ptrdiff_t;
	using reference = std::conditional_t<isConst, const value_type&, value_type&>;
	using pointer = std::conditional_t<isConst, const value_type*, value_type*>;

	template <typename ...Args>
	Iterator(Args&&... args) : IteratorBase(std::forward<Args>(args)...) {}

	Iterator& operator++() {
		IteratorBase::increment();
		return *this;
	}
	Iterator operator++(int) {
		Iterator tmp = *this;
		IteratorBase::increment();
		return tmp;
	}
	Iterator& operator--() {
		IteratorBase::decrement();
		return *this;
	}
	Iterator operator--(int) {
		Iterator tmp = *this;
		IteratorBase::decrement();
		return tmp;
	}
	bool operator==(const Iterator& other) const {
		return IteratorBase::equal(other);
	}
	bool operator!=(const Iterator& other) const {
		return !IteratorBase::equal(other);
	}
	reference operator*() {
		return IteratorBase::item();
	}
	pointer operator->() {
		return &operator*();
	}
};


#define MAKE_ITERABLE(ITERATOR_BASE, BEGIN_PARAMS, END_PARAMS)   \
	using iterator = Iterator<ITERATOR_BASE, false>; \
	using const_iterator = Iterator<ITERATOR_BASE, true>; \
	using reverse_iterator = std::reverse_iterator<iterator>; \
	using const_reverse_iterator = std::reverse_iterator<const_iterator>; \
	iterator begin() { return iterator BEGIN_PARAMS; } \
    iterator end() { return iterator BEGIN_PARAMS; } \
    const_iterator begin() const { return cbegin(); } \
    const_iterator end() const { return cend(); } \
    const_iterator cbegin() const { return const_cast<std::remove_const_t<std::remove_reference_t<decltype(self)>>*>(this)->begin(); } \
    const_iterator cend() const { return const_cast<std::remove_const_t<std::remove_reference_t<decltype(self)>>*>(this)->end(); } \
    reverse_iterator rbegin() { return reverse_iterator(end()); } \
    reverse_iterator rend() { return reverse_iterator(begin()); } \
    const_reverse_iterator rbegin() const { return crbegin(); } \
    const_reverse_iterator rend() const { return crend(); } \
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); } \
	const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }


} // namespace Mona
