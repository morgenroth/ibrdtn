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
		size_t PrimaryBlock::__sequencenumber = 0;
		size_t PrimaryBlock::__last_timestamp = 0;
		ibrcommon::Mutex PrimaryBlock::__sequence_lock;

		PrimaryBlock::PrimaryBlock()
		 : _procflags(0), _timestamp(0), _sequencenumber(0), _lifetime(3600), _fragmentoffset(0), _appdatalength(0)
		{
			relabel();

			// by default set destination as singleton bit
			set(DESTINATION_IS_SINGLETON, true);
		}

		PrimaryBlock::~PrimaryBlock()
		{
		}

		void PrimaryBlock::set(FLAGS flag, bool value)
		{
			if (value)
			{
				_procflags |= flag;
			}
			else
			{
				_procflags &= ~(flag);
			}
		}

		bool PrimaryBlock::get(FLAGS flag) const
		{
			return (_procflags & flag);
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

		bool PrimaryBlock::operator!=(const PrimaryBlock& other) const
		{
			return !((*this) == other);
		}

		bool PrimaryBlock::operator==(const PrimaryBlock& other) const
		{
			if (other._timestamp != _timestamp) return false;
			if (other._sequencenumber != _sequencenumber) return false;
			if (other._source != _source) return false;
			if (other.get(PrimaryBlock::FRAGMENT) != get(PrimaryBlock::FRAGMENT)) return false;

			if (get(PrimaryBlock::FRAGMENT))
			{
				if (other._fragmentoffset != _fragmentoffset) return false;
				if (other._appdatalength != _appdatalength) return false;
			}

			return true;
		}

		bool PrimaryBlock::operator<(const PrimaryBlock& other) const
		{
			if (_source < other._source) return true;
			if (_source != other._source) return false;

			if (_timestamp < other._timestamp) return true;
			if (_timestamp != other._timestamp) return false;

			if (_sequencenumber < other._sequencenumber) return true;
			if (_sequencenumber != other._sequencenumber) return false;

			if (other.get(PrimaryBlock::FRAGMENT))
			{
				if (!get(PrimaryBlock::FRAGMENT)) return true;
				return (_fragmentoffset < other._fragmentoffset);
			}

			return false;
		}

		bool PrimaryBlock::operator>(const PrimaryBlock& other) const
		{
			return !(((*this) < other) || ((*this) == other));
		}

		bool PrimaryBlock::isExpired() const
		{
			return dtn::utils::Clock::isExpired(_lifetime + _timestamp, _lifetime);
		}

		std::string PrimaryBlock::toString() const
		{
			return dtn::data::BundleID(*this).toString();
		}

		void PrimaryBlock::relabel()
		{
			if (dtn::utils::Clock::isBad())
			{
				_timestamp = 0;
			}
			else
			{
				_timestamp = dtn::utils::Clock::getTime();
			}

			ibrcommon::MutexLock l(__sequence_lock);
			if (_timestamp > __last_timestamp) {
				__last_timestamp = _timestamp;
				__sequencenumber = 0;
			}

			_sequencenumber = __sequencenumber;
			__sequencenumber++;
		}
	}
}
