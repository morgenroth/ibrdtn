/*
 * Clock.cpp
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

#include "ibrdtn/config.h"
#include "ibrdtn/utils/Clock.h"
#include "ibrdtn/data/AgeBlock.h"
#include "ibrdtn/data/MetaBundle.h"

#include <ibrcommon/Logger.h>
#include <iomanip>

namespace dtn
{
	namespace utils
	{
		double Clock::_rating = 1.0;

		struct timeval Clock::_offset;
		bool Clock::_offset_init = false;

		bool Clock::_modify_clock = false;

		/**
		 * The number of seconds between 1/1/1970 and 1/1/2000.
		 */
		const dtn::data::Timestamp Clock::TIMEVAL_CONVERSION = 946684800;

		/**
		 * Class to generate a monotonic timestamp
		 */
		ibrcommon::MonotonicClock Clock::_monotonic_clock;

		Clock::Clock()
		{
		}

		Clock::~Clock()
		{
		}

		double Clock::getRating()
		{
			return _rating;
		}

		void Clock::setRating(double val)
		{
			_rating = val;

			// debug quality of time
			IBRCOMMON_LOGGER_DEBUG_TAG("Clock", 25) << "new clock rating is " << std::setprecision(16) << val << IBRCOMMON_LOGGER_ENDL;
		}

		bool Clock::shouldModifyClock()
		{
			return _modify_clock;
		}

		void Clock::setModifyClock(bool val)
		{
			_modify_clock = val;
		}

		dtn::data::Timestamp Clock::getExpireTime(const dtn::data::Bundle &b)
		{
			if (b.timestamp == 0)
			{
				try {
					// use the AgeBlock to verify the age
					const dtn::data::AgeBlock &agebl = b.find<dtn::data::AgeBlock>();
					dtn::data::Number seconds_left = 0;
					if (b.lifetime > agebl.getSeconds()) {
						seconds_left = b.lifetime - agebl.getSeconds();
					}
					return getTime() + seconds_left;
				} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
					// if there is no AgeBlock this bundle is not valid
					return 0;
				}
			}

			return __getExpireTime(b.timestamp, b.lifetime);
		}

		dtn::data::Timestamp Clock::getExpireTime(const dtn::data::Timestamp &timestamp, const dtn::data::Number &lifetime)
		{
			return __getExpireTime(timestamp, lifetime);
		}

		dtn::data::Timestamp Clock::getExpireTime(const dtn::data::Number &lifetime)
		{
			return __getExpireTime(getTime(), lifetime);
		}

		dtn::data::Number Clock::__getExpireTime(const dtn::data::Timestamp &timestamp, const dtn::data::Number &lifetime)
		{
			// if our own clock or the bundles timestamp is invalid use the current timestamp
			if ((getRating() == 0.0) || timestamp == 0) return getTime() + lifetime;

			return timestamp + lifetime;
		}

		bool Clock::isExpired(const dtn::data::Bundle &b)
		{
			if (b.timestamp == 0)
			{
				// use the AgeBlock to verify the age
				try {
					const dtn::data::AgeBlock &agebl = b.find<dtn::data::AgeBlock>();
					return (b.lifetime < agebl.getSeconds());
				} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
					// if there is no AgeBlock this bundle is not valid
					return true;
				}
			}

			return isExpired(b.timestamp, b.lifetime);
		}

		bool Clock::isExpired(const dtn::data::MetaBundle &m)
		{
			// expiration adjusted by quality of time
			if ( Clock::getTime() > m.expiretime ) return true;

			return false;
		}

		bool Clock::isExpired(const dtn::data::Timestamp &timestamp, const dtn::data::Number &lifetime)
		{
			// can not check invalid timestamp
			// assume bundle has an age block and trust on later checks
			if (timestamp == 0) return false;

			// disable expiration if clock rating is too bad
			if (getRating() == 0.0) return false;

			return Clock::getTime() > __getExpireTime(timestamp, lifetime);
		}

		dtn::data::Timestamp Clock::getTime()
		{
			struct timeval now;
			Clock::getdtntimeofday(&now);

			return now.tv_sec;
		}

		dtn::data::Timestamp Clock::getMonotonicTimestamp()
		{
			return _monotonic_clock.getSeconds();
		}

		const struct timeval& Clock::getOffset()
		{
			return Clock::_offset;
		}

		void Clock::setOffset(const struct timeval &tv)
		{
			if (!Clock::shouldModifyClock())
			{
				if (!Clock::_offset_init)
				{
					timerclear(&Clock::_offset);
					Clock::_offset_init = true;
				}

				timeradd(&Clock::_offset, &tv, &Clock::_offset);
				IBRCOMMON_LOGGER_TAG("Clock", info) << "new local offset: " << Clock::toDouble(_offset) << "s" << IBRCOMMON_LOGGER_ENDL;
			}
			else
			{
				struct timezone tz;
				struct timeval now;
				::gettimeofday(&now, &tz);

				// adjust by the offset
				timersub(&now, &tv, &now);

#ifndef __WIN32__
				// set the local clock to the new timestamp
				::settimeofday(&now, &tz);
#endif
			}
		}

		void Clock::settimeofday(struct timeval *tv)
		{
			struct timezone tz;
			struct timeval now;
			::gettimeofday(&now, &tz);

			if (!Clock::shouldModifyClock())
			{
				if (!Clock::_offset_init)
				{
					timerclear(&Clock::_offset);
					Clock::_offset_init = true;
				}
				timersub(&now, tv, &Clock::_offset);
				IBRCOMMON_LOGGER_TAG("Clock", info) << "new local offset: " << Clock::toDouble(_offset) << "s" << IBRCOMMON_LOGGER_ENDL;
			}
			else
			{
#ifndef __WIN32__
				// set the local clock to the new timestamp
				::settimeofday(tv, &tz);
#endif
			}
		}

		void Clock::gettimeofday(struct timeval *tv)
		{
			struct timezone tz;
			::gettimeofday(tv, &tz);

			// correct by the local offset
			if (!Clock::shouldModifyClock())
			{
				if (!Clock::_offset_init)
				{
					timerclear(&Clock::_offset);
					Clock::_offset_init = true;
				}

				// add offset
				timersub(tv, &Clock::_offset, tv);
			}
		}

		void Clock::getdtntimeofday(struct timeval *tv)
		{
			struct timezone tz;
			::gettimeofday(tv, &tz);

			// correct by the local offset
			if (!Clock::shouldModifyClock())
			{
				if (!Clock::_offset_init)
				{
					timerclear(&Clock::_offset);
					Clock::_offset_init = true;
				}

				// add offset
				timersub(tv, &Clock::_offset, tv);
			}

			// do we believe we are before the year 2000?
			if (dtn::data::Timestamp(tv->tv_sec) < TIMEVAL_CONVERSION)
			{
				tv->tv_sec = 0;
			}
			else
			{
				// do bundle protocol time conversion
				tv->tv_sec -= TIMEVAL_CONVERSION.get<time_t>();
			}
		}

		double Clock::toDouble(const timeval &val) {
			return static_cast<double>(val.tv_sec) + (static_cast<double>(val.tv_usec) / 1000000.0);
		}

		dtn::data::Timestamp Clock::getUptime()
		{
			return Clock::getMonotonicTimestamp();
		}
	}
}
