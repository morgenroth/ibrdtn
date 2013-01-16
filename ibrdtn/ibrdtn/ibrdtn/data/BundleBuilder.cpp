/*
 * BundleBuilder.cpp
 *
 *  Created on: 10.01.2013
 *      Author: morgenro
 */

#include "ibrdtn/data/BundleBuilder.h"

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

		dtn::data::Block &BundleBuilder::insert(dtn::data::ExtensionBlock::Factory &f, size_t procflags)
		{
			switch (_alignment)
			{
			case FRONT:
			{
				dtn::data::Block &block = _target->push_front(f);
				block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
				return block;
			}

			case END:
			{
				dtn::data::Block &block = _target->push_back(f);
				block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
				return block;
			}

			default:
				if(_pos <= 0) {
					dtn::data::Block &block = _target->push_front(f);
					block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
					return block;
				}

				try {
					dtn::data::Block &prev_block = _target->getBlock(_pos-1);

					dtn::data::Block &block = _target->insert(f, prev_block);
					block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
					return block;
				} catch (const std::exception &ex) {
					dtn::data::Block &block = _target->push_back(f);
					block._procflags = procflags & (~(dtn::data::Block::LAST_BLOCK) | block._procflags);
					return block;
				}
				break;
			}
		}

		BundleBuilder::POSITION BundleBuilder::getAlignment() const
		{
			return _alignment;
		}

		dtn::data::Block& BundleBuilder::insert(char block_type, size_t procflags)
		{
			switch (block_type)
			{
				case 0:
				{
					throw dtn::InvalidDataException("block type is zero");
					break;
				}

				case dtn::data::PayloadBlock::BLOCK_TYPE:
				{
					return insert<dtn::data::PayloadBlock>(procflags);
					break;
				}

				default:
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
						}

						return block;
					}
					break;
				}
			}
		}
	} /* namespace data */
} /* namespace dtn */
