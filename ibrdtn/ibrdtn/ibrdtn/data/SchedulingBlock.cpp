/*
 * SchedulingBlock.cpp
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

#include "ibrdtn/data/SchedulingBlock.h"

namespace dtn
{
	namespace data
	{
		dtn::data::Block* SchedulingBlock::Factory::create()
		{
			return new SchedulingBlock();
		}

		SchedulingBlock::SchedulingBlock()
		 : dtn::data::Block(SchedulingBlock::BLOCK_TYPE), _flags(0), _priority(0)
		{
			// set the replicate in every fragment bit
			set(REPLICATE_IN_EVERY_FRAGMENT, true);
		}

		SchedulingBlock::~SchedulingBlock()
		{
		}

		void SchedulingBlock::setPriority(int8_t p)
		{
			_flags |= HAS_PRIORITY;
			_priority = p;
		}

		int8_t SchedulingBlock::getPriority() const
		{
			return _priority;
		}

		size_t SchedulingBlock::getLength() const
		{
			size_t len = dtn::data::SDNV(_flags).getLength();

			if (_flags & HAS_PRIORITY) dtn::data::SDNV(_priority).getLength();

			return len;
		}

		std::ostream& SchedulingBlock::serialize(std::ostream &stream, size_t &length) const
		{
			stream << dtn::data::SDNV(_flags);

			if (_flags & HAS_PRIORITY) {
				stream << dtn::data::SDNV(_priority);
			}

			length += getLength();
			return stream;
		}

		std::istream& SchedulingBlock::deserialize(std::istream &stream, const size_t)
		{
			dtn::data::SDNV flags;
			stream >> flags;
			_flags = flags.getValue();

			if (_flags & HAS_PRIORITY) {
				dtn::data::SDNV priority;
				stream >> priority;
				_priority = priority.getValue();
			}

			return stream;
		}
	} /* namespace data */
} /* namespace dtn */
