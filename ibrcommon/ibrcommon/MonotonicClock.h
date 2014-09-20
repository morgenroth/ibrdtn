/*
 * MonotonicClock.h
 *
 *  Created on: 10.09.2013
 *      Author: morgenro
 */

#ifndef MONOTONICCLOCK_H_
#define MONOTONICCLOCK_H_

#include <sys/time.h>
#include <sys/types.h>
#include <stdint.h>

/**
 * On windows the timespec is defined in the pthread
 * headers.
 */
#ifdef __WIN32__
#include <windows.h>
#include <pthread.h>
#endif

namespace ibrcommon {

	class MonotonicClock {
	public:
		MonotonicClock();
		virtual ~MonotonicClock();

		static void gettime(struct timeval &tv);
		static void gettime(struct timespec &ts);

		static void diff(const struct timespec &start, const struct timespec &end, struct timespec &diff);

	private:
#ifdef __WIN32__
		static LARGE_INTEGER getFILETIMEoffset();
#endif
	};

} /* namespace ctrl */
#endif /* MONOTONICCLOCK_H_ */
