/*
 * DTNTime.h
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

#ifndef DTNTIME_H_
#define DTNTIME_H_

#include "ibrdtn/data/Number.h"
#include <stdint.h>

namespace dtn
{
	namespace data
	{
		class DTNTime
		{
		public:
			DTNTime();
			DTNTime(const Timestamp &seconds, const Number &nanoseconds = 0);
			virtual ~DTNTime();

			const Timestamp& getTimestamp() const;
			const Number& getNanoseconds() const;

			/**
			 * set the DTNTime to the current time
			 */
			void set();

			void operator+=(const Timestamp &value);

			Length getLength() const;

		private:
			friend std::ostream &operator<<(std::ostream &stream, const dtn::data::DTNTime &obj);
			friend std::istream &operator>>(std::istream &stream, dtn::data::DTNTime &obj);

			Timestamp _seconds;
			Number _nanoseconds;
		};
	}
}


#endif /* DTNTIME_H_ */
