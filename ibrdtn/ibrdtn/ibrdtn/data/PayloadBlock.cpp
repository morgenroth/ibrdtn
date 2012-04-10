/*
 * PayloadBlock.cpp
 *
 *  Created on: 29.05.2009
 *      Author: morgenro
 */

#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/Exceptions.h"
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace data
	{
		PayloadBlock::PayloadBlock()
		 : Block(PayloadBlock::BLOCK_TYPE), _blobref(ibrcommon::BLOB::create())
		{
		}

		PayloadBlock::PayloadBlock(ibrcommon::BLOB::Reference ref)
		 : Block(PayloadBlock::BLOCK_TYPE), _blobref(ref)
		{
		}

		PayloadBlock::~PayloadBlock()
		{
		}

		ibrcommon::BLOB::Reference PayloadBlock::getBLOB() const
		{
			return _blobref;
		}

		size_t PayloadBlock::getLength() const
		{
			ibrcommon::BLOB::Reference blobref = _blobref;
			ibrcommon::BLOB::iostream io = blobref.iostream();
			return io.size();
		}

		std::ostream& PayloadBlock::serialize(std::ostream &stream, size_t &length) const
		{
			ibrcommon::BLOB::Reference blobref = _blobref;
			ibrcommon::BLOB::iostream io = blobref.iostream();

			try {
				ibrcommon::BLOB::copy(stream, *io, io.size());
				length += io.size();
			} catch (const ibrcommon::IOException &ex) {
				throw dtn::SerializationFailedException(ex.what());
			}

			return stream;
		}

		std::ostream& PayloadBlock::serialize(std::ostream &stream, size_t clip_offset, size_t clip_length) const
		{
			ibrcommon::BLOB::Reference blobref = _blobref;
			ibrcommon::BLOB::iostream io = blobref.iostream();

			try {
				(*io).seekg(clip_offset, std::ios::beg);
				ibrcommon::BLOB::copy(stream, *io, clip_length);
			} catch (const ibrcommon::IOException &ex) {
				throw dtn::SerializationFailedException(ex.what());
			}

			return stream;
		}

		std::istream& PayloadBlock::deserialize(std::istream &stream, const size_t length)
		{
			// lock the BLOB
			ibrcommon::BLOB::iostream io = _blobref.iostream();

			// clear the blob
			io.clear();

			// check if the blob is ready
			if (!(*io).good()) throw dtn::SerializationFailedException("could not open BLOB for payload");

			// set block not processed bit to false
			set(dtn::data::Block::FORWARDED_WITHOUT_PROCESSED, false);

			try {
				ibrcommon::BLOB::copy(*io, stream, length);
			} catch (const ibrcommon::IOException &ex) {
				throw dtn::PayloadReceptionInterrupted(length, ex.what());
			}

			return stream;
		}
	}
}
