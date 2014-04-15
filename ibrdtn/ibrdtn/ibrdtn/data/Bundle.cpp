/*
 * Bundle.cpp
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

#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/Serializer.h"
#include "ibrdtn/data/AgeBlock.h"
#include "ibrdtn/data/MetaBundle.h"
#include "ibrdtn/data/BundleID.h"
#include <algorithm>

namespace dtn
{
	namespace data
	{
		Bundle::Bundle(bool zero_timestamp)
		 : PrimaryBlock(zero_timestamp)
		{
			// if the timestamp is not set, add a ageblock
			if (timestamp == 0)
			{
				// add a new ageblock
				push_front<dtn::data::AgeBlock>();
			}
		}

		Bundle::~Bundle()
		{
			clear();
		}

		Bundle::iterator Bundle::begin()
		{
			return _blocks.begin();
		}

		Bundle::iterator Bundle::end()
		{
			return _blocks.end();
		}

		Bundle::const_iterator Bundle::begin() const
		{
			return _blocks.begin();
		}

		Bundle::const_iterator Bundle::end() const
		{
			return _blocks.end();
		}

		bool Bundle::operator==(const BundleID& other) const
		{
			return other == (const PrimaryBlock&)(*this);
		}

		bool Bundle::operator==(const MetaBundle& other) const
		{
			return other == (const PrimaryBlock&)(*this);
		}

		bool Bundle::operator!=(const Bundle& other) const
		{
			return (const PrimaryBlock&)(*this) != (const PrimaryBlock&)other;
		}

		bool Bundle::operator==(const Bundle& other) const
		{
			return (const PrimaryBlock&)(*this) == (const PrimaryBlock&)other;
		}

		bool Bundle::operator<(const Bundle& other) const
		{
			return (const PrimaryBlock&)(*this) < (const PrimaryBlock&)other;
		}

		bool Bundle::operator>(const Bundle& other) const
		{
			return (const PrimaryBlock&)(*this) > (const PrimaryBlock&)other;
		}

		void Bundle::remove(const dtn::data::Block &block)
		{
			for (iterator it = begin(); it != end(); ++it)
			{
				const refcnt_ptr<Block> &b = (*it);
				if (b == &block)
				{
					erase(it);
					return;
				}
			}
		}

		void Bundle::erase(Bundle::iterator b, Bundle::iterator e)
		{
			for (iterator it = b; it != e;)
			{
				_blocks.erase(it++);
			}

			if (size() > 0) {
				// set the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, true);
			}
		}

		void Bundle::erase(iterator it)
		{
			_blocks.erase(it);

			if (size() > 0) {
				// set the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, true);
			}
		}

		void Bundle::clear()
		{
			_blocks.clear();
		}

		dtn::data::PayloadBlock& Bundle::insert(iterator before, ibrcommon::BLOB::Reference &ref)
		{
			if (size() > 0) {
				// remove the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, false);
			}

			dtn::data::PayloadBlock *tmpblock = new dtn::data::PayloadBlock(ref);
			block_elem block( static_cast<dtn::data::Block*>(tmpblock) );

			_blocks.insert(before, block);

			// set the last block bit
			iterator last = end();
			--last;
			(**last).set(dtn::data::Block::LAST_BLOCK, true);

			return (*tmpblock);
		}

		dtn::data::PayloadBlock& Bundle::push_front(ibrcommon::BLOB::Reference &ref)
		{
			dtn::data::PayloadBlock *tmpblock = new dtn::data::PayloadBlock(ref);
			block_elem block( static_cast<dtn::data::Block*>(tmpblock) );
			_blocks.push_front(block);

			// if this was the first element
			if (size() == 1)
			{
				// set the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, true);
			}

			return (*tmpblock);
		}

		dtn::data::PayloadBlock& Bundle::push_back(ibrcommon::BLOB::Reference &ref)
		{
			if (size() > 0) {
				// remove the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, false);
			}

			dtn::data::PayloadBlock *tmpblock = new dtn::data::PayloadBlock(ref);
			block_elem block( static_cast<dtn::data::Block*>(tmpblock) );
			_blocks.push_back(block);

			// set the last block bit
			block->set(dtn::data::Block::LAST_BLOCK, true);

			return (*tmpblock);
		}

		dtn::data::Block& Bundle::push_front(dtn::data::ExtensionBlock::Factory &factory)
		{
			block_elem block( factory.create() );
			_blocks.push_front(block);

			// if this was the first element
			if (size() == 1)
			{
				// set the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, true);
			}

			return (*block);
		}

		dtn::data::Block& Bundle::push_back(dtn::data::ExtensionBlock::Factory &factory)
		{
			if (size() > 0) {
				// remove the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, false);
			}

			block_elem block( factory.create() );
			_blocks.push_back(block);

			// set the last block bit
			block->set(dtn::data::Block::LAST_BLOCK, true);

			return (*block);
		}
		
		Block& Bundle::insert(iterator before, dtn::data::ExtensionBlock::Factory& factory)
		{
			if (size() > 0) {
				// remove the last block bit
				iterator last = end();
				--last;
				(**last).set(dtn::data::Block::LAST_BLOCK, false);
			}

			block_elem block( factory.create() );
			_blocks.insert(before, block);

			// set the last block bit
			iterator last = end();
			--last;
			(**last).set(dtn::data::Block::LAST_BLOCK, true);

			return (*block);
		}

		Size Bundle::size() const
		{
			return _blocks.size();
		}

		bool Bundle::allEIDsInCBHE() const
		{
			if( destination.isCompressable()
			    && source.isCompressable()
			    && reportto.isCompressable()
			    && custodian.isCompressable()
			  )
			{
				for( const_iterator it = begin(); it != end(); ++it ) {
					if( (**it).get( Block::BLOCK_CONTAINS_EIDS ) )
					{
						std::list< EID > blockEIDs = (**it).getEIDList();
						for( std::list< EID >::const_iterator itBlockEID = blockEIDs.begin(); itBlockEID != blockEIDs.end(); ++itBlockEID )
						{
							if( ! itBlockEID->isCompressable() )
							{
								return false;
							}
						}
					}
				}
				return true;
			}
			else
			{
				return false;
			}
		}

		dtn::data::Length Bundle::getPayloadLength() const
		{
			try {
				const dtn::data::PayloadBlock &payload = find<dtn::data::PayloadBlock>();
				return payload.getLength();
			} catch (const NoSuchBlockFoundException&) {
				return 0;
			}
		}

		Bundle::const_iterator Bundle::find(block_t blocktype) const
		{
			return std::find(begin(), end(), blocktype);
		}

		Bundle::iterator Bundle::find(block_t blocktype)
		{
			return std::find(begin(), end(), blocktype);
		}

		Bundle::iterator Bundle::find(const dtn::data::Block &block)
		{
			for (iterator it = begin(); it != end(); ++it)
			{
				if ((&**it) == &block) return it;
			}

			return end();
		}

		Bundle::const_iterator Bundle::find(const dtn::data::Block &block) const
		{
			for (const_iterator it = begin(); it != end(); ++it)
			{
				if ((&**it) == &block) return it;
			}

			return end();
		}

	}
}
