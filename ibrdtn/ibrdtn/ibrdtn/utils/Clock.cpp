/*
 * Clock.cpp
 *
 *  Created on: 24.06.2010
 *      Author: morgenro
 */

#include "ibrdtn/utils/Clock.h"
#include "ibrdtn/data/AgeBlock.h"

#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace utils
	{
		int Clock::timezone = 0;
		float Clock::quality = 0;
		bool Clock::badclock = false;

		struct timeval Clock::_offset;
		bool Clock::_offset_init = false;

		bool Clock::modify_clock = false;

		/**
		 * The number of seconds between 1/1/1970 and 1/1/2000.
		 */
		u_int32_t Clock::TIMEVAL_CONVERSION = 946684800;

		Clock::Clock()
		{
		}

		Clock::~Clock()
		{
		}

		size_t Clock::getExpireTime(const dtn::data::Bundle &b)
		{
			if ((b._timestamp == 0) || dtn::utils::Clock::badclock)
			{
				// use the AgeBlock to verify the age
				try {
					const dtn::data::AgeBlock &agebl = b.getBlock<const dtn::data::AgeBlock>();
					size_t seconds_left = b._lifetime - agebl.getSeconds();
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

		size_t Clock::__getExpireTime(size_t timestamp, size_t lifetime)
		{
			// if the quality of time is zero, return standard expire time
			if (Clock::quality == 0) return timestamp + lifetime;

			// calculate sigma based on the quality of time and the original lifetime
			size_t sigma = lifetime * (1 - Clock::quality);

			// expiration adjusted by quality of time
			return timestamp + lifetime + sigma;
		}

		bool Clock::isExpired(const dtn::data::Bundle &b)
		{
			// use the AgeBlock to verify the age
			try {
				const dtn::data::AgeBlock &agebl = b.getBlock<const dtn::data::AgeBlock>();
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
			if ((Clock::quality == 0) || dtn::utils::Clock::badclock) return false;

			// calculate sigma based on the quality of time and the original lifetime
			size_t sigma = lifetime * (1 - Clock::quality);

			// expiration adjusted by quality of time
			if ( Clock::getTime() > (timestamp + lifetime + sigma)) return true;

			return false;
		}

		size_t Clock::getTime()
		{
			struct timeval now;
			Clock::gettimeofday(&now);

			// timezone
			int offset = Clock::timezone * 3600;

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
			int offset = Clock::timezone * 3600;

			return now.tv_sec + offset;
		}

		void Clock::setOffset(struct timeval &tv)
		{
			if (!modify_clock)
			{
				if (!Clock::_offset_init)
				{
					timerclear(&Clock::_offset);
					Clock::_offset_init = true;
				}
				timeradd(&Clock::_offset, &tv, &Clock::_offset);
				IBRCOMMON_LOGGER(info) << "[Clock] new local offset: " << _offset.tv_sec << " seconds and " << _offset.tv_usec << " microseconds" << IBRCOMMON_LOGGER_ENDL;
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

			if (!modify_clock)
			{
				if (!Clock::_offset_init)
				{
					timerclear(&Clock::_offset);
					Clock::_offset_init = true;
				}
				timersub(&now, tv, &Clock::_offset);
				IBRCOMMON_LOGGER(info) << "[Clock] new local offset: " << _offset.tv_sec << " seconds and " << _offset.tv_usec << " microseconds" << IBRCOMMON_LOGGER_ENDL;
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
			if (!modify_clock)
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
	}
}
