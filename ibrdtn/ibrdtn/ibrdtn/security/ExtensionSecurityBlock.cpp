/*
 * ExtensionSecurityBlock.cpp
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

#include "ibrdtn/security/ExtensionSecurityBlock.h"
#include <ibrcommon/Logger.h>
#include "ibrdtn/data/Serializer.h"
#include "ibrdtn/data/Bundle.h"
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <algorithm>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

namespace dtn
{
	namespace security
	{
		const dtn::data::block_t ExtensionSecurityBlock::BLOCK_TYPE = SecurityBlock::EXTENSION_SECURITY_BLOCK;

		dtn::data::Block* ExtensionSecurityBlock::Factory::create()
		{
			return new ExtensionSecurityBlock();
		}

		ExtensionSecurityBlock::ExtensionSecurityBlock()
		 : SecurityBlock(EXTENSION_SECURITY_BLOCK, ESB_RSA_AES128_EXT)
		{
		}

		ExtensionSecurityBlock::~ExtensionSecurityBlock()
		{
		}

		void ExtensionSecurityBlock::encrypt(dtn::data::Bundle& bundle, const SecurityKey &key, dtn::data::Bundle::iterator it, const dtn::data::EID& source, const dtn::data::EID& destination)
		{
			uint32_t salt = 0;

			// load the rsa key
			RSA *rsa_key = key.getRSA();

			// key used for encrypting the block. the key will be encrypted using RSA
			unsigned char ephemeral_key[ibrcommon::AES128Stream::key_size_in_bytes];
			createSaltAndKey(salt, ephemeral_key, ibrcommon::AES128Stream::key_size_in_bytes);

			dtn::security::ExtensionSecurityBlock& esb = SecurityBlock::encryptBlock<ExtensionSecurityBlock>(bundle, it, salt, ephemeral_key);

			// set the source and destination address of the new block
			if (source != bundle.source) esb.setSecuritySource( source );
			if (destination != bundle.destination) esb.setSecurityDestination( destination );

			// encrypt the ephemeral key and place it in _ciphersuite_params
			addSalt(esb._ciphersuite_params, salt);
			addKey(esb._ciphersuite_params, ephemeral_key, ibrcommon::AES128Stream::key_size_in_bytes, rsa_key);
			esb._ciphersuite_flags |= CONTAINS_CIPHERSUITE_PARAMS;

			// free the rsa key
			key.free(rsa_key);
		}

		void ExtensionSecurityBlock::decrypt(dtn::data::Bundle& bundle, const SecurityKey &key, dtn::data::Bundle::iterator it)
		{
			const dtn::security::ExtensionSecurityBlock& block = dynamic_cast<const dtn::security::ExtensionSecurityBlock&>(**it);

			// load the rsa key
			RSA *rsa_key = key.getRSA();

			// get key, convert with reinterpret_cast
			unsigned char keydata[ibrcommon::AES128Stream::key_size_in_bytes];

			if (!getKey(block._ciphersuite_params, keydata, ibrcommon::AES128Stream::key_size_in_bytes, rsa_key))
			{
				IBRCOMMON_LOGGER_ex(critical) << "could not get symmetric key decrypted" << IBRCOMMON_LOGGER_ENDL;
				throw ibrcommon::Exception("could not extract the key");
			}

			// get salt, convert with stringstream
			uint32_t salt = getSalt(block._ciphersuite_params);

			SecurityBlock::decryptBlock(bundle, it, salt, keydata);
		}

		void ExtensionSecurityBlock::decrypt(dtn::data::Bundle& bundle, const SecurityKey &key, const dtn::data::Number &correlator)
		{
			// iterate through all extension security blocks
			dtn::data::Bundle::find_iterator find_it(bundle.begin(), ExtensionSecurityBlock::BLOCK_TYPE);
			while (find_it.next(bundle.end()))
			{
				const dtn::security::ExtensionSecurityBlock &esb = dynamic_cast<const dtn::security::ExtensionSecurityBlock&>(**find_it);

				if ((correlator == 0) || (correlator == esb._correlator))
				{
					decrypt(bundle, key, find_it);
				}
			}
		}
	}
}
