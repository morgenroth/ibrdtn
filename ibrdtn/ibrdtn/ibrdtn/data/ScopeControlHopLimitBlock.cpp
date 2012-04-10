/*
 * ScopeControlHopLimitBlock.cpp
 *
 *  Created on: 05.08.2011
 *      Author: morgenro
 */

#include "ibrdtn/data/ScopeControlHopLimitBlock.h"

namespace dtn
{
	namespace data
	{
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

		size_t ScopeControlHopLimitBlock::getHopsToLive() const
		{
			if (_limit.getValue() < _count.getValue()) return 0;
			return _limit.getValue() - _count.getValue();
		}

		void ScopeControlHopLimitBlock::increment(size_t hops)
		{
			_count += dtn::data::SDNV(hops);
		}

		void ScopeControlHopLimitBlock::setLimit(size_t hops)
		{
			_count = 0;
			_limit = hops;
		}

		size_t ScopeControlHopLimitBlock::getLength() const
		{
			return _count.getLength() + _limit.getLength();
		}

		std::ostream& ScopeControlHopLimitBlock::serialize(std::ostream &stream, size_t &length) const
		{
			stream << _count;
			stream << _limit;
			length += getLength();
			return stream;
		}

		std::istream& ScopeControlHopLimitBlock::deserialize(std::istream &stream, const size_t)
		{
			stream >> _count;
			stream >> _limit;
			return stream;
		}
	} /* namespace data */
} /* namespace dtn */
