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

#include "ibrdtn/utils/Clock.h"
#include "ibrdtn/data/AgeBlock.h"

#include <ibrcommon/Logger.h>
#include <iomanip>

namespace dtn
{
	namespace utils
	{
		int Clock::_timezone = 0;
		double Clock::_rating = 0;
		bool Clock::_badclock = false;

		struct timeval Clock::_offset;
		bool Clock::_offset_init = false;

		bool Clock::_modify_clock = false;

		/**
		 * The number of seconds between 1/1/1970 and 1/1/2000.
		 */
		const uint32_t Clock::TIMEVAL_CONVERSION = 946684800;

		Clock::Clock()
		{
		}

		Clock::~Clock()
		{
		}

		bool Clock::isBad()
		{
			return _badclock;
		}

		void Clock::setBad(bool val)
		{
			_badclock = val;
		}

		int Clock::getTimezone()
		{
			return _timezone;
		}

		void Clock::setTimezone(int val)
		{
			_timezone = val;
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

		size_t Clock::getExpireTime(const dtn::data::Bundle &b)
		{
			if ((b._timestamp == 0) || dtn::utils::Clock::isBad())
			{
				// use the AgeBlock to verify the age
				try {
					const dtn::data::AgeBlock &agebl = b.find<const dtn::data::AgeBlock>();
					size_t seconds_left = 0;
					if (b._lifetime > agebl.getSeconds()) {
						seconds_left = b._lifetime - agebl.getSeconds();
					}
					return getTime() + seconds_left;
				} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };
			}

			return __getExpireTime(b._timestamp, b._lifetime);
		}

		size_t Clock::getExpireTime(size_t timestamp, size_t lifetime)
		{
			return __getExpireTime(timestamp, lifetime);
		}

		size_t Clock::getExpireTime(size_t lifetime)
		{
			return __getExpireTime(getTime(), lifetime);
		}

		size_t Clock::getLifetime(const dtn::data::BundleID &id, size_t expiretime)
		{
			// if the timestamp of the bundle is larger than the expiretime
			// the bundle is invalid
			if (id.timestamp > expiretime) return 0;

			// else the lifetime is the difference between the timestamp and the expiretime
			return id.timestamp - expiretime;
		}

		size_t Clock::__getExpireTime(size_t timestamp, size_t lifetime)
		{
			// if the quality of time is zero, return standard expire time
			if (Clock::getRating() == 0) return timestamp + lifetime;

			// calculate sigma based on the quality of time and the original lifetime
			size_t sigma = lifetime * (1 - Clock::getRating());

			// expiration adjusted by quality of time
			return timestamp + lifetime + sigma;
		}

		bool Clock::isExpired(const dtn::data::Bundle &b)
		{
			// use the AgeBlock to verify the age
			try {
				const dtn::data::AgeBlock &agebl = b.find<const dtn::data::AgeBlock>();
				return (b._lifetime < agebl.getSeconds());
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

			return __isExpired(b._timestamp, b._lifetime);
		}

		bool Clock::isExpired(size_t timestamp, size_t lifetime)
		{
			return __isExpired(timestamp, lifetime);
		}

		bool Clock::__isExpired(size_t timestamp, size_t lifetime)
		{
			// if the quality of time is zero or the clock is bad, then never expire a bundle
			if ((Clock::getRating() == 0) || dtn::utils::Clock::isBad()) return false;

			// calculate sigma based on the quality of time and the original lifetime
			size_t sigma = lifetime * (1 - Clock::getRating());

			// expiration adjusted by quality of time
			if ( Clock::getTime() > (timestamp + lifetime + sigma)) return true;

			return false;
		}

		size_t Clock::getTime()
		{
			struct timeval now;
			Clock::gettimeofday(&now);

			// timezone
			int offset = Clock::getTimezone() * 3600;

			// do we believe we are before the year 2000?
			if ((u_int)now.tv_sec < TIMEVAL_CONVERSION)
			{
				return 0;
			}

			return (now.tv_sec - TIMEVAL_CONVERSION) + offset;
		}

		size_t Clock::getUnixTimestamp()
		{
			struct timeval now;
			Clock::gettimeofday(&now);

			// timezone
			int offset = Clock::getTimezone() * 3600;

			return now.tv_sec + offset;
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

				// set the local clock to the new timestamp
				::settimeofday(&now, &tz);
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
				// set the local clock to the new timestamp
				::settimeofday(tv, &tz);
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

		double Clock::toDouble(const timeval &val) {
			return val.tv_sec + (val.tv_usec / 1000000.0);
		}
	}
}
