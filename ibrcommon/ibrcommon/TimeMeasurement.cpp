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
#include "ibrcommon/MonotonicClock.h"

#include <stdio.h>
#include <iostream>
#include <iomanip>

namespace ibrcommon
{
	TimeMeasurement::TimeMeasurement()
	{
		start();
		_end.tv_sec = _start.tv_sec;
		_end.tv_nsec = _start.tv_nsec;
	}

	TimeMeasurement::~TimeMeasurement()
	{
	}

	void TimeMeasurement::start()
	{
		// set start time
		MonotonicClock::gettime(_start);
	}

	void TimeMeasurement::stop()
	{
		// set stop time
		MonotonicClock::gettime(_end);
	}

	double TimeMeasurement::getMilliseconds() const
	{
		struct timespec diff;
		getTime(diff);

		// merge seconds and nanoseconds
		double delay_ms = ((double)diff.tv_sec * 1000.0) + ((double)diff.tv_nsec / 1000000.0);

		return delay_ms;
	}

	double TimeMeasurement::getMicroseconds() const
	{
		struct timespec diff;
		getTime(diff);

		// merge seconds and nanoseconds
		double delay_us = ((double)diff.tv_sec * 1000000.0) + ((double)diff.tv_nsec / 1000.0);

		return delay_us;
	}

	time_t TimeMeasurement::getSeconds() const
	{
		return _end.tv_sec - _start.tv_sec;
	}

	std::ostream& TimeMeasurement::format(std::ostream &stream, const double value)
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
		struct timespec diff;
		measurement.getTime(diff);

		// convert nanoseconds to milliseconds
		double delay_ms = (double)diff.tv_nsec / 1000000.0;

		if (diff.tv_sec < 1)
		{
			TimeMeasurement::format(stream, delay_ms); stream << " ms";
			return stream;
		}

		// convert time interval to seconds
		double seconds = (double)diff.tv_sec + (delay_ms / 1000.0);

		if (diff.tv_sec >= 3600)
		{
			TimeMeasurement::format(stream, seconds / 3600.0); stream << " h";
		}
		else if (diff.tv_sec >= 60)
		{
			TimeMeasurement::format(stream, seconds / 60.0); stream << " m";
		}
		else
		{
			TimeMeasurement::format(stream, seconds); stream << " s";
		}

		return stream;
	}

	void TimeMeasurement::getTime(struct timespec &diff) const
	{
		MonotonicClock::diff(_start, _end, diff);
	}
}
