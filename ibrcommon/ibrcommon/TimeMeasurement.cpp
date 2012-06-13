/*
 * TimeMeasurement.cpp
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

#include "ibrcommon/config.h"
#include "ibrcommon/TimeMeasurement.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

#ifdef HAVE_MACH_MACH_TIME_H
#include <mach/mach_time.h>
#endif

namespace ibrcommon
{
	TimeMeasurement::TimeMeasurement()
	{
		start(); stop();
	}

	TimeMeasurement::~TimeMeasurement()
	{
	}

	void TimeMeasurement::start()
	{
#ifdef HAVE_MACH_MACH_TIME_H
		_uint64_start = mach_absolute_time();
#else
		// set sending time
		clock_gettime(CLOCK_MONOTONIC, &_start);

#endif
	}

	void TimeMeasurement::stop()
	{
#ifdef HAVE_MACH_MACH_TIME_H
		_uint64_end = mach_absolute_time();
#else
		// set receiving time
		clock_gettime(CLOCK_MONOTONIC, &_end);
#endif
	}

	float TimeMeasurement::getMilliseconds() const
	{
		// calc difference
		u_int64_t timeElapsed = getNanoseconds();

		// make it readable
		float delay_ms = (float)timeElapsed / 1000000;

		return delay_ms;
	}

	u_int64_t TimeMeasurement::getNanoseconds() const
	{
		// calc difference
#ifdef HAVE_MACH_MACH_TIME_H
		int64_t val = TimeMeasurement::timespecDiff(_uint64_end, _uint64_start);
#else
		int64_t val = TimeMeasurement::timespecDiff(&_end, &_start);
#endif

		if (val < 0) return 0;
		return val;
	}

	float TimeMeasurement::getMicroseconds() const
	{
		// calc difference
		u_int64_t timeElapsed = getNanoseconds();

		// make it readable
		float delay_m = (float)timeElapsed / 1000;

		return delay_m;
	}

	float TimeMeasurement::getSeconds() const
	{
		return getMilliseconds() / 1000;
	}

	std::ostream& TimeMeasurement::format(std::ostream &stream, const float value)
	{
#ifdef __UCLIBC__
		char buf[32];
		snprintf(buf, 32, "%4.2f", value);
		stream << std::string(buf);
#else
		stream << std::setiosflags(std::ios::fixed) << std::setprecision(2) << value;
#endif
		return stream;
	}

	std::ostream &operator<<(std::ostream &stream, const TimeMeasurement &measurement)
	{
		// calc difference
#ifdef HAVE_MACH_MACH_TIME_H
		u_int64_t timeElapsed = TimeMeasurement::timespecDiff(measurement._uint64_end, measurement._uint64_start);
#else
		u_int64_t timeElapsed = TimeMeasurement::timespecDiff(&(measurement._end), &(measurement._start));
#endif

		// make it readable
		float delay_ms = (float)timeElapsed / 1000000;
		float delay_sec = delay_ms / 1000;
		float delay_min = delay_sec / 60;
		float delay_h = delay_min / 60;

		if (delay_h > 1)
		{
			TimeMeasurement::format(stream, delay_h); stream << " h";
		}
		else if (delay_min > 1)
		{
			TimeMeasurement::format(stream, delay_min); stream << " m";
		}
		else if (delay_sec > 1)
		{
			TimeMeasurement::format(stream, delay_sec); stream << " s";
		}
		else
		{
			TimeMeasurement::format(stream, delay_ms); stream << " ms";
		}

		return stream;
	}

	int64_t TimeMeasurement::timespecDiff(const uint64_t &timeA, const uint64_t &timeB)
	{
		int64_t duration = timeA - timeB;

#ifdef HAVE_MACH_MACH_TIME_H
		mach_timebase_info_data_t info;
		mach_timebase_info(&info);

		/* Convert to nanoseconds */
		duration *= info.numer;
		duration /= info.denom;
#endif
		return duration;
	}

	int64_t TimeMeasurement::timespecDiff(const struct timespec *timeA_p, const struct timespec *timeB_p)
	{
		//Casting to 64Bit, otherwise it caps out at ~ 5 secs for 32bit machines
		return ( ( (int64_t)(timeA_p->tv_sec) * 1e9 + (int64_t)(timeA_p->tv_nsec)) -  ( (int64_t)(timeB_p->tv_sec) * 1e9 + (int64_t)(timeB_p->tv_nsec)) );
	}
}
