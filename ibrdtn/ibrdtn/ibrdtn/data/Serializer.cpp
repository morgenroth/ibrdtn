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
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/BundleString.h"
#include "ibrdtn/data/StatusReportBlock.h"
#include "ibrdtn/data/CustodySignalBlock.h"
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
			_dictionary.add(obj._source);
			_dictionary.add(obj._reportto);
			_dictionary.add(obj._custodian);

			// add EID of all secondary blocks
			std::list<refcnt_ptr<Block> > list = obj._blocks._blocks;

			for (std::list<refcnt_ptr<Block> >::const_iterator iter = list.begin(); iter != list.end(); iter++)
			{
				const Block &b = (*(*iter));
				_dictionary.add( b.getEIDList() );
			}
		}

		Serializer& DefaultSerializer::operator <<(const dtn::data::Bundle& obj)
		{
			// rebuild the dictionary
			rebuildDictionary(obj);

			// check if the bundle header could be compressed
			_compressable = isCompressable(obj);

			// serialize the primary block
			(*this) << (PrimaryBlock&)obj;

			// serialize all secondary blocks
			std::list<refcnt_ptr<Block> > list = obj._blocks._blocks;
			
			for (std::list<refcnt_ptr<Block> >::const_iterator iter = list.begin(); iter != list.end(); iter++)
			{
				const Block &b = (*(*iter));
				(*this) << b;
			}

			return (*this);
		}

		Serializer& DefaultSerializer::operator<<(const dtn::data::BundleFragment &obj)
		{
			// rebuild the dictionary
			rebuildDictionary(obj._bundle);

			// check if the bundle header could be compressed
			_compressable = isCompressable(obj._bundle);

			PrimaryBlock prim = obj._bundle;
			prim.set(dtn::data::PrimaryBlock::FRAGMENT, true);

			// set the application length according to the payload block size
			try {
				const dtn::data::PayloadBlock &payload = obj._bundle.getBlock<dtn::data::PayloadBlock>();
				prim._appdatalength = payload.getLength();
			} catch (dtn::data::Bundle::NoSuchBlockFoundException&) {
				prim._appdatalength = 0;
			}

			// set the fragmentation offset
			prim._fragmentoffset += obj._offset;

			// serialize the primary block
			(*this) << prim;

			// serialize all secondary blocks
			std::list<refcnt_ptr<Block> > list = obj._bundle._blocks._blocks;
			bool post_payload = false;

			for (std::list<refcnt_ptr<Block> >::const_iterator iter = list.begin(); iter != list.end(); iter++)
			{
				const Block &b = (*(*iter));

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
			bool compressable = ( obj._source.isCompressable() &&
					obj._destination.isCompressable() &&
					obj._reportto.isCompressable() &&
					obj._custodian.isCompressable() );

			if (compressable)
			{
				// add EID of all secondary blocks
				std::list<refcnt_ptr<Block> > list = obj._blocks._blocks;

				for (std::list<refcnt_ptr<Block> >::const_iterator iter = list.begin(); iter != list.end(); iter++)
				{
					const Block &b = (*(*iter));
					const std::list<dtn::data::EID> eids = b.getEIDList();

					for (std::list<dtn::data::EID>::const_iterator eit = eids.begin(); eit != eids.end(); eit++)
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
			_stream << dtn::data::SDNV(obj._procflags);	// processing flags

			// predict the block length
			size_t len = 0;
			dtn::data::SDNV primaryheader[14];

			primaryheader[8] = SDNV(obj._timestamp);		// timestamp
			primaryheader[9] = SDNV(obj._sequencenumber);	// sequence number
			primaryheader[10] = SDNV(obj._lifetime);		// lifetime

			pair<size_t, size_t> ref;

			if (_compressable)
			{
				// destination reference
				ref = obj._destination.getCompressed();
				primaryheader[0] = SDNV(ref.first);
				primaryheader[1] = SDNV(ref.second);

				// source reference
				ref = obj._source.getCompressed();
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
				ref = _dictionary.getRef(obj._source);
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

			for (int i = 0; i < 12; i++)
			{
				len += primaryheader[i].getLength();
			}

			if (obj.get(dtn::data::Bundle::FRAGMENT))
			{
				primaryheader[12] = SDNV(obj._fragmentoffset);
				primaryheader[13] = SDNV(obj._appdatalength);

				len += primaryheader[12].getLength();
				len += primaryheader[13].getLength();
			}

			// write the block length
			_stream << SDNV(len);

			/*
			 * write the ref block of the dictionary
			 * this includes scheme and ssp for destination, source, reportto and custodian.
			 */
			for (int i = 0; i < 11; i++)
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
			_stream << obj._blocktype;
			_stream << dtn::data::SDNV(obj._procflags);

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (_eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (obj._eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				_stream << SDNV(obj._eids.size());
				for (std::list<dtn::data::EID>::const_iterator it = obj._eids.begin(); it != obj._eids.end(); it++)
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
			_stream << obj._blocktype;
			_stream << dtn::data::SDNV(obj._procflags);

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (_eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (obj._eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				_stream << SDNV(obj._eids.size());
				for (std::list<dtn::data::EID>::const_iterator it = obj._eids.begin(); it != obj._eids.end(); it++)
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
			std::list<refcnt_ptr<Block> > list = obj._blocks._blocks;

			for (std::list<refcnt_ptr<Block> >::const_iterator iter = list.begin(); iter != list.end(); iter++)
			{
				const Block &b = (*(*iter));
				len += getLength( b );
			}

			return len;
		}

		size_t DefaultSerializer::getLength(const dtn::data::PrimaryBlock& obj) const
		{
			size_t len = 0;

			len += sizeof(dtn::data::BUNDLE_VERSION);		// bundle version
			len += dtn::data::SDNV(obj._procflags).getLength();	// processing flags

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
				ref = obj._source.getCompressed();
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
				ref = _dictionary.getRef(obj._source);
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
			primaryheader[8] = SDNV(obj._timestamp);

			// sequence number
			primaryheader[9] = SDNV(obj._sequencenumber);

			// lifetime
			primaryheader[10] = SDNV(obj._lifetime);

			for (int i = 0; i < 11; i++)
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
				primaryheader[12] = SDNV(obj._fragmentoffset);
				primaryheader[13] = SDNV(obj._appdatalength);

				len += primaryheader[12].getLength();
				len += primaryheader[13].getLength();
			}

			len += SDNV(len).getLength();

			return len;
		}

		size_t DefaultSerializer::getLength(const dtn::data::Block &obj) const
		{
			size_t len = 0;

			len += sizeof(obj._blocktype);
			len += dtn::data::SDNV(obj._procflags).getLength();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (_eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (obj._eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				len += dtn::data::SDNV(obj._eids.size()).getLength();
				for (std::list<dtn::data::EID>::const_iterator it = obj._eids.begin(); it != obj._eids.end(); it++)
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
			obj.clearBlocks();

			// read the primary block
			(*this) >> (PrimaryBlock&)obj;

			// read until the last block
			bool lastblock = false;

			// read all BLOCKs
			while (!_stream.eof() && !lastblock)
			{
				char block_type;

				// BLOCK_TYPE
				block_type = _stream.peek();

				switch (block_type)
				{
					case 0:
					{
						throw dtn::InvalidDataException("block type is zero");
						break;
					}

					case dtn::data::PayloadBlock::BLOCK_TYPE:
					{
						if (obj.get(dtn::data::Bundle::APPDATA_IS_ADMRECORD))
						{
							// create a temporary block
							dtn::data::ExtensionBlock &block = obj.push_back<dtn::data::ExtensionBlock>();

							// read the block data
							(*this) >> block;

							// access the payload to get the first byte
							char admfield;
							ibrcommon::BLOB::Reference ref = block.getBLOB();
							ref.iostream()->get(admfield);

							// write the block into a temporary stream
							stringstream ss;
							DefaultSerializer serializer(ss, _dictionary);
							DefaultDeserializer deserializer(ss, _dictionary);

							serializer << block;

							// remove the temporary block
							obj.remove(block);

							switch (admfield >> 4)
							{
								case 1:
								{
									dtn::data::StatusReportBlock &block = obj.push_back<dtn::data::StatusReportBlock>();
									deserializer >> block;
									lastblock = block.get(Block::LAST_BLOCK);
									break;
								}

								case 2:
								{
									dtn::data::CustodySignalBlock &block = obj.push_back<dtn::data::CustodySignalBlock>();
									deserializer >> block;
									lastblock = block.get(Block::LAST_BLOCK);
									break;
								}

								default:
								{
									// drop unknown administrative block
									break;
								}
							}

						}
						else
						{
							dtn::data::PayloadBlock &block = obj.push_back<dtn::data::PayloadBlock>();

							try {
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
										obj._appdatalength = ex.length;
										obj._fragmentoffset = 0;
									}
								}
								else
								{
									throw ex;
								}
							}

							lastblock = block.get(Block::LAST_BLOCK);
						}
						break;
					}

					default:
					{
						// get a extension block factory
						try {
							ExtensionBlock::Factory &f = dtn::data::ExtensionBlock::Factory::get(block_type);

							dtn::data::Block &block = obj.push_back(f);
							(*this) >> block;
							lastblock = block.get(Block::LAST_BLOCK);
						}
						catch (const ibrcommon::Exception &ex)
						{
							dtn::data::ExtensionBlock &block = obj.push_back<dtn::data::ExtensionBlock>();
							(*this) >> block;
							lastblock = block.get(Block::LAST_BLOCK);

							if (block.get(dtn::data::Block::DISCARD_IF_NOT_PROCESSED))
							{
								IBRCOMMON_LOGGER_DEBUG(5) << "unprocessable block in bundle " << obj.toString() << " has been removed" << IBRCOMMON_LOGGER_ENDL;

								// remove the block
								obj.remove(block);
							}
						}
						break;
					}
				}
			}

			// validate this bundle
			_validator.validate(obj);

			return (*this);
		}

		Deserializer& DefaultDeserializer::operator>>(dtn::data::MetaBundle &obj)
		{
			dtn::data::PrimaryBlock pb;
			(*this) >> pb;

			obj.appdatalength = pb._appdatalength;
			obj.custodian = pb._custodian;
			obj.destination = pb._destination;
			obj.expiretime = dtn::utils::Clock::getExpireTime(pb._timestamp, pb._lifetime);
			obj.fragment = pb.get(dtn::data::PrimaryBlock::FRAGMENT);
			obj.hopcount = 0;
			obj.lifetime = pb._lifetime;
			obj.offset = pb._fragmentoffset;
			obj.procflags = pb._procflags;
			obj.received = 0;
			obj.reportto = pb._reportto;
			obj.sequencenumber = pb._sequencenumber;
			obj.source = pb._source;
			obj.timestamp = pb._timestamp;

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
			obj._procflags = tmpsdnv.getValue();

			// BLOCK LENGTH
			_stream >> blocklength;

			// EID References
			pair<SDNV, SDNV> ref[4];
			for (int i = 0; i < 4; i++)
			{
				_stream >> ref[i].first;
				_stream >> ref[i].second;
			}

			// timestamp
			_stream >> tmpsdnv;
			obj._timestamp = tmpsdnv.getValue();

			// sequence number
			_stream >> tmpsdnv;
			obj._sequencenumber = tmpsdnv.getValue();

			// lifetime
			_stream >> tmpsdnv;
			obj._lifetime = tmpsdnv.getValue();

			try {
				// dictionary
				_stream >> _dictionary;

				// decode EIDs
				obj._destination = _dictionary.get(ref[0].first.getValue(), ref[0].second.getValue());
				obj._source = _dictionary.get(ref[1].first.getValue(), ref[1].second.getValue());
				obj._reportto = _dictionary.get(ref[2].first.getValue(), ref[2].second.getValue());
				obj._custodian = _dictionary.get(ref[3].first.getValue(), ref[3].second.getValue());
				_compressed = false;
			} catch (const dtn::InvalidDataException&) {
				// error while reading the dictionary. We assume that this is a compressed bundle header.
				obj._destination = dtn::data::EID(ref[0].first.getValue(), ref[0].second.getValue());
				obj._source = dtn::data::EID(ref[1].first.getValue(), ref[1].second.getValue());
				obj._reportto = dtn::data::EID(ref[2].first.getValue(), ref[2].second.getValue());
				obj._custodian = dtn::data::EID(ref[3].first.getValue(), ref[3].second.getValue());
				_compressed = true;
			}

			// fragmentation?
			if (obj.get(dtn::data::Bundle::FRAGMENT))
			{
				_stream >> tmpsdnv;
				obj._fragmentoffset = tmpsdnv.getValue();

				_stream >> tmpsdnv;
				obj._appdatalength = tmpsdnv.getValue();
			}
			
			// validate this primary block
			_validator.validate(obj);

			return (*this);
		}

		Deserializer& DefaultDeserializer::operator >>(dtn::data::Block& obj)
		{
			dtn::data::SDNV procflags_sdnv;
			_stream.get(obj._blocktype);
			_stream >> procflags_sdnv;

			// set the processing flags but do not overwrite the lastblock bit
			obj._procflags = procflags_sdnv.getValue() & (~(dtn::data::Block::LAST_BLOCK) | obj._procflags);

			// read EIDs
			if ( obj.get(dtn::data::Block::BLOCK_CONTAINS_EIDS))
			{
				SDNV eidcount;
				_stream >> eidcount;

				for (unsigned int i = 0; i < eidcount.getValue(); i++)
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
			_stream << obj._blocktype;
			_stream << dtn::data::SDNV(obj._procflags);

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (_eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (obj._eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				_stream << SDNV(obj._eids.size());
				for (std::list<dtn::data::EID>::const_iterator it = obj._eids.begin(); it != obj._eids.end(); it++)
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

			len += sizeof(obj._blocktype);
			len += dtn::data::SDNV(obj._procflags).getLength();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (_eids.size() > 0)
			assert(!obj.get(Block::BLOCK_CONTAINS_EIDS) || (obj._eids.size() > 0));
#endif

			if (obj.get(Block::BLOCK_CONTAINS_EIDS))
			{
				len += dtn::data::SDNV(obj._eids.size()).getLength();
				for (std::list<dtn::data::EID>::const_iterator it = obj._eids.begin(); it != obj._eids.end(); it++)
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
			char block_type;

			// BLOCK_TYPE
			block_type = _stream.peek();

			switch (block_type)
			{
				case 0:
				{
					throw dtn::InvalidDataException("block type is zero");
					break;
				}

				case dtn::data::PayloadBlock::BLOCK_TYPE:
				{
					if (_bundle.get(dtn::data::Bundle::APPDATA_IS_ADMRECORD))
					{
						// create a temporary block
						dtn::data::ExtensionBlock &block = _bundle.push_back<dtn::data::ExtensionBlock>();

						// remember the current read position
						int blockbegin = _stream.tellg();

						// read the block data
						(*this) >> block;

						// access the payload to get the first byte
						char admfield;
						ibrcommon::BLOB::Reference ref = block.getBLOB();
						ref.iostream()->get(admfield);

						// remove the temporary block
						_bundle.remove(block);

						// reset the read pointer
						// BEWARE: this will not work on non-buffered streams like TCP!
						_stream.seekg(blockbegin);

						switch (admfield >> 4)
						{
							case 1:
							{
								dtn::data::StatusReportBlock &block = _bundle.push_back<dtn::data::StatusReportBlock>();
								(*this) >> block;
								return block;
							}

							case 2:
							{
								dtn::data::CustodySignalBlock &block = _bundle.push_back<dtn::data::CustodySignalBlock>();
								(*this) >> block;
								return block;
							}

							default:
							{
								// drop unknown administrative block
								return block;
							}
						}

					}
					else
					{
						dtn::data::PayloadBlock &block = _bundle.push_back<dtn::data::PayloadBlock>();
						(*this) >> block;
						return block;
					}
					break;
				}

				default:
				{
					// get a extension block factory
					try {
						ExtensionBlock::Factory &f = dtn::data::ExtensionBlock::Factory::get(block_type);
						dtn::data::Block &block = _bundle.push_back(f);
						(*this) >> block;
						return block;
					} catch (const ibrcommon::Exception &ex) {
						dtn::data::ExtensionBlock &block = _bundle.push_back<dtn::data::ExtensionBlock>();
						(*this) >> block;
						return block;
					}
					break;
				}
			}
		}

		Deserializer&  SeparateDeserializer::operator >>(dtn::data::Block& obj)
		{
			dtn::data::SDNV procflags_sdnv;
			_stream.get(obj._blocktype);
			_stream >> procflags_sdnv;
			obj._procflags = procflags_sdnv.getValue();

			// read EIDs
			if ( obj.get(dtn::data::Block::BLOCK_CONTAINS_EIDS))
			{
				SDNV eidcount;
				_stream >> eidcount;

				for (unsigned int i = 0; i < eidcount.getValue(); i++)
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

			return (*this);
		}
	}
}
