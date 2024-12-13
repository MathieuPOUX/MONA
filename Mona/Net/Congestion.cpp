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

#include "Mona/Net/Congestion.h"

using namespace std;

namespace Mona {

uint32_t Congestion::operator()(uint32_t duration) const {
	if (!_congested)
		return 0;
	uint64_t elapsed = _congested.elapsed();
	return elapsed > duration ? range<uint32_t>(elapsed) : 0;
}

Congestion& Congestion::operator=(uint64_t queueing) {
	bool congested(queueing && queueing>_lastQueueing);
	_lastQueueing = queueing;
	if (congested) {
		if (!_congested)
			_congested.update();
	} else
		_congested = 0;
	return self;
}


} // namespace Mona
