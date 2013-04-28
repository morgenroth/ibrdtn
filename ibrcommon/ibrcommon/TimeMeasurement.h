/*
 * TimeMeasurement.h
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

		size_t getNanoseconds() const;
		size_t getMicroseconds() const;
		size_t getMilliseconds() const;
		size_t getSeconds() const;

		friend std::ostream &operator<<(std::ostream &stream, const TimeMeasurement &measurement);

		static std::ostream& format(std::ostream &stream, const float value);

	private:
		static ssize_t timespecDiff(const struct timespec *timeA_p, const struct timespec *timeB_p);
		static ssize_t timespecDiff(const size_t &stop, const size_t &start);

		struct timespec _start;
		struct timespec _end;

		size_t _uint64_start;
		size_t _uint64_end;
	};
}

#endif /* TIMEMEASUREMENT_H_ */
