/*
 * Block.cpp
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

#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/Bundle.h"
#include <iostream>

using namespace std;

namespace dtn
{
	namespace data
	{
		Block::Block(unsigned char blocktype)
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

		void Block::clearEIDs()
		{
			_eids.clear();
		}

		const Block::eid_list& Block::getEIDList() const
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

		const size_t& Block::getProcessingFlags() const
		{
			return _procflags;
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
