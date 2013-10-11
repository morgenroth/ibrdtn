/*
 * PayloadIntegrityBlock.h
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

#ifndef _PAYLOAD_INTEGRITY_BLOCK_H_
#define _PAYLOAD_INTEGRITY_BLOCK_H_

#include "ibrdtn/security/SecurityBlock.h"
#include "ibrdtn/security/SecurityKey.h"
#include "ibrdtn/data/ExtensionBlock.h"
#include "ibrdtn/data/Bundle.h"
#include <openssl/evp.h>

namespace dtn
{
	namespace security
	{
		/**
		Signs bundles for connections of security aware nodes. A factory with a rsa 
		key can be created for signing or verifieing the bundle. From the bundle the
		primary block, the payload block, PayloadIntegrityBlock and the 
		PayloadConfidentialBlock will be covered by the signature.\n
		A sign can be added using the addHash()-Method. Verification can be done via
		one of the verify()-Methods.
		*/
		class PayloadIntegrityBlock : public SecurityBlock
		{
			friend class dtn::data::Bundle;
			public:
				class Factory : public dtn::data::ExtensionBlock::Factory
				{
				public:
					Factory() : dtn::data::ExtensionBlock::Factory(PayloadIntegrityBlock::BLOCK_TYPE) {};
					virtual ~Factory() {};
					virtual dtn::data::Block* create();
				};

				/** The block type of this class. */
				static const dtn::data::block_t BLOCK_TYPE;
				
				/** frees the internal PKEY object, without deleting the rsa object 
				given in the constructor */
				virtual ~PayloadIntegrityBlock();

				/**
				Takes a bundle and adds a PayloadIntegrityBlock to it using the key
				given in the constructor after the primary block.
				@param bundle the bundle to be hashed and signed
				*/
				static void sign(dtn::data::Bundle &bundle, const SecurityKey &key, const dtn::data::EID& destination);

				/**
				Tests if the bundles signatures is correct. There might be multiple PIBs
				inside the bundle, which may be tested and the result will be 1 if one 
				matches.
				@param bundle the bundle to be checked
				@return -1 if an error occured, 0 if the signature does not match, 1 if
				the signature matches
				*/
				static void verify(const dtn::data::Bundle &bundle, const SecurityKey &key);

				/**
				Removes all PayloadIntegrityBlocks from a bundle
				@param bundle the bundle, which shall be cleaned from pibs
				*/
				static void strip(dtn::data::Bundle& bundle);

				/**
				Parses the PayloadIntegrityBlock from a Stream
				@param stream the stream to read from
				*/
				virtual std::istream &deserialize(std::istream &stream, const dtn::data::Length &length);

			protected:
				/**
				Constructs an empty PayloadIntegrityBlock in order for adding it to a 
				bundle and sets its ciphersuite id to PIB_RSA_SHA256.
				*/
				PayloadIntegrityBlock();
				
				/**
				Returns the size of the security result field. This is used for strict
				canonicalisation, where the block itself is included to the canonical
				form, but the security result is excluded or unknown.
				*/
				virtual dtn::data::Length getSecurityResultSize() const;

			private:
				/** If the PIB does not know the size of the sign in advance, this
				member can be set, so the size of the security result can be calculated
				correctly */
				dtn::data::Length result_size;

				/**
				Calculates a signature using the PIB-RSA-SHA256 algorithm.
				@param bundle the bundle to be hashed
				@param ignore the security block, wichs security result shall be ignored
				@return a string with the signature
				*/
				static const std::string calcHash(const dtn::data::Bundle &bundle, const SecurityKey &key, PayloadIntegrityBlock& ignore);

				/**
				Set key_size to new_size, when _security_result is empty at the mutable
				canonicialization process and no RSA object is set to calculate the size
				of _security_result.
				@param new_size the new size of the sign, which will be stored in
				_security_result
				*/
				void setResultSize(const SecurityKey &key);
		};

		/**
		 * This creates a static block factory
		 */
		static PayloadIntegrityBlock::Factory __PayloadIntegrityBlockFactory__;
	}
}
#endif /* _PAYLOAD_INTEGRITY_BLOCK_H_ */
