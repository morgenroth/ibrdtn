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

#include "ibrdtn/data/SDNV.h"
#include <stdint.h>

namespace dtn
{
	namespace data
	{
		class DTNTime
		{
		public:
			DTNTime();
			DTNTime(uint64_t seconds, uint64_t nanoseconds = 0);
			DTNTime(SDNV seconds, SDNV nanoseconds);
			virtual ~DTNTime();

			const SDNV& getTimestamp() const;
			const SDNV& getNanoseconds() const;

			/**
			 * set the DTNTime to the current time
			 */
			void set();

			void operator+=(const size_t value);

			size_t getLength() const;

		private:
			friend std::ostream &operator<<(std::ostream &stream, const dtn::data::DTNTime &obj);
			friend std::istream &operator>>(std::istream &stream, dtn::data::DTNTime &obj);

			SDNV _seconds;
			SDNV _nanoseconds;
		};
	}
}


#endif /* DTNTIME_H_ */
