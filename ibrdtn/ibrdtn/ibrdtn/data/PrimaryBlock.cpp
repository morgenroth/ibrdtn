/*
 * PrimaryBlock.cpp
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

#include "ibrdtn/data/PrimaryBlock.h"
#include "ibrdtn/data/Exceptions.h"
#include "ibrdtn/utils/Clock.h"
#include <ibrcommon/thread/MutexLock.h>

namespace dtn
{
	namespace data
	{
		// set initial sequence number to zero
		Number PrimaryBlock::__sequencenumber = 0;

		// set initial absolute sequence number to a random value
		Number PrimaryBlock::__sequencenumber_abs = dtn::data::Number().random<uint32_t>();

		// set last assigned time-stamp to zero
		Timestamp PrimaryBlock::__last_timestamp = 0;

		// initialize lock for sequence number assignment
		ibrcommon::Mutex PrimaryBlock::__sequence_lock;

		PrimaryBlock::PrimaryBlock(bool zero_timestamp)
		 : lifetime(3600), appdatalength(0)
		{
			relabel(zero_timestamp);

			// by default set destination as singleton bit
			set(DESTINATION_IS_SINGLETON, true);
		}

		PrimaryBlock::~PrimaryBlock()
		{
		}

		void PrimaryBlock::set(FLAGS flag, bool value)
		{
			procflags.setBit(flag, value);
		}

		bool PrimaryBlock::get(FLAGS flag) const
		{
			return procflags.getBit(flag);
		}

		PrimaryBlock::PRIORITY PrimaryBlock::getPriority() const
		{
			if (get(PRIORITY_BIT1))
			{
				return PRIO_MEDIUM;
			}

			if (get(PRIORITY_BIT2))
			{
				return PRIO_HIGH;
			}

			return PRIO_LOW;
		}

		void PrimaryBlock::setPriority(PrimaryBlock::PRIORITY p)
		{
			// set the priority to the real bundle
			switch (p)
			{
			case PRIO_LOW:
				set(PRIORITY_BIT1, false);
				set(PRIORITY_BIT2, false);
				break;

			case PRIO_HIGH:
				set(PRIORITY_BIT1, false);
				set(PRIORITY_BIT2, true);
				break;

			case PRIO_MEDIUM:
				set(PRIORITY_BIT1, true);
				set(PRIORITY_BIT2, false);
				break;
			}
		}

		bool PrimaryBlock::isFragment() const
		{
			return get(FRAGMENT);
		}

		void PrimaryBlock::setFragment(bool val)
		{
			set(FRAGMENT, val);
		}

		bool PrimaryBlock::operator!=(const PrimaryBlock& other) const
		{
			return (const BundleID&)*this != (const BundleID&)other;
		}

		bool PrimaryBlock::operator==(const PrimaryBlock& other) const
		{
			return (const BundleID&)*this == (const BundleID&)other;
		}

		bool PrimaryBlock::operator<(const PrimaryBlock& other) const
		{
			return (const BundleID&)*this < other;
		}

		bool PrimaryBlock::operator>(const PrimaryBlock& other) const
		{
			return (const BundleID&)*this > other;
		}

		void PrimaryBlock::relabel(bool zero_timestamp)
		{
			if ((dtn::utils::Clock::getRating() > 0.0) && !zero_timestamp)
			{
				timestamp = dtn::utils::Clock::getTime();

				ibrcommon::MutexLock l(__sequence_lock);
				if (timestamp > __last_timestamp) {
					__last_timestamp = timestamp;
					__sequencenumber = 0;
				}

				sequencenumber = __sequencenumber;
				__sequencenumber++;
			}
			else
			{
				// set timestamp to zero
				timestamp = 0;

				// assign absolute sequence number
				ibrcommon::MutexLock l(__sequence_lock);
				sequencenumber = __sequencenumber_abs;
				__sequencenumber_abs++;

				// trim sequence-number to 32-bit for compatibility reasons
				__sequencenumber_abs.trim<uint32_t>();
			}
		}
	}
}
