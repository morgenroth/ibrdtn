/*
 * ScopeControlHopLimitBlock.cpp
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

#include "ibrdtn/data/ScopeControlHopLimitBlock.h"

namespace dtn
{
	namespace data
	{
		const dtn::data::block_t ScopeControlHopLimitBlock::BLOCK_TYPE = 199;

		dtn::data::Block* ScopeControlHopLimitBlock::Factory::create()
		{
			return new ScopeControlHopLimitBlock();
		}

		ScopeControlHopLimitBlock::ScopeControlHopLimitBlock()
		 : dtn::data::Block(ScopeControlHopLimitBlock::BLOCK_TYPE)
		{
			// set the replicate in every fragment bit
			set(REPLICATE_IN_EVERY_FRAGMENT, true);
		}

		ScopeControlHopLimitBlock::~ScopeControlHopLimitBlock()
		{
		}

		Number ScopeControlHopLimitBlock::getHopsToLive() const
		{
			if (_limit < _count) return 0;
			return _limit - _count;
		}

		void ScopeControlHopLimitBlock::increment(const Number &hops)
		{
			_count += hops;
		}

		void ScopeControlHopLimitBlock::setLimit(const Number &hops)
		{
			_count = 0;
			_limit = hops;
		}

		Length ScopeControlHopLimitBlock::getLength() const
		{
			return _count.getLength() + _limit.getLength();
		}

		std::ostream& ScopeControlHopLimitBlock::serialize(std::ostream &stream, Length &length) const
		{
			stream << _count;
			stream << _limit;
			length += getLength();
			return stream;
		}

		std::istream& ScopeControlHopLimitBlock::deserialize(std::istream &stream, const Length&)
		{
			stream >> _count;
			stream >> _limit;
			return stream;
		}
	} /* namespace data */
} /* namespace dtn */
