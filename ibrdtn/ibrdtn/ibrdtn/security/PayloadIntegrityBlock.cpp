/*
 * PayloadIntegrityBlock.cpp
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

#include "ibrdtn/security/PayloadIntegrityBlock.h"
#include "ibrdtn/security/MutualSerializer.h"
#include "ibrdtn/data/Bundle.h"

#include <ibrcommon/ssl/RSASHA256Stream.h>
#include <ibrcommon/Logger.h>
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
		const dtn::data::block_t PayloadIntegrityBlock::BLOCK_TYPE = SecurityBlock::PAYLOAD_INTEGRITY_BLOCK;

		dtn::data::Block* PayloadIntegrityBlock::Factory::create()
		{
			return new PayloadIntegrityBlock();
		}

		PayloadIntegrityBlock::PayloadIntegrityBlock()
		 : SecurityBlock(PAYLOAD_INTEGRITY_BLOCK, PIB_RSA_SHA256), result_size(0)
		{
		}

		PayloadIntegrityBlock::~PayloadIntegrityBlock()
		{
		}

		dtn::data::Length PayloadIntegrityBlock::getSecurityResultSize() const
		{
			if (result_size > 0)
			{
				return result_size;
			}

			return SecurityBlock::getSecurityResultSize();
		}

		void PayloadIntegrityBlock::sign(dtn::data::Bundle &bundle, const SecurityKey &key, const dtn::data::EID& destination)
		{
			PayloadIntegrityBlock& pib = bundle.push_front<PayloadIntegrityBlock>();
			pib.set(REPLICATE_IN_EVERY_FRAGMENT, true);

			// check if this is a fragment
			if (bundle.get(dtn::data::PrimaryBlock::FRAGMENT))
			{
				dtn::data::PayloadBlock& plb = bundle.find<dtn::data::PayloadBlock>();
				ibrcommon::BLOB::Reference blobref = plb.getBLOB();
				ibrcommon::BLOB::iostream stream = blobref.iostream();
				addFragmentRange(pib._ciphersuite_params, bundle.fragmentoffset, stream.size());
			}

			// set the source and destination address of the new block
			if (!key.reference.sameHost(bundle.source)) pib.setSecuritySource( key.reference );
			if (!destination.sameHost(bundle.destination)) pib.setSecurityDestination( destination );

			pib.setResultSize(key);
			pib.setCiphersuiteId(SecurityBlock::PIB_RSA_SHA256);
			pib._ciphersuite_flags |= CONTAINS_SECURITY_RESULT;
			std::string sign = calcHash(bundle, key, pib);
			pib._security_result.set(SecurityBlock::integrity_signature, sign);
		}

		const std::string PayloadIntegrityBlock::calcHash(const dtn::data::Bundle &bundle, const SecurityKey &key, PayloadIntegrityBlock& ignore)
		{
			EVP_PKEY *pkey = key.getEVP();
			ibrcommon::RSASHA256Stream rs2s(pkey);

			// serialize the bundle in the mutable form
			dtn::security::MutualSerializer ms(rs2s, &ignore);
			(dtn::data::DefaultSerializer&)ms << bundle; rs2s << std::flush;

			int return_code = rs2s.getSign().first;
			std::string sign_string = rs2s.getSign().second;
			SecurityKey::free(pkey);

			if (return_code)
				return sign_string;
			else
			{
				IBRCOMMON_LOGGER_ex(critical) << "an error occured at the creation of the hash and it is invalid" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
				return std::string("");
			}
		}

		void PayloadIntegrityBlock::verify(const dtn::data::Bundle& bundle, const SecurityKey &key, const PayloadIntegrityBlock &sb, const bool use_eid)
		{
			// check if we have the public key of the security source
			if (use_eid)
			{
				if (!sb.isSecuritySource(bundle, key.reference))
				{
					throw VerificationFailedException("key not match the security source");
				}
			}

			// check the correct algorithm
			if (sb._ciphersuite_id != SecurityBlock::PIB_RSA_SHA256)
			{
				throw VerificationFailedException("can not verify the PIB because of an invalid algorithm");
			}

			EVP_PKEY *pkey = key.getEVP();
			if (pkey == NULL) throw VerificationFailedException("verification error");

			ibrcommon::RSASHA256Stream rs2s(pkey, true);

			// serialize the bundle in the mutable form
			dtn::security::MutualSerializer ms(rs2s, &sb);
			(dtn::data::DefaultSerializer&)ms << bundle; rs2s << std::flush;

			int ret = rs2s.getVerification(sb._security_result.get(SecurityBlock::integrity_signature));
			SecurityKey::free(pkey);

			if (ret == 0)
			{
				throw VerificationFailedException("verification failed");
			}
			else if (ret < 0)
			{
				throw VerificationFailedException("verification error");
			}
		}

		void PayloadIntegrityBlock::verify(const dtn::data::Bundle &bundle, const SecurityKey &key)
		{
			// iterate over all PIBs to find the right one
			dtn::data::Bundle::const_find_iterator it(bundle.begin(), PayloadIntegrityBlock::BLOCK_TYPE);

			while (it.next(bundle.end()))
			{
				verify(bundle, key, dynamic_cast<const PayloadIntegrityBlock&>(**it));
			}
		}

		void PayloadIntegrityBlock::setResultSize(const SecurityKey &key)
		{
			EVP_PKEY *pkey = key.getEVP();

			// size of integrity_signature
			if ((result_size = EVP_PKEY_size(pkey)) > 0)
			{
				// sdnv length
				result_size += dtn::data::Number(result_size).getLength();

				// type
				result_size++;
			}
			else
			{
				result_size = _security_result.getLength();
			}

			SecurityKey::free(pkey);
		}

		void PayloadIntegrityBlock::strip(dtn::data::Bundle& bundle, const SecurityKey &key, const bool all)
		{
			dtn::data::Bundle::find_iterator it(bundle.begin(), PayloadIntegrityBlock::BLOCK_TYPE);

			// search for valid PIB
			while (it.next(bundle.end()))
			{
				const PayloadIntegrityBlock &pib = dynamic_cast<const PayloadIntegrityBlock&>(**it);

				// check if the PIB is valid
				try {
					verify(bundle, key, pib);

					// found an valid PIB, remove it
					bundle.remove(pib);

					// remove all previous pibs if all = true
					if (all)
					{
						bundle.erase(std::remove(bundle.begin(), (dtn::data::Bundle::iterator&)it, PayloadIntegrityBlock::BLOCK_TYPE), bundle.end());
					}

					return;
				} catch (const ibrcommon::Exception&) { };
			}
		}

		void PayloadIntegrityBlock::strip(dtn::data::Bundle& bundle)
		{
			bundle.erase(std::remove(bundle.begin(), bundle.end(), PayloadIntegrityBlock::BLOCK_TYPE), bundle.end());
		}

		std::istream& PayloadIntegrityBlock::deserialize(std::istream &stream, const dtn::data::Length &length)
		{
			// deserialize the SecurityBlock
			SecurityBlock::deserialize(stream, length);

			// set the key size locally
			result_size = _security_result.getLength();

			return stream;
		}
	}
}
