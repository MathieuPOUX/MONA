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

namespace Mona {


template <typename Type>
struct Format {
	Format() {
		// Check that Type implement correctly a stringify method
		static_assert(
            std::is_same_v<decltype(std::declval<const Type>().template stringify<std::string>(std::declval<std::string&>())), void>,
            "Must implement void stringify(OutType& out)"
        );
	}
	std::string str() const {
		std::string value;
		to(value);
		return value;
	}
	operator std::string() const {
		return str();
	}
	template<typename OutType>
	OutType& to(OutType& out) const {
		static_cast<const Type&>(*this).template stringify<OutType>(out);
		return out;
	}
};


} // namespace Mona
