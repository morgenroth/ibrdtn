/*
 * MonotonicClock.cpp
 *
 *  Created on: 10.09.2013
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/MonotonicClock.h"
#include <time.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif

#ifdef HAVE_MACH_MACH_TIME_H
#include <mach/mach_time.h>
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace ibrcommon {

	MonotonicClock::MonotonicClock()
	{
	}

	MonotonicClock::~MonotonicClock()
	{
	}

#ifdef __WIN32__
	LARGE_INTEGER MonotonicClock::getFILETIMEoffset()
	{
		SYSTEMTIME s;
		FILETIME f;
		LARGE_INTEGER t;

		s.wYear = 1970;
		s.wMonth = 1;
		s.wDay = 1;
		s.wHour = 0;
		s.wMinute = 0;
		s.wSecond = 0;
		s.wMilliseconds = 0;
		SystemTimeToFileTime(&s, &f);
		t.QuadPart = f.dwHighDateTime;
		t.QuadPart <<= 32;
		t.QuadPart |= f.dwLowDateTime;
		return (t);
	}
#endif

	void MonotonicClock::gettime(struct timeval &tv)
	{
		struct timespec ts;
		gettime(ts);

		tv.tv_sec = ts.tv_sec;
		tv.tv_usec = ts.tv_nsec / 1000;
	}

	void MonotonicClock::gettime(struct timespec &ts)
	{
#ifdef __WIN32__
		LARGE_INTEGER t;
		FILETIME f;
		double microseconds;
		static LARGE_INTEGER offset;
		static double frequencyToMicroseconds;
		static int initialized = 0;
		static BOOL usePerformanceCounter = 0;

		if (!initialized) {
			LARGE_INTEGER performanceFrequency;
			initialized = 1;
			usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
			if (usePerformanceCounter) {
				QueryPerformanceCounter(&offset);
				frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
			} else {
				offset = getFILETIMEoffset();
				frequencyToMicroseconds = 10.;
			}
		}
		if (usePerformanceCounter) QueryPerformanceCounter(&t);
		else {
			GetSystemTimeAsFileTime(&f);
			t.QuadPart = f.dwHighDateTime;
			t.QuadPart <<= 32;
			t.QuadPart |= f.dwLowDateTime;
		}

		t.QuadPart -= offset.QuadPart;
		microseconds = (double)t.QuadPart / frequencyToMicroseconds;
		t.QuadPart = microseconds;
		ts.tv_sec = t.QuadPart / 1000000;
		ts.tv_nsec = (t.QuadPart % 1000000) * 1000;
#elif HAVE_MACH_MACH_TIME_H // OS X does not have clock_gettime, use clock_get_time
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), REALTIME_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		ts.tv_sec = mts.tv_sec;
		ts.tv_nsec = mts.tv_nsec;
#else
#ifdef CLOCK_BOOTTIME
		// use BOOTTIME as monotonic clock
		if (::clock_gettime(CLOCK_BOOTTIME, &ts) != 0)
		{
			// fall-back to monotonic clock if boot-time is not available
			::clock_gettime(CLOCK_MONOTONIC, &ts);
		}
#else
		::clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
#endif
	}

	void MonotonicClock::diff(const struct timespec &start, const struct timespec &end, struct timespec &diff)
	{
		if ((end.tv_nsec - start.tv_nsec) < 0)
		{
			diff.tv_sec = end.tv_sec - start.tv_sec - 1;
			diff.tv_nsec = (1000000000 + end.tv_nsec) - start.tv_nsec;
		}
		else
		{
			diff.tv_sec = end.tv_sec - start.tv_sec;
			diff.tv_nsec = end.tv_nsec - start.tv_nsec;
		}
	}

} /* namespace ibrcommon */
