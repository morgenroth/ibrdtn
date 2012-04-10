/*
 * DTNTime.h
 *
 *  Created on: 15.12.2009
 *      Author: morgenro
 */

#ifndef DTNTIME_H_
#define DTNTIME_H_

#include "ibrdtn/data/SDNV.h"

namespace dtn
{
	namespace data
	{
		class DTNTime
		{
		public:
			DTNTime();
			DTNTime(size_t seconds, size_t nanoseconds = 0);
			DTNTime(SDNV seconds, SDNV nanoseconds);
			virtual ~DTNTime();

			SDNV getTimestamp() const;

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
