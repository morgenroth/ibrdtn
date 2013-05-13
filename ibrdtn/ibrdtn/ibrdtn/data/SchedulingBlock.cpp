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
		const dtn::data::block_t SchedulingBlock::BLOCK_TYPE = 198;

		dtn::data::Block* SchedulingBlock::Factory::create()
		{
			return new SchedulingBlock();
		}

		SchedulingBlock::SchedulingBlock()
		 : dtn::data::Block(SchedulingBlock::BLOCK_TYPE), _priority(0)
		{
			// set the replicate in every fragment bit
			set(REPLICATE_IN_EVERY_FRAGMENT, true);
		}

		SchedulingBlock::~SchedulingBlock()
		{
		}

		void SchedulingBlock::setPriority(Integer p)
		{
			_flags.setBit(HAS_PRIORITY, true);
			_priority = p;
		}

		Integer SchedulingBlock::getPriority() const
		{
			return _priority;
		}

		Length SchedulingBlock::getLength() const
		{
			Length len = _flags.getLength();

			if (_flags.getBit(HAS_PRIORITY)) _priority.getLength();

			return len;
		}

		std::ostream& SchedulingBlock::serialize(std::ostream &stream, Length &length) const
		{
			stream << _flags;

			if (_flags.getBit(HAS_PRIORITY)) {
				stream << _priority;
			}

			length += getLength();
			return stream;
		}

		std::istream& SchedulingBlock::deserialize(std::istream &stream, const Length&)
		{
			stream >> _flags;

			if (_flags.getBit(HAS_PRIORITY)) {
				stream >> _priority;
			}

			return stream;
		}
	} /* namespace data */
} /* namespace dtn */
