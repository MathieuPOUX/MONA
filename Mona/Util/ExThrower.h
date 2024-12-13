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
#include "Mona/Util/Event.h"
#include "Mona/Logs/Logs.h"

namespace Mona {

#define THROW(LEVEL, EX_TYPE, ...) { if(onException) { static thread_local Exception Ex; Ex.set<EX_TYPE>(__VA_ARGS__); onException(LEVEL, Ex); } else LOG(LEVEL, __VA_ARGS__); }
#define THROW_EX(LEVEL, EX) { if(onException) onException(LEVEL, EX); else LOG(LEVEL, EX); }
#define THROW_AUTO(FUNCTION) { if((FUNCTION)) { if(ex)  THROW_EX(WARN, ex); } else { THROW_EX(ERROR, ex); } }

struct ExThrower : virtual Object {
	typedef Event<void(LOG_LEVEL, const Exception& ex)>	ON(Exception);

	enum {
		CRITIC = LOG_CRITIC,
		ERROR = LOG_ERROR,
		WARN = LOG_WARN,
		NOTE = LOG_NOTE,
		INFO = LOG_INFO,
		DEBUG = LOG_DEBUG,
		TRACE = LOG_TRACE,
		DEFAULT = LOG_DEFAULT
	};
protected:
	ExThrower() {}

};

} // namespace Mona

