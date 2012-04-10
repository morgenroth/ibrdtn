/*
 * DTNTime.cpp
 *
 *  Created on: 15.12.2009
 *      Author: morgenro
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

		DTNTime::DTNTime(size_t seconds, size_t nanoseconds)
		 : _seconds(seconds), _nanoseconds(nanoseconds)
		{
		}

		DTNTime::DTNTime(SDNV seconds, SDNV nanoseconds)
		 : _seconds(seconds), _nanoseconds(nanoseconds)
		{
		}

		DTNTime::~DTNTime()
		{
		}

		void DTNTime::set()
		{
			timeval tv;
			dtn::utils::Clock::gettimeofday(&tv);
			_seconds = tv.tv_sec;
			_nanoseconds = SDNV(tv.tv_usec * 1000);
		}

		size_t DTNTime::getLength() const
		{
			return _seconds.getLength() + _nanoseconds.getLength();
		}

		SDNV DTNTime::getTimestamp() const
		{
			return _seconds;
		}

		void DTNTime::operator+=(const size_t value)
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
