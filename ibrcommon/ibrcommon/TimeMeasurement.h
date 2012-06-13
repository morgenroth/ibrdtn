/*
 * TimeMeasurement.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef TIMEMEASUREMENT_H_
#define TIMEMEASUREMENT_H_

#include <time.h>
#include <iostream>
#include <sys/types.h>
#include <stdint.h>

namespace ibrcommon
{
	class TimeMeasurement
	{
	public:
		TimeMeasurement();
		virtual ~TimeMeasurement();

		void start();
		void stop();

		u_int64_t getNanoseconds() const;
		float getMicroseconds() const;
		float getMilliseconds() const;
		float getSeconds() const;

		friend std::ostream &operator<<(std::ostream &stream, const TimeMeasurement &measurement);

		static std::ostream& format(std::ostream &stream, const float value);

	private:
		static int64_t timespecDiff(const struct timespec *timeA_p, const struct timespec *timeB_p);
		static int64_t timespecDiff(const uint64_t &stop, const uint64_t &start);

		struct timespec _start;
		struct timespec _end;

		uint64_t _uint64_start;
		uint64_t _uint64_end;
	};
}

#endif /* TIMEMEASUREMENT_H_ */
