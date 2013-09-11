/*
 * MonotonicClock.h
 *
 *  Created on: 10.09.2013
 *      Author: morgenro
 */

#ifndef MONOTONICCLOCK_H_
#define MONOTONICCLOCK_H_

#include <time.h>
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

		void get(struct timespec &ts) const;
		time_t getSeconds() const;

		static void gettime(struct timespec &ts);

		static void diff(const struct timespec &start, const struct timespec &end, struct timespec &diff);

	private:
#ifdef __WIN32__
		static LARGE_INTEGER getFILETIMEoffset() const;
#endif

		struct timespec _start;
	};

} /* namespace ctrl */
#endif /* MONOTONICCLOCK_H_ */
