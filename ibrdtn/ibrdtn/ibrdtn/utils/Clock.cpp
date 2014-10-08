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

		struct timeval Clock::_offset = Clock::__initialize_clock();
		struct timeval Clock::__clock_reference = Clock::__get_clock_reference();

		bool Clock::_modify_clock = false;

		/**
		 * The number of seconds between 1/1/1970 and 1/1/2000.
		 */
		const dtn::data::Timestamp Clock::TIMEVAL_CONVERSION = 946684800;

		/**
		 * Class to generate a monotonic timestamp
		 */
		struct timespec Clock::_uptime_reference = Clock::__get_monotonic_ts();

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
			struct timeval tv;

			// retrieve assumed clock
			gettimeofday(&tv);

			// switch clock mode
			_modify_clock = val;

			// set clock initially
			settimeofday(&tv);
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
			struct timespec ts;
			ibrcommon::MonotonicClock::gettime(ts);
			return dtn::data::Timestamp(ts.tv_sec);
		}

		const struct timeval& Clock::getOffset()
		{
			return Clock::_offset;
		}

		void Clock::setOffset(const struct timeval &tv)
		{
			// do not modify time on reference nodes
			if (Clock::getRating() == 1.0) return;

			if (!Clock::shouldModifyClock())
			{
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
			struct timeval now;
			struct timezone tz;

			// do not modify time on reference nodes
			if (Clock::getRating() == 1.0) return;

			if (!Clock::shouldModifyClock())
			{
				// get the monotonic time of day
				__monotonic_gettimeofday(&now);

				// determine the new offset
				timersub(&now, tv, &Clock::_offset);
			}
			else
			{
				// get the hosts time of day
				::gettimeofday(&now, &tz);

				// determine the new offset
				timersub(&now, tv, &Clock::_offset);

#ifndef __WIN32__
				// set the local clock to the new timestamp
				::settimeofday(tv, &tz);
#endif
			}

			IBRCOMMON_LOGGER_TAG("Clock", info) << "new local offset: " << Clock::toDouble(_offset) << "s" << IBRCOMMON_LOGGER_ENDL;
		}

		void Clock::gettimeofday(struct timeval *tv)
		{
			if (!Clock::shouldModifyClock() && Clock::getRating() < 1.0)
			{
				// get the monotonic time of day
				__monotonic_gettimeofday(tv);

				// add offset
				timersub(tv, &Clock::_offset, tv);
			}
			else
			{
				struct timezone tz;
				::gettimeofday(tv, &tz);
			}
		}

		void Clock::getdtntimeofday(struct timeval *tv)
		{
			// query local time
			Clock::gettimeofday(tv);

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
			struct timespec now;
			struct timespec result;
			ibrcommon::MonotonicClock::gettime(now);
			ibrcommon::MonotonicClock::diff(_uptime_reference, now, result);

			return dtn::data::Timestamp(result.tv_sec);
		}

		struct timespec Clock::__get_monotonic_ts()
		{
			struct timespec ts;

			// initialize up-time reference
			ibrcommon::MonotonicClock::gettime(ts);
			return ts;
		}

		struct timeval Clock::__get_clock_reference()
		{
			struct timezone tz;
			struct timeval now, tv;

			::gettimeofday(&now, &tz);
			ibrcommon::MonotonicClock::gettime(tv);

			timersub(&now, &tv, &now);

			return now;
		}

		void Clock::__monotonic_gettimeofday(struct timeval *tv)
		{
			// use boot-time as reference to stay independent from
			// external clock adjustments
			struct timeval mtv;
			ibrcommon::MonotonicClock::gettime(mtv);

			// combine clock reference and current boot-time
			timeradd(&__clock_reference, &mtv, tv);
		}

		struct timeval Clock::__initialize_clock()
		{
			struct timeval ret;
			timerclear(&ret);
			return ret;

		}
	}
}
