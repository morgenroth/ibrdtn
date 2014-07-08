/*
 * BundleBuilder.cpp
 *
 *  Created on: 10.01.2013
 *      Author: morgenro
 */

#include "ibrdtn/data/BundleBuilder.h"
#include <iterator>

namespace dtn
{
	namespace data
	{
		BundleBuilder::BundleBuilder(Bundle &target, POSITION alignment, int pos)
		: _target(&target), _alignment(alignment), _pos(pos)
		{
		}

		BundleBuilder::~BundleBuilder()
		{
		}

		dtn::data::Block &BundleBuilder::insert(dtn::data::ExtensionBlock::Factory &f, const Bitset<Block::ProcFlags> &procflags)
		{
			switch (_alignment)
			{
			case FRONT:
			{
				dtn::data::Block &block = _target->push_front(f);
				bool last_block = block.get(dtn::data::Block::LAST_BLOCK);
				block._procflags = procflags;
				block.set(dtn::data::Block::LAST_BLOCK, last_block);
				return block;
			}

			case END:
			{
				dtn::data::Block &block = _target->push_back(f);
				bool last_block = block.get(dtn::data::Block::LAST_BLOCK);
				block._procflags = procflags;
				block.set(dtn::data::Block::LAST_BLOCK, last_block);
				return block;
			}

			default:
				if(_pos <= 0) {
					dtn::data::Block &block = _target->push_front(f);
					bool last_block = block.get(dtn::data::Block::LAST_BLOCK);
					block._procflags = procflags;
					block.set(dtn::data::Block::LAST_BLOCK, last_block);
					return block;
				}

				dtn::data::Bundle::iterator it = _target->begin();
				std::advance(it, _pos-1);

				dtn::data::Block &block = (it == _target->end()) ? _target->push_back(f) : _target->insert(it, f);

				bool last_block = block.get(dtn::data::Block::LAST_BLOCK);
				block._procflags = procflags;
				block.set(dtn::data::Block::LAST_BLOCK, last_block);
				return block;
			}
		}

		BundleBuilder::POSITION BundleBuilder::getAlignment() const
		{
			return _alignment;
		}

		dtn::data::Block& BundleBuilder::insert(dtn::data::block_t block_type, const Bitset<dtn::data::Block::ProcFlags> &procflags)
		{
			// exit if the block type is zero
			if (block_type == 0) throw dtn::InvalidDataException("block type is zero");

			if (block_type == dtn::data::PayloadBlock::BLOCK_TYPE)
			{
				// use standard payload routines to add a payload block
				return insert<dtn::data::PayloadBlock>(procflags);
			}
			else
			{
				// get a extension block factory
				try {
					ExtensionBlock::Factory &f = dtn::data::ExtensionBlock::Factory::get(block_type);
					return insert(f, procflags);
				} catch (const ibrcommon::Exception &ex) {
					dtn::data::ExtensionBlock &block = insert<dtn::data::ExtensionBlock>(procflags);

					// set block type of this unknown block
					block.setType(block_type);

					if (block.get(dtn::data::Block::DISCARD_IF_NOT_PROCESSED))
					{
						// remove the block
						_target->remove(block);

						throw DiscardBlockException();
					}

					return block;
				}
			}
		}
	} /* namespace data */
} /* namespace dtn */
