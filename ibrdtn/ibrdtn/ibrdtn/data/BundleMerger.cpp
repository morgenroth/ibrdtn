/*
 * BundleMerger.cpp
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

#include "ibrdtn/config.h"
#include "ibrdtn/data/BundleMerger.h"
#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/Exceptions.h"
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace data
	{
		BundleMerger::Container::Container(ibrcommon::BLOB::Reference &ref)
		 : _bundle(), _blob(ref), _initialized(false), _hasFirstFragBlocksAdded(false), _hasLastFragBlocksAdded(false), _appdatalength(0)
		{
		}

		BundleMerger::Container::~Container()
		{

		}

		bool BundleMerger::Container::isComplete()
		{
			return Chunk::isComplete(_appdatalength, _chunks);
		}

		Bundle& BundleMerger::Container::getBundle()
		{
			return _bundle;
		}

		bool BundleMerger::Container::contains(Length offset, Length length) const
		{
			// check if the offered payload is already in the chunk list
			for (std::set<Chunk>::const_iterator iter = _chunks.begin(); iter != _chunks.end(); ++iter)
			{
				const Chunk &chunk = (*iter);

				if (offset < chunk.offset) break;

				if ( (offset >= chunk.offset) && (offset < (chunk.offset + chunk.length)) )
				{
					if ((offset + length) <= (chunk.offset + chunk.length)) return true;
				}
			}

			return false;
		}

		void BundleMerger::Container::add(Length offset, Length length)
		{
			BundleMerger::Chunk chunk(offset, length);
			_chunks.insert(chunk);
		}

		BundleMerger::Container &operator<<(BundleMerger::Container &c, const dtn::data::Bundle &obj)
		{
			// check if the given bundle is a fragment
			if (!obj.get(dtn::data::Bundle::FRAGMENT))
			{
				throw ibrcommon::Exception("This bundle is not a fragment!");
			}

			if (c._initialized)
			{
				if (	(c._bundle.timestamp != obj.timestamp) ||
						(c._bundle.sequencenumber != obj.sequencenumber) ||
						(c._bundle.source != obj.source) )
					throw ibrcommon::Exception("This fragment does not belongs to the others.");

				// set verification status to 'false' if one fragment was not verified
				if (c._bundle.get(dtn::data::PrimaryBlock::DTNSEC_STATUS_VERIFIED) && !obj.get(dtn::data::PrimaryBlock::DTNSEC_STATUS_VERIFIED)) {
					c._bundle.set(dtn::data::PrimaryBlock::DTNSEC_STATUS_VERIFIED, false);
				}
			}
			else
			{
				// copy the bundle
				c._bundle = obj;

				// store the app data length
				c._appdatalength = obj.appdatalength.get<dtn::data::Length>();

				// remove all block of the copy
				c._bundle.clear();

				// mark the copy as non-fragment
				c._bundle.set(dtn::data::Bundle::FRAGMENT, false);

				// add a new payloadblock
				c._bundle.push_back(c._blob);

				c._hasFirstFragBlocksAdded = false;
				c._hasLastFragBlocksAdded = false;

				c._initialized = true;
			}

			ibrcommon::BLOB::iostream stream = c._blob.iostream();
			(*stream).seekp(obj.fragmentoffset.get<std::streampos>());

			const dtn::data::PayloadBlock &p = obj.find<dtn::data::PayloadBlock>();
			const Length plength = p.getLength();

			// skip write operation if chunk is already in the merged bundle
			if (c.contains(obj.fragmentoffset.get<dtn::data::Length>(), plength)) return c;

			// copy payload of the fragment into the new blob
			{
				ibrcommon::BLOB::Reference ref = p.getBLOB();
				ibrcommon::BLOB::iostream s = ref.iostream();
				(*stream) << (*s).rdbuf() << std::flush;
			}

			// add the chunk to the list of chunks
			c.add(obj.fragmentoffset.get<dtn::data::Length>(), plength);

			// check if fragment is the first one
			// add blocks only once
			if (!c._hasFirstFragBlocksAdded && obj.fragmentoffset == 0)
			{
				c._hasFirstFragBlocksAdded = true;

				dtn::data::Bundle::const_iterator payload_it = obj.find(dtn::data::PayloadBlock::BLOCK_TYPE);

				// abort if the bundle do not contains a payload block
				if (payload_it == obj.end()) throw ibrcommon::Exception("Payload block missing.");

				// iterate from begin to the payload block
				for (dtn::data::Bundle::const_iterator block_it = obj.begin(); block_it != payload_it; ++block_it)
				{
					// get the current block and type
					const Block &current_block = (**block_it);
					block_t block_type = current_block.getType();

					// search the position of the payload block
					dtn::data::Bundle::iterator p = c._bundle.find(dtn::data::PayloadBlock::BLOCK_TYPE);
					if (p == c._bundle.end()) throw ibrcommon::Exception("Payload block missing.");

					try
					{
						ExtensionBlock::Factory &f = dtn::data::ExtensionBlock::Factory::get(block_type);

						// insert new Block before payload block and copy block
						dtn::data::Block &block = c._bundle.insert(p, f);
						block = current_block;

						IBRCOMMON_LOGGER_DEBUG_TAG("BundleMerger", 5) << "Reassemble: Added Block before Payload: " << obj.toString() << "  " << block_type << IBRCOMMON_LOGGER_ENDL;
					}
					catch(const ibrcommon::Exception &ex)
					{
						// insert new Block before payload block and copy block
						dtn::data::Block &block = c._bundle.insert<dtn::data::ExtensionBlock>(p);
						block = current_block;

						IBRCOMMON_LOGGER_DEBUG_TAG("BundleMerger", 5) << "Reassemble: Added Block before Payload: " << obj.toString() << "  " << block_type << IBRCOMMON_LOGGER_ENDL;
					}
				}

			}

			//check if fragment is the last one
			//add blocks only once
			if(!c._hasLastFragBlocksAdded && obj.fragmentoffset + plength == obj.appdatalength)
			{
				c._hasLastFragBlocksAdded = true;

				// start to iterate after the payload block
				dtn::data::Bundle::const_iterator payload_it = obj.find(dtn::data::PayloadBlock::BLOCK_TYPE);

				// abort if the bundle do not contains a payload block
				if (payload_it == obj.end()) throw ibrcommon::Exception("Payload block missing.");

				// start with the block after the payload block
				for (payload_it++; payload_it != obj.end(); ++payload_it)
				{
					//get the current block and type
					const Block &current_block = (**payload_it);
					block_t block_type = current_block.getType();

					try
					{
						ExtensionBlock::Factory &f = dtn::data::ExtensionBlock::Factory::get(block_type);

						//push back new Block after payload block and copy block
						dtn::data::Block &block = c._bundle.push_back(f);
						block = current_block;

						IBRCOMMON_LOGGER_DEBUG_TAG("BundleMerger", 5) << "Reassemble: Added Block after Payload: " << obj.toString()<< "  " << block_type << IBRCOMMON_LOGGER_ENDL;
					}
					catch(const ibrcommon::Exception &ex)
					{
						//push back new Block after payload block and copy block
						dtn::data::Block &block = c._bundle.push_back<dtn::data::ExtensionBlock>();
						block = current_block;

						IBRCOMMON_LOGGER_DEBUG_TAG("BundleMerger", 5) << "Reassemble: Added Block after Payload: " << obj.toString()<< "  " << block_type << IBRCOMMON_LOGGER_ENDL;
					}
				}
			}

			return c;
		}

		BundleMerger::Container BundleMerger::getContainer()
		{
			ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
			return Container(ref);
		}

		BundleMerger::Container BundleMerger::getContainer(ibrcommon::BLOB::Reference &ref)
		{
			return Container(ref);
		}

		BundleMerger::Chunk::Chunk(Length o, Length l)
		 : offset(o), length(l)
		{
		}

		BundleMerger::Chunk::~Chunk()
		{
		}

		bool BundleMerger::Chunk::operator<(const Chunk &other) const
		{
			if (offset < other.offset) return true;
			if (offset != other.offset) return false;

			return (length < other.length);
		}

		bool BundleMerger::Chunk::isComplete(Length length, const std::set<Chunk> &chunks)
		{
			// check if the bundle payload is complete
			Length position = 0;

			for (std::set<Chunk>::const_iterator iter = chunks.begin(); iter != chunks.end(); ++iter)
			{
				const Chunk &chunk = (*iter);

				// if the next offset is too small, we do not got all fragments
				if (chunk.offset > position) return false;

				position = chunk.offset + chunk.length;
			}

			// return true, if we reached the application data length
			return (position >= length);
		}
	}
}
