/*
 * Block.cpp
 *
 *  Created on: 04.06.2009
 *      Author: morgenro
 */

#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/Bundle.h"
#include <iostream>

using namespace std;

namespace dtn
{
	namespace data
	{
		Block::Block(char blocktype)
		 : _blocktype(blocktype), _procflags(0)
		{
		}

		Block::~Block()
		{
		}

		void Block::addEID(const EID &eid)
		{
			_eids.push_back(eid);

			// add proc flag if not set
			_procflags |= Block::BLOCK_CONTAINS_EIDS;
		}

		std::list<dtn::data::EID> Block::getEIDList() const
		{
			return _eids;
		}

		void Block::set(ProcFlags flag, const bool &value)
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

		bool Block::get(ProcFlags flag) const
		{
			return (_procflags & flag);
		}

		size_t Block::getLength_strict() const
		{
			return getLength();
		}

		std::ostream& Block::serialize_strict(std::ostream &stream, size_t &length) const
		{
			return serialize(stream, length);
		}
	}
}
