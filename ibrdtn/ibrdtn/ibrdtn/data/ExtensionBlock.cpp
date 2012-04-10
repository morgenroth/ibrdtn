/*
 * ExtensionBlock.cpp
 *
 *  Created on: 26.05.2010
 *      Author: morgenro
 */

#include "ibrdtn/data/ExtensionBlock.h"
#include "ibrdtn/data/Exceptions.h"
#include "ibrdtn/data/ExtensionBlock.h"
#include <ibrcommon/thread/MutexLock.h>

namespace dtn
{
	namespace data
	{
		ExtensionBlock::FactoryList *ExtensionBlock::factories = NULL;

		ExtensionBlock::FactoryList::FactoryList()
		{ }

		ExtensionBlock::FactoryList::~FactoryList()
		{ }

		void ExtensionBlock::FactoryList::initialize()
		{
			static ibrcommon::Mutex mutex;
			ibrcommon::MutexLock l(mutex);

			if (ExtensionBlock::factories == NULL)
			{
				ExtensionBlock::factories = new ExtensionBlock::FactoryList();
			}
		}

		ExtensionBlock::Factory& ExtensionBlock::FactoryList::get(const char type) throw (ibrcommon::Exception)
		{
			std::map<char, ExtensionBlock::Factory*>::iterator iter = fmap.find(type);

			if (iter != fmap.end())
			{
				return *(iter->second);
			}

			throw ibrcommon::Exception("Factory not available");
		}

		void ExtensionBlock::FactoryList::add(const char type, Factory *f) throw (ibrcommon::Exception)
		{
			try {
				get(type);
				throw ibrcommon::Exception("extension block type already taken");
			} catch (const ibrcommon::Exception&) {
				fmap[type] = f;
			}
		}

		void ExtensionBlock::FactoryList::remove(const char type)
		{
			fmap.erase(type);
		}

		ExtensionBlock::Factory::Factory(char type)
		 : _type(type)
		{
			ExtensionBlock::FactoryList::initialize();
			ExtensionBlock::factories->add(type, this);
		}

		ExtensionBlock::Factory::~Factory()
		{
			ExtensionBlock::factories->remove(_type);
		}

		ExtensionBlock::Factory& ExtensionBlock::Factory::get(char type) throw (ibrcommon::Exception)
		{
			ExtensionBlock::FactoryList::initialize();
			return ExtensionBlock::factories->get(type);
		}

		ExtensionBlock::ExtensionBlock()
		 : Block(0), _blobref(ibrcommon::BLOB::create())
		{
		}

		ExtensionBlock::ExtensionBlock(ibrcommon::BLOB::Reference ref)
		 : Block(0), _blobref(ref)
		{
		}

		ExtensionBlock::~ExtensionBlock()
		{
		}

		ibrcommon::BLOB::Reference ExtensionBlock::getBLOB() const
		{
			return _blobref;
		}

		size_t ExtensionBlock::getLength() const
		{
			ibrcommon::BLOB::Reference blobref = _blobref;
			ibrcommon::BLOB::iostream io = blobref.iostream();
			return io.size();
		}

		std::ostream& ExtensionBlock::serialize(std::ostream &stream, size_t &length) const
		{
			ibrcommon::BLOB::Reference blobref = _blobref;
			ibrcommon::BLOB::iostream io = blobref.iostream();

			try {
				ibrcommon::BLOB::copy(stream, *io, io.size());
			} catch (const ibrcommon::IOException &ex) {
				throw dtn::SerializationFailedException(ex.what());
			}

			return stream;
		}

		std::istream& ExtensionBlock::deserialize(std::istream &stream, const size_t length)
		{
			// lock the BLOB
			ibrcommon::BLOB::iostream io = _blobref.iostream();

			// clear the blob
			io.clear();

			// check if the blob is ready
			if (!(*io).good()) throw dtn::SerializationFailedException("could not open BLOB for payload");

			try {
				ibrcommon::BLOB::copy(*io, stream, length);
			} catch (const ibrcommon::IOException &ex) {
				throw dtn::SerializationFailedException(ex.what());
			}

			// set block not processed bit
			set(dtn::data::Block::FORWARDED_WITHOUT_PROCESSED, true);

			return stream;
		}
	}
}
