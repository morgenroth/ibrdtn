/*
 * Clock.h
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

#ifndef CLOCK_H_
#define CLOCK_H_

#include <sys/types.h>
#include <sys/time.h>

#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/BundleID.h"

namespace dtn
{
	namespace utils
	{
		class Clock
		{
		public:
			/**
			 * Return the current unix timestamp adjusted by
			 * the configured timezone offset
			 */
			static size_t getUnixTimestamp();

			/**
			 * Return the current DTN timestamp adjusted by
			 * the configured timezone offset
			 */
			static size_t getTime();

			/**
			 * Check if a bundle is expired
			 * @return True if the bundle is expired
			 */
			static bool isExpired(const dtn::data::Bundle &b);

			/**
			 * This method is deprecated because it does not recognize the AgeBlock
			 * as alternative age verification.
			 */
			static bool isExpired(size_t timestamp, size_t lifetime = 0) __attribute__ ((deprecated));

			/**
			 * Return the time of expiration of the given bundle
			 */
			static size_t getExpireTime(const dtn::data::Bundle &b);

			/**
			 * This method is deprecated because it does not recognize the AgeBlock
			 * as alternative age verification.
			 */
			static size_t getExpireTime(size_t timestamp, size_t lifetime) __attribute__ ((deprecated));

			/**
			 * Returns the timestamp when this lifetime is going to be expired
			 * depending on the current knowledge of time.
			 * @param lifetime The lifetime in seconds.
			 * @return A DTN timestamp.
			 */
			static size_t getExpireTime(size_t lifetime);

			/**
			 * Returns the calculated lifetime in seconds based on the BundleID and the
			 * expiretime
			 */
			static size_t getLifetime(const dtn::data::BundleID &id, size_t expiretime);

			/**
			 * Tells the internal clock the offset to the common network time.
			 */
			static void settimeofday(struct timeval *tv);

			/**
			 * Get the time of the day like ::gettimeofday(), but
			 * correct the value by the known clock offset.
			 * @param tv
			 */
			static void gettimeofday(struct timeval *tv);

			/**
			 * set the local offset of the clock
			 * @param tv
			 */
			static void setOffset(const struct timeval &tv);

			static const uint32_t TIMEVAL_CONVERSION;

			/**
			 * If set to true, all time based functions assume a bad clock and try to use other mechanisms
			 * to detect expiration.
			 * @return True, if the local clock is marked as bad
			 */
			static bool isBad();

			/**
			 * Set the bad state of the clock returned by Clock::isBad()
			 */
			static void setBad(bool val);

			/**
			 * Specify a timezone offset in hours
			 */
			static int getTimezone();

			/**
			 * Set a timezone offset in hours
			 */
			static void setTimezone(int val);

			/**
			 * Defines an estimation about the precision of the local time. If the clock is definitely wrong
			 * the value is zero and one when we have a perfect time sync. Everything between one and zero gives
			 * an abstract knowledge about the rating of the local clock.
			 */
			static double getRating();

			/**
			 * Set the rating returned by Clock::getRating()
			 */
			static void setRating(double val);

			/**
			 * if set to true, the function settimeofday() and setOffset() will modify the clock of the host
			 * instead of storing the local offset.
			 */
			static bool shouldModifyClock();

			/**
			 * Set the clock modification parameter returned by Clock::shouldModifyClock()
			 */
			static void setModifyClock(bool val);

			/**
			 * Converts a timeval to a double value
			 */
			static double toDouble(const timeval &val);

		private:
			/**
			 * Timezone offset in hours
			 */
			static int _timezone;

			/**
			 * Defines an estimation about the precision of the local time. If the clock is definitely wrong
			 * the value is zero and one when we have a perfect time sync. Everything between one and zero gives
			 * an abstract knowledge about the rating of the local clock.
			 */
			static double _rating;

			/**
			 * If set to true, all time based functions assume a bad clock and try to use other mechanisms
			 * to detect expiration.
			 */
			static bool _badclock;

			/**
			 * if set to true, the function settimeofday() and setOffset() will modify the clock of the host
			 * instead of storing the local offset.
			 */
			static bool _modify_clock;

			Clock();
			virtual ~Clock();

			static bool __isExpired(size_t timestamp, size_t lifetime = 0);
			static size_t __getExpireTime(size_t timestamp, size_t lifetime);

			static struct timeval _offset;
			static bool _offset_init;
		};
	}
}

#endif /* CLOCK_H_ */
