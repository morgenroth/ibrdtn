/*
 * PayloadConfidentialBlock.h
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

#ifndef _PAYLOAD_CONFIDENTIAL_BLOCK_H_
#define _PAYLOAD_CONFIDENTIAL_BLOCK_H_
#include "ibrdtn/security/SecurityBlock.h"
#include "ibrdtn/security/SecurityKey.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/ExtensionBlock.h"

namespace dtn
{
	namespace security
	{
		/**
		The PayloadConfidentialBlock encrypts the payload, PayloadConfidentialBlocks,
		which are already there and PayloadIntegrityBlocks, which are already there.
		Payload Confidential or Integrity Blocks are encrypted because they can 
		contain e.g. signatures which make guessing the plaintext easier. You can 
		instantiate a factory, which will take care of everything. The factory can 
		be given a rsa key and the corresponding node. You may wish to add more keys
		using addDestination(), so one or more nodes are able to recover the 
		payload. For each destination a PayloadConfidentialBlock is placed in the
		bundle, when calling encrypt(). Be sure, that no other 
		PayloadConfidentialBlocks or PayloadIntegrityBlocks are inside this bundle 
		if using encryption with more than one key.
		*/
		class PayloadConfidentialBlock : public SecurityBlock
		{
			/**
			This class is allowed to call the parameterless contructor and the 
			constructor with a bundle as argument.
			*/
			friend class dtn::data::Bundle;
			public:
				class Factory : public dtn::data::ExtensionBlock::Factory
				{
				public:
					Factory() : dtn::data::ExtensionBlock::Factory(PayloadConfidentialBlock::BLOCK_TYPE) {};
					virtual ~Factory() {};
					virtual dtn::data::Block* create();
				};

				/** The block type of this class. */
				static const dtn::data::block_t BLOCK_TYPE;

				/** does nothing */
				virtual ~PayloadConfidentialBlock();

				/**
				Encrypts the Payload inside this Bundle. If PIBs or PCBs are found, they
				will be encrypted, too, with a correlator set. The encrypted blocks will
				be each placed inside a PayloadConfidentialBlock, which will be inserted
				at the same place, except for the payload, which be encrypted in place.
				@param bundle the bundle with the to be encrypted payload
				*/
				static void encrypt(dtn::data::Bundle& bundle, const dtn::security::SecurityKey &long_key, const dtn::data::EID& source);

				/**
				Decrypts the Payload inside this Bundle. All correlated Blocks, which
				are found, will be decrypted, too, placed at the position, where their 
				PayloadConfidentialBlock was, which contained them. After a matching 
				PayloadConfidentialBlock with key information is searched by looking 
				after the security destination. If the payload has been decrypted 
				successfully, the correlated blocks will be decrypted. If one block 
				fails to decrypt, it will be deleted.
				@param bundle the bundle with the to be decrypted payload
				@return true if decryption has been successfull, false otherwise
				*/
				static void decrypt(dtn::data::Bundle& bundle, const dtn::security::SecurityKey &long_key);

			protected:
				/**
				Creates an empty PayloadConfidentialBlock. With ciphersuite_id set to
				PCB_RSA_AES128_PAYLOAD_PIB_PCB
				*/
				PayloadConfidentialBlock();

				/**
				Decrypts the payload using the ephemeral_key and salt.
				@param bundle the payload containing bundle
				@param ephemeral_key the AES key
				@param salt the salt
				@return true if tag verification succeeded, false otherwise
				*/
				static bool decryptPayload(dtn::data::Bundle& bundle, const unsigned char ephemeral_key[ibrcommon::AES128Stream::key_size_in_bytes], const uint32_t salt);
		};

		/**
		 * This creates a static block factory
		 */
		static PayloadConfidentialBlock::Factory __PayloadConfidentialBlockFactory__;
	}
}

#endif
