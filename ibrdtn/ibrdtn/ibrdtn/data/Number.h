/*
 * Number.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#ifndef NUMBER_H_
#define NUMBER_H_

#include "ibrdtn/data/SDNV.h"
#include <stdint.h>
#include <sys/types.h>

namespace dtn
{
	namespace data
	{
		typedef size_t Length;
		typedef size_t Size;
		typedef size_t Timeout;

		typedef unsigned char block_t;
		typedef dtn::data::SDNV<Size> Number;
		typedef dtn::data::SDNV<float> Float;
		typedef dtn::data::SDNV<int> Integer;
		typedef dtn::data::SDNV<Size> Timestamp;

		template<typename E>
		class Bitset : public dtn::data::SDNV<Size> {
		public:
			Bitset(Size initial) : dtn::data::SDNV<Size>(initial) {
			}

			Bitset() {
			}

			~Bitset() { }

			void setBit(E flag, const bool &value)
			{
				if (value)
				{
					(dtn::data::SDNV<Size>&)(*this) |= flag;
				}
				else
				{
					(dtn::data::SDNV<Size>&)(*this) &= ~(flag);
				}
			}

			bool getBit(E flag) const
			{
				return ((const dtn::data::SDNV<Size>&)(*this) & flag);
			}
		};
	}
}


#endif /* NUMBER_H_ */
