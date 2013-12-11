/*
 * DTNTime.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ibrdtn/data/DTNTime.h"
#include "ibrdtn/utils/Clock.h"
#include <sys/time.h>

namespace dtn
{
	namespace data
	{
		DTNTime::DTNTime()
		 : _seconds(0), _nanoseconds(0)
		{
			set();
		}

		DTNTime::DTNTime(const Timestamp &seconds, const Number &nanoseconds)
		 : _seconds(seconds), _nanoseconds(nanoseconds)
		{
		}

		DTNTime::~DTNTime()
		{
		}

		void DTNTime::set()
		{
			timeval tv;
			dtn::utils::Clock::getdtntimeofday(&tv);
			_seconds = tv.tv_sec;
			_nanoseconds = Number(tv.tv_usec) * 1000;
		}

		Length DTNTime::getLength() const
		{
			return _seconds.getLength() + _nanoseconds.getLength();
		}

		const Number& DTNTime::getTimestamp() const
		{
			return _seconds;
		}

		const Number& DTNTime::getNanoseconds() const
		{
			return _nanoseconds;
		}

		void DTNTime::operator+=(const Timestamp &value)
		{
			_seconds += value;
		}

		std::ostream& operator<<(std::ostream &stream, const dtn::data::DTNTime &obj)
		{
			stream << obj._seconds << obj._nanoseconds;
			return stream;
		}

		std::istream& operator>>(std::istream &stream, dtn::data::DTNTime &obj)
		{
			stream >> obj._seconds;
			stream >> obj._nanoseconds;
			return stream;
		}
	}
}
