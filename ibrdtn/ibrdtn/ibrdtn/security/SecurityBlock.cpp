/*
 * SecurityBlock.cpp
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
#include "ibrdtn/security/SecurityBlock.h"
#include "ibrdtn/security/MutableSerializer.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/ExtensionBlock.h"

#include <ibrcommon/Logger.h>
#include <cstdlib>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <vector>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

// include code for platform-independent endianess conversion
#include "ibrdtn/data/Endianess.h"

namespace dtn
{
	namespace security
	{
		const std::string SecurityBlock::TLVList::toString() const
		{
			std::stringstream ss;

			for (std::set<TLV>::const_iterator iter = begin(); iter != end(); ++iter)
			{
				ss << (*iter);
			}

			return ss.str();
		}

		dtn::data::Length SecurityBlock::TLVList::getLength() const
		{
			return getPayloadLength();
		}

		dtn::data::Length SecurityBlock::TLVList::getPayloadLength() const
		{
			dtn::data::Length len = 0;

			for (std::set<SecurityBlock::TLV>::const_iterator iter = begin(); iter != end(); ++iter)
			{
				len += (*iter).getLength();
			}

			return len;
		}

		const std::string SecurityBlock::TLVList::get(SecurityBlock::TLV_TYPES type) const
		{
			for (std::set<SecurityBlock::TLV>::const_iterator iter = begin(); iter != end(); ++iter)
			{
				if ((*iter).getType() == type)
				{
					return (*iter).getValue();
				}
			}

			throw ElementMissingException();
		}

		void SecurityBlock::TLVList::get(TLV_TYPES type, unsigned char *value, dtn::data::Length length) const
		{
			const std::string data = get(type);

			if (length < data.size())
			{
				::memcpy(value, data.c_str(), length);
			}
			else
			{
				::memcpy(value, data.c_str(), data.size());
			}
		}

		void SecurityBlock::TLVList::set(SecurityBlock::TLV_TYPES type, std::string value)
		{
			SecurityBlock::TLV tlv(type, value);

			erase(tlv);
			insert(tlv);
		}

		void SecurityBlock::TLVList::set(TLV_TYPES type, const unsigned char *value, dtn::data::Length length)
		{
			const std::string data(reinterpret_cast<const char *>(value), length);
			set(type, data);
		}

		void SecurityBlock::TLVList::remove(SecurityBlock::TLV_TYPES type)
		{
			erase(SecurityBlock::TLV(type, ""));
		}

		const std::string SecurityBlock::TLV::getValue() const
		{
			return _value;
		}

		SecurityBlock::TLV_TYPES SecurityBlock::TLV::getType() const
		{
			return _type;
		}

		dtn::data::Length SecurityBlock::TLV::getLength() const
		{
			return _value.getLength() + sizeof(char);
		}

		std::ostream& operator<<(std::ostream &stream, const SecurityBlock::TLVList &tlvlist)
		{
			dtn::data::Number length(tlvlist.getPayloadLength());
			stream << length;

			for (std::set<SecurityBlock::TLV>::const_iterator iter = tlvlist.begin(); iter != tlvlist.end(); ++iter)
			{
				stream << (*iter);
			}
			return stream;
		}

		std::istream& operator>>(std::istream &stream, SecurityBlock::TLVList &tlvlist)
		{
			dtn::data::Number length;
			stream >> length;
			dtn::data::Length read_length = 0;

			while (length > read_length)
			{
				SecurityBlock::TLV tlv;
				stream >> tlv;
				tlvlist.insert(tlv);
				read_length += tlv.getLength();
			}

			return stream;
		}

		bool SecurityBlock::TLV::operator<(const SecurityBlock::TLV &tlv) const
		{
			return (_type < tlv._type);
		}

		bool SecurityBlock::TLV::operator==(const SecurityBlock::TLV &tlv) const
		{
			return (_type == tlv._type);
		}

		std::ostream& operator<<(std::ostream &stream, const SecurityBlock::TLV &tlv)
		{
			stream.put((char)tlv._type);
			stream << tlv._value;
			return stream;
		}

		std::istream& operator>>(std::istream &stream, SecurityBlock::TLV &tlv)
		{
			char tlv_type;
			stream.get(tlv_type); tlv._type = SecurityBlock::TLV_TYPES(tlv_type);
			stream >> tlv._value;
			return stream;
		}

		SecurityBlock::SecurityBlock(const dtn::security::SecurityBlock::BLOCK_TYPES type, const dtn::security::SecurityBlock::CIPHERSUITE_IDS id)
		: Block(type), _ciphersuite_id(id), _ciphersuite_flags(0), _correlator(0)
		{

		}

		SecurityBlock::SecurityBlock(const dtn::security::SecurityBlock::BLOCK_TYPES type)
		: Block(type), _ciphersuite_id(0), _ciphersuite_flags(0), _correlator(0)
		{

		}

		SecurityBlock::~SecurityBlock()
		{
		}

		void SecurityBlock::store_security_references()
		{
			// clear the EID list
			_eids.clear();

			// first the security source
			if (_security_source == dtn::data::EID())
			{
				_ciphersuite_flags &= ~(SecurityBlock::CONTAINS_SECURITY_SOURCE);
			}
			else
			{
				_ciphersuite_flags |= SecurityBlock::CONTAINS_SECURITY_SOURCE;
				_eids.push_back(_security_source);
			}

			// then the destination
			if (_security_destination == dtn::data::EID())
			{
				_ciphersuite_flags &= ~(SecurityBlock::CONTAINS_SECURITY_DESTINATION);
			}
			else
			{
				_ciphersuite_flags |= SecurityBlock::CONTAINS_SECURITY_DESTINATION;
				_eids.push_back(_security_destination);
			}

			if (_eids.size() > 0)
			{
				set(Block::BLOCK_CONTAINS_EIDS, true);
			}
			else
			{
				set(Block::BLOCK_CONTAINS_EIDS, false);
			}
		}

		const dtn::data::EID SecurityBlock::getSecuritySource() const
		{
			return _security_source;
		}

		const dtn::data::EID SecurityBlock::getSecurityDestination() const
		{
			return _security_destination;
		}

		void SecurityBlock::setSecuritySource(const dtn::data::EID &source)
		{
			_security_source = source;
			store_security_references();
		}

		void SecurityBlock::setSecurityDestination(const dtn::data::EID &destination)
		{
			_security_destination = destination;
			store_security_references();
		}

		void SecurityBlock::setCiphersuiteId(const CIPHERSUITE_IDS id)
		{
			_ciphersuite_id = static_cast<uint64_t>(id);
		}

		void SecurityBlock::setCorrelator(const dtn::data::Number &corr)
		{
			_correlator = corr;
			_ciphersuite_flags |= SecurityBlock::CONTAINS_CORRELATOR;
		}

		bool SecurityBlock::isCorrelatorPresent(const dtn::data::Bundle& bundle, const dtn::data::Number &correlator)
		{
			bool return_val = false;
			for (dtn::data::Bundle::const_iterator it = bundle.begin(); it != bundle.end() && !return_val; ++it)
			{
				const dtn::data::Block &b = (**it);
				const dtn::data::block_t type = b.getType();
				if (type == BUNDLE_AUTHENTICATION_BLOCK
					|| type == PAYLOAD_INTEGRITY_BLOCK
					|| type == PAYLOAD_CONFIDENTIAL_BLOCK
					|| type == EXTENSION_SECURITY_BLOCK)
				return_val = static_cast<const SecurityBlock&>(b)._correlator == correlator;
			}
			return return_val;
		}

		dtn::data::Number SecurityBlock::createCorrelatorValue(const dtn::data::Bundle& bundle)
		{
			dtn::data::Number corr;
			corr.random<uint32_t>();

			while (isCorrelatorPresent(bundle, corr)) {
				corr.random<uint32_t>();
			}
			return corr;
		}

		dtn::data::Length SecurityBlock::getLength() const
		{
			dtn::data::Length length = _ciphersuite_id.getLength()
				+ _ciphersuite_flags.getLength();

			if (_ciphersuite_flags & CONTAINS_CORRELATOR)
			{
				length += _correlator.getLength();
			}

			if (_ciphersuite_flags & CONTAINS_CIPHERSUITE_PARAMS)
			{
				const dtn::data::Number size(_ciphersuite_params.getLength());
				length += size.getLength() + size.get<dtn::data::Length>();
			}

			if (_ciphersuite_flags & CONTAINS_SECURITY_RESULT)
			{
				const dtn::data::Number size(getSecurityResultSize());
				length += size.getLength() + size.get<dtn::data::Length>();
			}

			return length;
		}

		dtn::data::Length SecurityBlock::getLength_mutable() const
		{
			// ciphersuite_id
			dtn::data::Length length = MutableSerializer::sdnv_size;

			// ciphersuite_flags
			length += MutableSerializer::sdnv_size;

			// correlator
			if (_ciphersuite_flags & CONTAINS_CORRELATOR)
			{
				length += MutableSerializer::sdnv_size;
			}

			// ciphersuite parameters
			if (_ciphersuite_flags & CONTAINS_CIPHERSUITE_PARAMS)
			{
				length += MutableSerializer::sdnv_size;
				length += _ciphersuite_params.getLength();
			}
			// security result
			if (_ciphersuite_flags & CONTAINS_SECURITY_RESULT)
			{
				length += MutableSerializer::sdnv_size + getSecurityResultSize();
			}

			return length;
		}

		std::ostream& SecurityBlock::serialize(std::ostream &stream, dtn::data::Length &) const
		{
			stream << _ciphersuite_id << _ciphersuite_flags;

			if (_ciphersuite_flags & CONTAINS_CORRELATOR)
			{
				stream << _correlator;
			}

			if (_ciphersuite_flags & CONTAINS_CIPHERSUITE_PARAMS)
			{
				stream << _ciphersuite_params;
			}

			if (_ciphersuite_flags & CONTAINS_SECURITY_RESULT)
			{
				stream << _security_result;
			}

			return stream;
		}

		std::ostream& SecurityBlock::serialize_strict(std::ostream &stream, dtn::data::Length &) const
		{
			stream << _ciphersuite_id << _ciphersuite_flags;

			if (_ciphersuite_flags & CONTAINS_CORRELATOR)
			{
				stream << _correlator;
			}

			if (_ciphersuite_flags & CONTAINS_CIPHERSUITE_PARAMS)
			{
				stream << _ciphersuite_params;
			}

			if (_ciphersuite_flags & CONTAINS_SECURITY_RESULT)
			{
				stream << dtn::data::Number(getSecurityResultSize());
			}

			return stream;
		}

		std::istream& SecurityBlock::deserialize(std::istream &stream, const dtn::data::Length&)
		{
#ifdef __DEVELOPMENT_ASSERTIONS__
			// recheck blocktype. if blocktype is set wrong, this will be a huge fail
			assert(_blocktype == BUNDLE_AUTHENTICATION_BLOCK || _blocktype == PAYLOAD_INTEGRITY_BLOCK || _blocktype == PAYLOAD_CONFIDENTIAL_BLOCK || _blocktype == EXTENSION_SECURITY_BLOCK);
#endif

			stream >> _ciphersuite_id >> _ciphersuite_flags;

#ifdef __DEVELOPMENT_ASSERTIONS__
			// recheck ciphersuite_id
			assert(_ciphersuite_id == BAB_HMAC || _ciphersuite_id == PIB_RSA_SHA256 || _ciphersuite_id == PCB_RSA_AES128_PAYLOAD_PIB_PCB || _ciphersuite_id == ESB_RSA_AES128_EXT);
			// recheck ciphersuite_flags, could be more exhaustive
			assert(_ciphersuite_flags < 32);
#endif

			// copy security source and destination
			if (_ciphersuite_flags & SecurityBlock::CONTAINS_SECURITY_SOURCE)
			{
				if (_eids.size() == 0)
					throw dtn::SerializationFailedException("ciphersuite flags indicate a security source, but it is not present");

				_security_source = _eids.front();
			}

			if (_ciphersuite_flags & SecurityBlock::CONTAINS_SECURITY_DESTINATION)
			{
				if (_ciphersuite_flags & SecurityBlock::CONTAINS_SECURITY_SOURCE)
				{
					if (_eids.size() < 2)
						throw dtn::SerializationFailedException("ciphersuite flags indicate a security destination, but it is not present");

					_security_destination = *(++_eids.begin());
				}
				else
				{
					if (_eids.size() == 0)
						throw dtn::SerializationFailedException("ciphersuite flags indicate a security destination, but it is not present");

					_security_destination = _eids.front();
				}
			}

			if (_ciphersuite_flags & CONTAINS_CORRELATOR)
			{
				stream >> _correlator;
			}
			if (_ciphersuite_flags & CONTAINS_CIPHERSUITE_PARAMS)
			{
				stream >> _ciphersuite_params;
#ifdef __DEVELOPMENT_ASSERTIONS__
				assert(_ciphersuite_params.getLength() > 0);
#endif
			}

 			if (_ciphersuite_flags & CONTAINS_SECURITY_RESULT)
			{
 				stream >> _security_result;
#ifdef __DEVELOPMENT_ASSERTIONS__
				assert(_security_result.getLength() > 0);
#endif
			}

			return stream;
		}

		dtn::security::MutableSerializer& SecurityBlock::serialize_mutable(dtn::security::MutableSerializer &serializer, bool include_security_result) const
		{
			serializer << _ciphersuite_id;
			serializer << _ciphersuite_flags;

			if (_ciphersuite_flags & CONTAINS_CORRELATOR)
			{
				serializer << _correlator;
			}

			if (_ciphersuite_flags & CONTAINS_CIPHERSUITE_PARAMS)
			{
				serializer << _ciphersuite_params;
			}

			if (_ciphersuite_flags & CONTAINS_SECURITY_RESULT)
			{
				if (include_security_result) {
					serializer << _security_result;
				} else {
					serializer << dtn::data::Number(getSecurityResultSize());
				}
			}

			return serializer;
		}

		dtn::data::Length SecurityBlock::getSecurityResultSize() const
		{
#ifdef __DEVELOPMENT_ASSERTIONS__
			assert(_security_result.getLength() != 0);
#endif
			return _security_result.getLength();
		}

		void SecurityBlock::createSaltAndKey(uint32_t& salt, unsigned char* key, dtn::data::Length key_size)
		{

			if (!RAND_bytes(reinterpret_cast<unsigned char *>(&salt), sizeof(uint32_t)))
			{
				IBRCOMMON_LOGGER_ex(critical) << "failed to generate salt. maybe /dev/urandom is missing for seeding the PRNG" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
			if (!RAND_bytes(key, static_cast<int>(key_size)))
			{
				IBRCOMMON_LOGGER_ex(critical) << "failed to generate key. maybe /dev/urandom is missing for seeding the PRNG" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
		}

		void SecurityBlock::addKey(TLVList& security_parameter, unsigned char const * const key, dtn::data::Length key_size, RSA * rsa)
		{
			// encrypt the ephemeral key and place it in _ciphersuite_params
#ifdef __DEVELOPMENT_ASSERTIONS__
			assert(key_size < (dtn::data::Length)RSA_size(rsa)-41);
#endif
			std::vector<unsigned char> encrypted_key(RSA_size(rsa));
			int encrypted_key_len = RSA_public_encrypt(static_cast<int>(key_size), key, &encrypted_key[0], rsa, RSA_PKCS1_OAEP_PADDING);
			if (encrypted_key_len == -1)
			{
				IBRCOMMON_LOGGER_ex(critical) << "failed to encrypt the symmetric AES key" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
				return;
			}
			security_parameter.set(SecurityBlock::key_information, std::string(reinterpret_cast<char *>(&encrypted_key[0]), encrypted_key_len));
		}

		bool SecurityBlock::getKey(const TLVList& security_parameter, unsigned char * key, dtn::data::Length key_size, RSA * rsa)
		{
			std::string key_string = security_parameter.get(SecurityBlock::key_information);
			// get key, convert with reinterpret_cast
			const unsigned char *encrypted_key = reinterpret_cast<const unsigned char*>(key_string.c_str());
			std::vector<unsigned char> the_key(RSA_size(rsa));
			RSA_blinding_on(rsa, NULL);
			int plaintext_key_len = RSA_private_decrypt(static_cast<int>(key_string.size()), encrypted_key, &the_key[0], rsa, RSA_PKCS1_OAEP_PADDING);
			RSA_blinding_off(rsa);
			if (plaintext_key_len == -1)
			{
				IBRCOMMON_LOGGER_ex(critical) << "failed to decrypt the symmetric AES key" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
				return false;
			}
#ifdef __DEVELOPMENT_ASSERTIONS__
			assert((dtn::data::Length)plaintext_key_len == key_size);
#endif
			std::copy(&the_key[0], &the_key[key_size], key);
			return true;
		}

		void SecurityBlock::copyEID(const Block& from, Block& to, dtn::data::Length skip)
		{
			// take eid list, getEIDList() is broken
			std::list<dtn::data::EID> their_eids = from.getEIDList();
			std::list<dtn::data::EID>::iterator it = their_eids.begin();

			while (it != their_eids.end() && skip > 0)
			{
				skip--;
				++it;
			}

			for (; it != their_eids.end(); ++it)
				to.addEID(*it);
		}

		void SecurityBlock::addSalt(TLVList& security_parameters, const uint32_t &salt)
		{
			uint32_t nsalt = GUINT32_TO_BE(salt);
			security_parameters.set(SecurityBlock::salt, (const unsigned char*)&nsalt, sizeof(nsalt));
		}

		uint32_t SecurityBlock::getSalt(const TLVList& security_parameters)
		{
			uint32_t nsalt = 0;
			security_parameters.get(SecurityBlock::salt, (unsigned char*)&nsalt, sizeof(nsalt));
			return GUINT32_TO_BE(nsalt);
		}

		void SecurityBlock::decryptBlock(dtn::data::Bundle& bundle, dtn::data::Bundle::iterator &it, uint32_t salt, const unsigned char key[ibrcommon::AES128Stream::key_size_in_bytes])
		{
			const dtn::security::SecurityBlock &block = dynamic_cast<const dtn::security::SecurityBlock&>(**it);

			// the array for the extracted tag
			unsigned char tag[ibrcommon::AES128Stream::tag_len];

			// the array for the extracted iv
			unsigned char iv[ibrcommon::AES128Stream::iv_len];

			// get iv, convert with reinterpret_cast
			block._ciphersuite_params.get(SecurityBlock::initialization_vector, iv, ibrcommon::AES128Stream::iv_len);

			// get data and tag, the last tag_len bytes are the tag. cut them of and reinterpret_cast
			std::string block_data = block._security_result.get(SecurityBlock::encapsulated_block);

			// create a pointer to the tag begin
			const char *tag_p = block_data.c_str() + (block_data.size() - ibrcommon::AES128Stream::tag_len);

			// copy the tag
			::memcpy(tag, tag_p, ibrcommon::AES128Stream::tag_len);

			// strip off the tag from block data
			block_data.resize(block_data.size() - ibrcommon::AES128Stream::tag_len);

			// decrypt block
			std::stringstream plaintext;
			ibrcommon::AES128Stream decrypt(ibrcommon::CipherStream::CIPHER_DECRYPT, plaintext, key, salt, iv);
			decrypt << block_data << std::flush;

			// verify the decrypt tag
			if (!decrypt.verify(tag))
			{
				throw DecryptException("decryption of block failed - tag is bad");
			}

			// deserialize block
			dtn::data::DefaultDeserializer ddser(plaintext);

			// peek the block type
			dtn::data::block_t block_type = (dtn::data::block_t)plaintext.peek();

			// get the position of "block"
			dtn::data::Bundle::iterator b_it = bundle.find(block);

			if (block_type == dtn::data::PayloadBlock::BLOCK_TYPE)
			{
				dtn::data::PayloadBlock &plaintext_block = bundle.insert<dtn::data::PayloadBlock>(b_it);
				ddser >> plaintext_block;
			}
			else
			{
				try {
					dtn::data::ExtensionBlock::Factory &f = dtn::data::ExtensionBlock::Factory::get(block_type);
					dtn::data::Block &plaintext_block = bundle.insert(b_it, f);
					ddser >> plaintext_block;

					plaintext_block.clearEIDs();

					// copy eids
					// remove security_source and destination
					dtn::data::Length skip = 0;
					if (block._ciphersuite_flags & SecurityBlock::CONTAINS_SECURITY_DESTINATION)
						skip++;
					if (block._ciphersuite_flags & SecurityBlock::CONTAINS_SECURITY_SOURCE)
						skip++;
					copyEID(plaintext_block, plaintext_block, skip);

				} catch (const ibrcommon::Exception &ex) {
					dtn::data::ExtensionBlock &plaintext_block = bundle.insert<dtn::data::ExtensionBlock>(b_it);
					ddser >> plaintext_block;

					plaintext_block.clearEIDs();

					// copy eids
					// remove security_source and destination
					dtn::data::Length skip = 0;
					if (block._ciphersuite_flags & SecurityBlock::CONTAINS_SECURITY_DESTINATION)
						skip++;
					if (block._ciphersuite_flags & SecurityBlock::CONTAINS_SECURITY_SOURCE)
						skip++;
					copyEID(plaintext_block, plaintext_block, skip);
				}
			}

			bundle.remove(block);
		}

		void SecurityBlock::addFragmentRange(TLVList& ciphersuite_params, const dtn::data::Number &offset, const dtn::data::Number &range)
		{
			std::stringstream ss;
			ss << offset << range;

			ciphersuite_params.set(SecurityBlock::fragment_range, ss.str());
		}

		void SecurityBlock::getFragmentRange(const TLVList& ciphersuite_params, dtn::data::Number &offset, dtn::data::Number &range)
		{
			std::stringstream ss( ciphersuite_params.get(SecurityBlock::fragment_range) );
			ss >> offset >> range;
		}

		bool SecurityBlock::isSecuritySource(const dtn::data::Bundle& bundle, const dtn::data::EID& eid) const
		{
			IBRCOMMON_LOGGER_DEBUG_TAG("SecurityBlock", 30) << "check security source: " << getSecuritySource(bundle).getString() << " == " << eid.getNode().getString() << IBRCOMMON_LOGGER_ENDL;
			return getSecuritySource(bundle).sameHost(eid);
		}

		bool SecurityBlock::isSecurityDestination(const dtn::data::Bundle& bundle, const dtn::data::EID& eid) const
		{
			IBRCOMMON_LOGGER_DEBUG_TAG("SecurityBlock", 30) << "check security destination: " << getSecurityDestination(bundle).getString() << " == " << eid.getNode().getString() << IBRCOMMON_LOGGER_ENDL;
			return getSecurityDestination(bundle).sameHost(eid);
		}
		
		const dtn::data::EID SecurityBlock::getSecuritySource(const dtn::data::Bundle& bundle) const
		{
			const dtn::data::EID source = getSecuritySource();
			if (source.isNone())
				return bundle.source.getNode();
			return source;
		}

		const dtn::data::EID SecurityBlock::getSecurityDestination(const dtn::data::Bundle& bundle) const
		{
			const dtn::data::EID destination = getSecurityDestination();
			if (destination.isNone())
				return bundle.destination.getNode();
			return destination;
		}
	}
}
