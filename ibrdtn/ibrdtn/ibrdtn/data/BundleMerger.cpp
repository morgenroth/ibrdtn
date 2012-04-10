/*
 * BundleMerger.cpp
 *
 *  Created on: 25.01.2010
 *      Author: morgenro
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
		 : _bundle(), _blob(ref), _initialized(false), _appdatalength(0)
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

		bool BundleMerger::Container::contains(size_t offset, size_t length) const
		{
			// check if the offered payload is already in the chunk list
			for (std::set<Chunk>::const_iterator iter = _chunks.begin(); iter != _chunks.end(); iter++)
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

		void BundleMerger::Container::add(size_t offset, size_t length)
		{
			BundleMerger::Chunk chunk(offset, length);
			_chunks.insert(chunk);
		}

		BundleMerger::Container &operator<<(BundleMerger::Container &c, dtn::data::Bundle &obj)
		{
			// check if the given bundle is a fragment
			if (!obj.get(dtn::data::Bundle::FRAGMENT))
			{
				throw ibrcommon::Exception("This bundle is not a fragment!");
			}

			if (c._initialized)
			{
				if (	(c._bundle._timestamp != obj._timestamp) ||
						(c._bundle._sequencenumber != obj._sequencenumber) ||
						(c._bundle._source != obj._source) )
					throw ibrcommon::Exception("This fragment does not belongs to the others.");
			}
			else
			{
				// copy the bundle
				c._bundle = obj;

				// store the app data length
				c._appdatalength = obj._appdatalength;

				// remove all block of the copy
				c._bundle.clearBlocks();

				// mark the copy as non-fragment
				c._bundle.set(dtn::data::Bundle::FRAGMENT, false);

				// add a new payloadblock
				c._bundle.push_back(c._blob);

				c._initialized = true;
			}

			ibrcommon::BLOB::iostream stream = c._blob.iostream();
			(*stream).seekp(obj._fragmentoffset);

			dtn::data::PayloadBlock &p = obj.getBlock<dtn::data::PayloadBlock>();
			const size_t plength = p.getLength();

			// skip write operation if chunk is already in the merged bundle
			if (c.contains(obj._fragmentoffset, plength)) return c;

			ibrcommon::BLOB::Reference ref = p.getBLOB();
			ibrcommon::BLOB::iostream s = ref.iostream();

			// copy payload of the fragment into the new blob
			(*stream) << (*s).rdbuf() << std::flush;

			// add the chunk to the list of chunks
			c.add(obj._fragmentoffset, plength);

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

		BundleMerger::Chunk::Chunk(size_t o, size_t l)
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

		bool BundleMerger::Chunk::isComplete(size_t length, const std::set<Chunk> &chunks)
		{
			// check if the bundle payload is complete
			size_t position = 0;

			for (std::set<Chunk>::const_iterator iter = chunks.begin(); iter != chunks.end(); iter++)
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
