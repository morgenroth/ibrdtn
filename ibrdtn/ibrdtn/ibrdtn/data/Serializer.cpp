/*
 * Serializer.cpp
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

#include "ibrdtn/data/Serializer.h"
#include "ibrdtn/data/BundleBuilder.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/BundleString.h"
#include "ibrdtn/data/ExtensionBlock.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/MetaBundle.h"
#include "ibrdtn/utils/Clock.h"
#include <ibrcommon/refcnt_ptr.h>
#include <ibrcommon/Logger.h>
#include <list>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

namespace dtn
{
	namespace data
	{
		DefaultSerializer::DefaultSerializer(std::ostream& stream)
		 : _stream(stream), _compressable(false)
		{
		}

		DefaultSerializer::DefaultSerializer(std::ostream& stream, const Dictionary &d)
		 : _stream(stream), _dictionary(d), _compressable(false)
		{
		}

		void DefaultSerializer::rebuildDictionary(const dtn::data::Bundle &obj)
		{
			// clear the dictionary
			_dictionary.clear();

			// rebuild the dictionary
			_dictionary.add(obj._destination);
			_dictionary.add(obj.source);
			_dictionary.add(obj._reportto);
			_dictionary.add(obj._custodian);

			// add EID of all secondary blocks
			for (Bundle::const_iterator iter = obj.begin(); iter != obj.end(); ++iter)
			{
				const Block &b = (**iter);
				_dictionary.add( b.getEIDList() );
			}

			// check if the bundle header could be compressed
			_compressable = isCompressable(obj);
		}

		Serializer& DefaultSerializer::operator <<(const dtn::data::Bundle& obj)
		{
			// rebuild the dictionary
			rebuildDictionary(obj);

			// serialize the primary block
			(*this) << (PrimaryBlock&)obj;

			// serialize all secondary blocks
			for (Bundle::const_iterator iter = obj.begin(); iter != obj.end(); ++iter)
			{
				const Block &b = (**iter);
				(*this) << b;
			}

			return (*this);
		}

		Serializer& DefaultSerializer::operator<<(const dtn::data::BundleFragment &obj)
		{
			// rebuild the dictionary
			rebuildDictionary(obj._bundle);

			PrimaryBlock prim = obj._bundle;
			prim.set(dtn::data::PrimaryBlock::FRAGMENT, true);

			// set the application length according to the payload block size
			dtn::data::Bundle::const_iterator it = obj._bundle.find(dtn::data::PayloadBlock::BLOCK_TYPE);

			if (it != obj._bundle.end()) {
				const dtn::data::PayloadBlock &payload = dynamic_cast<const dtn::data::PayloadBlock&>(**it);
				prim.appdatalength = payload.getLength();
			} else {
				prim.appdatalength = 0;
			}

			// set the fragmentation offset
			prim.fragmentoffset += obj._offset;

			// serialize the primary block
			(*this) << prim;

			// serialize all secondary blocks
			bool post_payload = false;

			for (Bundle::const_iterator iter = obj._bundle.begin(); iter != obj._bundle.end(); ++iter)
			{
				const Block &b = (**iter);

				try {
					// test if this is the payload block
					const dtn::data::PayloadBlock &payload = dynamic_cast<const dtn::data::PayloadBlock&>(b);

					// serialize the clipped block
					serialize(payload, obj._offset, obj._length);

					// we had serialized the payload block
					post_payload = true;
				} catch (const std::bad_cast&) {
					// serialize this block if
					// ... this block is before the payload block and marked as replicated in every fragment
					// ... this block is after the payload block and all remaining bytes of the payload block are included
					if (post_payload || b.get(dtn::data::Block::REPLICATE_IN_EVERY_FRAGMENT))
					{
						(*this) << b;
					}
				}
			}

			return (*this);
		}

		bool DefaultSerializer::isCompressable(const dtn::data::Bundle &obj) const
		{
			// check if all EID are compressable
			bool compressable = ( obj.source.isCompressable() &&
					obj._destination.isCompressable() &&
					obj._reportto.isCompressable() &&
					obj._custodian.isCompressable() );

			if (compressable)
			{
				// add EID of all secondary blocks
				for (Bundle::const_iterator iter = obj.begin(); iter != obj.end(); ++iter)
				{
					const Block &b = (**iter);
					const std::list<dtn::data::EID> eids = b.getEIDList();

					for (std::list<dtn::data::EID>::const_iterator eit = eids.begin(); eit != eids.end(); ++eit)
					{
						const dtn::data::EID &eid = (*eit);
						if (!eid.isCompressable())
						{
							return false;
						}
					}
				}
			}

			return compressable;
		}

		Serializer& DefaultSerializer::operator <<(const dtn::data::PrimaryBlock& obj)
		{
			_stream << dtn::data::BUNDLE_VERSION;		// bundle version
			_stream << dtn::data::SDNV(obj.procflags);	// processing flags

			// predict the block length
			size_t len = 0;
			dtn::data::SDNV primaryheader[14];

			primaryheader[8] = SDNV(obj.timestamp);		// timestamp
			primaryheader[9] = SDNV(obj.sequencenumber);	// sequence number
			primaryheader[10] = SDNV(obj.lifetime);		// lifetime

			pair<size_t, size_t> ref;

			if (_compressable)
			{
				// destination reference
				ref = obj._destination.getCompressed();
				primaryheader[0] = SDNV(ref.first);
				primaryheader[1] = SDNV(ref.second);

				// source reference
				ref = obj.source.getCompressed();
				primaryheader[2] = SDNV(ref.first);
				primaryheader[3] = SDNV(ref.second);

				// reportto reference
				ref = obj._reportto.getCompressed();
				primaryheader[4] = SDNV(ref.first);
				primaryheader[5] = SDNV(ref.second);

				// custodian reference
				ref = obj._custodian.getCompressed();
				primaryheader[6] = SDNV(ref.first);
				primaryheader[7] = SDNV(ref.second);

				// dictionary size is zero in a compressed bundle header
				primaryheader[11] = SDNV(0);
			}
			else
			{
				// destination reference
				ref = _dictionary.getRef(obj._destination);
				primaryheader[0] = SDNV(ref.first);
				primaryheader[1] = SDNV(ref.second);

				// source reference
				ref = _dictionary.getRef(obj.source);
				primaryheader[2] = SDNV(ref.first);
				primaryheader[3] = SDNV(ref.second);

				// reportto reference
				ref = _dictionary.getRef(obj._reportto);
				primaryheader[4] = SDNV(ref.first);
				primaryheader[5] = SDNV(ref.second);

				// custodian reference
				ref = _dictionary.getRef(obj._custodian);
				primaryheader[6] = SDNV(ref.first);
				primaryheader[7] = SDNV(ref.second);

				// dictionary size
				primaryheader[11] = SDNV(_dictionary.getSize());
				len += _dictionary.getSize();
			}

			for (int i = 0; i < 12; ++i)
			{
				len += primaryheader[i].getLength();
			}

			if (obj.get(dtn::data::Bundle::FRAGMENT))
			{
				primaryheader[12] = SDNV(obj.fragmentoffset);
				primaryheader[13] = SDNV(obj.appdatalength);

				len += primaryheader[12].getLength();
				len += primaryheader[13].getLength();
			}

			// write the block length
			_stream << SDNV(len);

			/*
			 * write the ref block of the dictionary
			 * this includes scheme and ssp for destination, source, reportto and custodian.
			 */
			for (int i = 0; i < 11; ++i)
			{
				_stream << primaryheader[i];
			}

			if (_compressable)
			{
				// write the size of the dictionary (always zero here)
				_stream << primaryheader[11];
			}
			else
			{
				// write size of dictionary + bytearray
				_stream << _dictionary;
			}

			if (obj.get(dtn::data::Bundle::FRAGMENT))
			{
				_stream << primaryheader[12]; // FRAGMENTATION_OFFSET
				_stream << primaryheader[13]; // APPLICATION_DATA_LENGTH
			}
			
			return (*this);
		}

		Serializer& DefaultSerializer::operator <<(const dtn::data::Block& obj)
		{
			_stream << obj.getType();
			_stream << dtn::data::SDNV(obj.getProcessingFlags());

			const Block::eid_list &eids = obj.getEIDList();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				_stream << SDNV(eids.size());
				for (Block::eid_list::const_iterator it = eids.begin(); it != eids.end(); ++it)
				{
					pair<size_t, size_t> offsets;

					if (_compressable)
					{
						offsets = (*it).getCompressed();
					}
					else
					{
						offsets = _dictionary.getRef(*it);
					}

					_stream << SDNV(offsets.first);
					_stream << SDNV(offsets.second);
				}
			}

			// write size of the payload in the block
			_stream << SDNV(obj.getLength());

			// write the payload of the block
			size_t slength = 0;
			obj.serialize(_stream, slength);

			return (*this);
		}

		Serializer& DefaultSerializer::serialize(const dtn::data::PayloadBlock& obj, size_t clip_offset, size_t clip_length)
		{
			_stream << obj.getType();
			_stream << dtn::data::SDNV(obj.getProcessingFlags());

			const Block::eid_list &eids = obj.getEIDList();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				_stream << SDNV(eids.size());
				for (Block::eid_list::const_iterator it = eids.begin(); it != eids.end(); ++it)
				{
					pair<size_t, size_t> offsets;

					if (_compressable)
					{
						offsets = (*it).getCompressed();
					}
					else
					{
						offsets = _dictionary.getRef(*it);
					}

					_stream << SDNV(offsets.first);
					_stream << SDNV(offsets.second);
				}
			}

			// get the remaining payload size
			size_t payload_size = obj.getLength();
			size_t remain = payload_size - clip_offset;

			// check if the remaining data length is >= clip_length
			if (payload_size < clip_offset)
			{
				// set the real predicted payload length
				// write size of the payload in the block
				_stream << SDNV(0);
			}
			else
			if (remain > clip_length)
			{
				// set the real predicted payload length
				// write size of the payload in the block
				_stream << SDNV(clip_length);

				// now skip the <offset>-bytes and all bytes after <offset + length>
				obj.serialize( _stream, clip_offset, clip_length );
			}
			else
			{
				// set the real predicted payload length
				// write size of the payload in the block
				_stream << SDNV(remain);

				// now skip the <offset>-bytes and all bytes after <offset + length>
				obj.serialize( _stream, clip_offset, remain );
			}

			return (*this);
		}

		size_t DefaultSerializer::getLength(const dtn::data::Bundle &obj)
		{
			// rebuild the dictionary
			rebuildDictionary(obj);

			size_t len = 0;
			len += getLength( (PrimaryBlock&)obj );
			
			// add size of all blocks
			for (Bundle::const_iterator iter = obj.begin(); iter != obj.end(); ++iter)
			{
				const Block &b = (**iter);
				len += getLength( b );
			}

			return len;
		}

		size_t DefaultSerializer::getLength(const dtn::data::PrimaryBlock& obj) const
		{
			size_t len = 0;

			len += sizeof(dtn::data::BUNDLE_VERSION);		// bundle version
			len += dtn::data::SDNV(obj.procflags).getLength();	// processing flags

			// primary header
			dtn::data::SDNV primaryheader[14];
			pair<size_t, size_t> ref;

			if (_compressable)
			{
				// destination reference
				ref = obj._destination.getCompressed();
				primaryheader[0] = SDNV(ref.first);
				primaryheader[1] = SDNV(ref.second);

				// source reference
				ref = obj.source.getCompressed();
				primaryheader[2] = SDNV(ref.first);
				primaryheader[3] = SDNV(ref.second);

				// reportto reference
				ref = obj._reportto.getCompressed();
				primaryheader[4] = SDNV(ref.first);
				primaryheader[5] = SDNV(ref.second);

				// custodian reference
				ref = obj._custodian.getCompressed();
				primaryheader[6] = SDNV(ref.first);
				primaryheader[7] = SDNV(ref.second);

				// dictionary size
				primaryheader[11] = SDNV(0);
			}
			else
			{
				// destination reference
				ref = _dictionary.getRef(obj._destination);
				primaryheader[0] = SDNV(ref.first);
				primaryheader[1] = SDNV(ref.second);

				// source reference
				ref = _dictionary.getRef(obj.source);
				primaryheader[2] = SDNV(ref.first);
				primaryheader[3] = SDNV(ref.second);

				// reportto reference
				ref = _dictionary.getRef(obj._reportto);
				primaryheader[4] = SDNV(ref.first);
				primaryheader[5] = SDNV(ref.second);

				// custodian reference
				ref = _dictionary.getRef(obj._custodian);
				primaryheader[6] = SDNV(ref.first);
				primaryheader[7] = SDNV(ref.second);

				// dictionary size
				primaryheader[11] = SDNV(_dictionary.getSize());
			}

			// timestamp
			primaryheader[8] = SDNV(obj.timestamp);

			// sequence number
			primaryheader[9] = SDNV(obj.sequencenumber);

			// lifetime
			primaryheader[10] = SDNV(obj.lifetime);

			for (int i = 0; i < 11; ++i)
			{
				len += primaryheader[i].getLength();
			}

			if (_compressable)
			{
				// write the size of the dictionary (always zero here)
				len += primaryheader[11].getLength();
			}
			else
			{
				// write size of dictionary + bytearray
				len += primaryheader[11].getLength();
				len += _dictionary.getSize();
			}

			if (obj.get(dtn::data::Bundle::FRAGMENT))
			{
				primaryheader[12] = SDNV(obj.fragmentoffset);
				primaryheader[13] = SDNV(obj.appdatalength);

				len += primaryheader[12].getLength();
				len += primaryheader[13].getLength();
			}

			len += SDNV(len).getLength();

			return len;
		}

		size_t DefaultSerializer::getLength(const dtn::data::Block &obj) const
		{
			size_t len = 0;

			len += sizeof(obj.getType());
			len += dtn::data::SDNV(obj.getProcessingFlags()).getLength();

			const Block::eid_list &eids = obj.getEIDList();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				len += dtn::data::SDNV(eids.size()).getLength();
				for (Block::eid_list::const_iterator it = eids.begin(); it != eids.end(); ++it)
				{
					pair<size_t, size_t> offsets = _dictionary.getRef(*it);
					len += SDNV(offsets.first).getLength();
					len += SDNV(offsets.second).getLength();
				}
			}

			// size of the payload in the block
			size_t payload_size = obj.getLength();
			len += payload_size;

			// size of the payload size
			len += SDNV(payload_size).getLength();

			return len;
		}

		DefaultDeserializer::DefaultDeserializer(std::istream& stream)
		 : _stream(stream), _validator(_default_validator), _compressed(false), _fragmentation(false)
		{
		}

		DefaultDeserializer::DefaultDeserializer(std::istream &stream, Validator &v)
		 : _stream(stream), _validator(v), _compressed(false), _fragmentation(false)
		{
		}

		DefaultDeserializer::DefaultDeserializer(std::istream &stream, const Dictionary &d)
		 : _stream(stream), _validator(_default_validator), _dictionary(d), _compressed(false), _fragmentation(false)
		{
		}

		void DefaultDeserializer::setFragmentationSupport(bool val)
		{
			_fragmentation = val;
		}

		Deserializer& DefaultDeserializer::operator >>(dtn::data::Bundle& obj)
		{
			// clear all blocks
			obj.clear();

			// read the primary block
			(*this) >> (PrimaryBlock&)obj;

			// read until the last block
			bool lastblock = false;

			char block_type;
			dtn::data::SDNV procflags_sdnv;

			// create a bundle builder
			dtn::data::BundleBuilder builder(obj);

			// read all BLOCKs
			while (!_stream.eof() && !lastblock)
			{
				// BLOCK_TYPE
				_stream.get(block_type);

				// read processing flags
				_stream >> procflags_sdnv;

				try {
					// create a block object
					dtn::data::Block &block = builder.insert(block_type, procflags_sdnv.getValue());

					try {
						// read block content
						(*this) >> block;
					} catch (dtn::PayloadReceptionInterrupted &ex) {
						// some debugging
						IBRCOMMON_LOGGER_DEBUG(15) << "Reception of bundle payload failed." << IBRCOMMON_LOGGER_ENDL;

						// interrupted transmission
						if (!obj.get(dtn::data::PrimaryBlock::DONT_FRAGMENT) && (block.getLength() > 0) && _fragmentation)
						{
							IBRCOMMON_LOGGER_DEBUG(25) << "Create a fragment." << IBRCOMMON_LOGGER_ENDL;

							if ( !obj.get(dtn::data::PrimaryBlock::FRAGMENT) )
							{
								obj.set(dtn::data::PrimaryBlock::FRAGMENT, true);
								obj.appdatalength = ex.length;
								obj.fragmentoffset = 0;
							}
						}
						else
						{
							throw;
						}
					}
				} catch (BundleBuilder::DiscardBlockException &ex) {
					// skip EIDs
					if ( procflags_sdnv.getValue() & dtn::data::Block::BLOCK_CONTAINS_EIDS )
					{
						SDNV eidcount;
						_stream >> eidcount;

						for (unsigned int i = 0; i < eidcount.getValue(); ++i)
						{
							SDNV scheme, ssp;
							_stream >> scheme;
							_stream >> ssp;
						}
					}

					// read the size of the payload in the block
					SDNV block_size;
					_stream >> block_size;

					// skip payload
					_stream.ignore(block_size.getValue());
				}

				lastblock = (procflags_sdnv.getValue() & Block::LAST_BLOCK);
			}

			// validate this bundle
			_validator.validate(obj);

			return (*this);
		}

		Deserializer& DefaultDeserializer::operator>>(dtn::data::MetaBundle &obj)
		{
			dtn::data::PrimaryBlock pb;
			(*this) >> pb;

			obj.appdatalength = pb.appdatalength;
			obj.custodian = pb._custodian;
			obj.destination = pb._destination;
			obj.expiretime = dtn::utils::Clock::getExpireTime(pb.timestamp, pb.lifetime);
			obj.fragment = pb.get(dtn::data::PrimaryBlock::FRAGMENT);
			obj.hopcount = 0;
			obj.lifetime = pb.lifetime;
			obj.offset = pb.fragmentoffset;
			obj.procflags = pb.procflags;
			obj.received = 0;
			obj.reportto = pb._reportto;
			obj.sequencenumber = pb.sequencenumber;
			obj.source = pb.source;
			obj.timestamp = pb.timestamp;

			return (*this);
		}

		Deserializer& DefaultDeserializer::operator >>(dtn::data::PrimaryBlock& obj)
		{
			char version = 0;
			SDNV tmpsdnv;
			SDNV blocklength;

			// check for the right version
			_stream.get(version);
			if (version != dtn::data::BUNDLE_VERSION) throw dtn::InvalidProtocolException("Bundle version differ from ours.");

			// PROCFLAGS
			_stream >> tmpsdnv;	// processing flags
			obj.procflags = tmpsdnv.getValue();

			// BLOCK LENGTH
			_stream >> blocklength;

			// EID References
			pair<SDNV, SDNV> ref[4];
			for (int i = 0; i < 4; ++i)
			{
				_stream >> ref[i].first;
				_stream >> ref[i].second;
			}

			// timestamp
			_stream >> tmpsdnv;
			obj.timestamp = tmpsdnv.getValue();

			// sequence number
			_stream >> tmpsdnv;
			obj.sequencenumber = tmpsdnv.getValue();

			// lifetime
			_stream >> tmpsdnv;
			obj.lifetime = tmpsdnv.getValue();

			try {
				// dictionary
				_stream >> _dictionary;

				// decode EIDs
				obj._destination = _dictionary.get(ref[0].first.getValue(), ref[0].second.getValue());
				obj.source = _dictionary.get(ref[1].first.getValue(), ref[1].second.getValue());
				obj._reportto = _dictionary.get(ref[2].first.getValue(), ref[2].second.getValue());
				obj._custodian = _dictionary.get(ref[3].first.getValue(), ref[3].second.getValue());
				_compressed = false;
			} catch (const dtn::InvalidDataException&) {
				// error while reading the dictionary. We assume that this is a compressed bundle header.
				obj._destination = dtn::data::EID(ref[0].first.getValue(), ref[0].second.getValue());
				obj.source = dtn::data::EID(ref[1].first.getValue(), ref[1].second.getValue());
				obj._reportto = dtn::data::EID(ref[2].first.getValue(), ref[2].second.getValue());
				obj._custodian = dtn::data::EID(ref[3].first.getValue(), ref[3].second.getValue());
				_compressed = true;
			}

			// fragmentation?
			if (obj.get(dtn::data::Bundle::FRAGMENT))
			{
				_stream >> tmpsdnv;
				obj.fragmentoffset = tmpsdnv.getValue();

				_stream >> tmpsdnv;
				obj.appdatalength = tmpsdnv.getValue();
			}
			
			// validate this primary block
			_validator.validate(obj);

			return (*this);
		}

		Deserializer& DefaultDeserializer::operator >>(dtn::data::Block& obj)
		{
			// read EIDs
			if ( obj.get(dtn::data::Block::BLOCK_CONTAINS_EIDS))
			{
				SDNV eidcount;
				_stream >> eidcount;

				for (unsigned int i = 0; i < eidcount.getValue(); ++i)
				{
					SDNV scheme, ssp;
					_stream >> scheme;
					_stream >> ssp;

					if (_compressed)
					{
						obj.addEID( dtn::data::EID(scheme.getValue(), ssp.getValue()) );
					}
					else
					{
						obj.addEID( _dictionary.get(scheme.getValue(), ssp.getValue()) );
					}
				}
			}

			// read the size of the payload in the block
			SDNV block_size;
			_stream >> block_size;

			// validate this block
			_validator.validate(obj, block_size.getValue());

			// read the payload of the block
			obj.deserialize(_stream, block_size.getValue());

			return (*this);
		}

		AcceptValidator::AcceptValidator()
		{
		}

		AcceptValidator::~AcceptValidator()
		{
		}

		void AcceptValidator::validate(const dtn::data::PrimaryBlock&) const throw (RejectedException)
		{
		}

		void AcceptValidator::validate(const dtn::data::Block&, const size_t) const throw (RejectedException)
		{

		}

		void AcceptValidator::validate(const dtn::data::Bundle&) const throw (RejectedException)
		{

		}

		SeparateSerializer::SeparateSerializer(std::ostream& stream)
		 : DefaultSerializer(stream)
		{
		}

		SeparateSerializer::~SeparateSerializer()
		{
		}

		Serializer& SeparateSerializer::operator <<(const dtn::data::Block& obj)
		{
			_stream << obj.getType();
			_stream << dtn::data::SDNV(obj.getProcessingFlags());

			const Block::eid_list &eids = obj.getEIDList();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (_ids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				_stream << SDNV(eids.size());
				for (Block::eid_list::const_iterator it = eids.begin(); it != eids.end(); ++it)
				{
					dtn::data::BundleString str((*it).getString());
					_stream << str;
				}
			}

			// write size of the payload in the block
			_stream << SDNV(obj.getLength());

			// write the payload of the block
			size_t slength = 0;
			obj.serialize(_stream, slength);

			return (*this);
		}

		size_t SeparateSerializer::getLength(const dtn::data::Block &obj) const
		{
			size_t len = 0;

			len += sizeof(obj.getType());
			len += dtn::data::SDNV(obj.getProcessingFlags()).getLength();

			const Block::eid_list &eids = obj.getEIDList();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				len += dtn::data::SDNV(eids.size()).getLength();
				for (Block::eid_list::const_iterator it = eids.begin(); it != eids.end(); ++it)
				{
					dtn::data::BundleString str((*it).getString());
					len += str.getLength();
				}
			}

			// size of the payload in the block
			len += obj.getLength();

			return len;
		}

		SeparateDeserializer::SeparateDeserializer(std::istream& stream, Bundle &b)
		 : DefaultDeserializer(stream), _bundle(b)
		{
		}

		SeparateDeserializer::~SeparateDeserializer()
		{
		}

		dtn::data::Block& SeparateDeserializer::readBlock()
		{
			BundleBuilder builder(_bundle);

			char block_type;
			dtn::data::SDNV procflags_sdnv;

			// BLOCK_TYPE
			_stream.get(block_type);

			// read processing flags
			_stream >> procflags_sdnv;

			// create a block object
			dtn::data::Block &obj = builder.insert(block_type, procflags_sdnv.getValue());

			// read EIDs
			if ( obj.get(dtn::data::Block::BLOCK_CONTAINS_EIDS))
			{
				SDNV eidcount;
				_stream >> eidcount;

				for (unsigned int i = 0; i < eidcount.getValue(); ++i)
				{
					dtn::data::BundleString str;
					_stream >> str;
					obj.addEID(dtn::data::EID(str));
				}
			}

			// read the size of the payload in the block
			SDNV block_size;
			_stream >> block_size;
//			obj._blocksize = block_size.getValue();

			// validate this block
			_validator.validate(obj, block_size.getValue());

			// read the payload of the block
			obj.deserialize(_stream, block_size.getValue());

			return obj;
		}
	}
}
