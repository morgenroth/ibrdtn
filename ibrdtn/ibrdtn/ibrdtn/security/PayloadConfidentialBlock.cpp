/*
 * PayloadConfidentialBlock.cpp
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

#include "ibrdtn/security/PayloadConfidentialBlock.h"
#include "ibrdtn/security/PayloadIntegrityBlock.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/SDNV.h"

#include <openssl/err.h>
#include <openssl/rsa.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>

#include <stdint.h>
#include <typeinfo>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

namespace dtn
{
	namespace security
	{
		dtn::data::Block* PayloadConfidentialBlock::Factory::create()
		{
			return new PayloadConfidentialBlock();
		}

		PayloadConfidentialBlock::PayloadConfidentialBlock()
		: SecurityBlock(PAYLOAD_CONFIDENTIAL_BLOCK, PCB_RSA_AES128_PAYLOAD_PIB_PCB)
		{
		}

		PayloadConfidentialBlock::~PayloadConfidentialBlock()
		{
		}

		void PayloadConfidentialBlock::encrypt(dtn::data::Bundle& bundle, const dtn::security::SecurityKey &long_key, const dtn::data::EID& source)
		{
			// contains the random salt
			uint32_t salt;

			// contains the random key
			unsigned char ephemeral_key[ibrcommon::AES128Stream::key_size_in_bytes];

			unsigned char iv[ibrcommon::AES128Stream::iv_len];
			unsigned char tag[ibrcommon::AES128Stream::tag_len];

			// get all PCBs
			const std::list<const PayloadConfidentialBlock* > pcbs = bundle.getBlocks<PayloadConfidentialBlock>();

			// get all PIBs
			const std::list<const PayloadIntegrityBlock* > pibs = bundle.getBlocks<PayloadIntegrityBlock>();

			// create a new payload confidential block
			PayloadConfidentialBlock& pcb = bundle.push_front<PayloadConfidentialBlock>();

			// create a random salt and key
			createSaltAndKey(salt, ephemeral_key, ibrcommon::AES128Stream::key_size_in_bytes);

			// encrypt payload - BEGIN
			dtn::data::PayloadBlock& plb = bundle.getBlock<dtn::data::PayloadBlock>();
			ibrcommon::BLOB::Reference blobref = plb.getBLOB();

			{
				ibrcommon::BLOB::iostream stream = blobref.iostream();
				ibrcommon::AES128Stream aes_stream(ibrcommon::CipherStream::CIPHER_ENCRYPT, *stream, ephemeral_key, salt);

				// encrypt in place
				((ibrcommon::CipherStream&)aes_stream).encrypt(*stream);

				// check if this is a fragment
				if (bundle.get(dtn::data::PrimaryBlock::FRAGMENT))
				{
					// ... and set the corresponding cipher suit params
					addFragmentRange(pcb._ciphersuite_params, bundle._fragmentoffset, stream.size());
				}

				// get the IV
				aes_stream.getIV(iv);

				// get the tag
				aes_stream.getTag(tag);
			}
			// encrypt payload - END

			// set the source and destination address of the new block
			if (source != bundle._source.getNode()) pcb.setSecuritySource( source );
			if (long_key.reference != bundle._destination.getNode()) pcb.setSecurityDestination( long_key.reference );

			// set replicate in every fragment to true
			pcb.set(REPLICATE_IN_EVERY_FRAGMENT, true);

			// store encypted key, tag, iv and salt
			addSalt(pcb._ciphersuite_params, salt);

			// get the RSA key
			RSA *rsa_key = long_key.getRSA();

			// encrypt the random key and add it to the ciphersuite params
			addKey(pcb._ciphersuite_params, ephemeral_key, ibrcommon::AES128Stream::key_size_in_bytes, rsa_key);

			// free the RSA key
			long_key.free(rsa_key);

			pcb._ciphersuite_params.set(SecurityBlock::initialization_vector, iv, ibrcommon::AES128Stream::iv_len);
			pcb._ciphersuite_flags |= SecurityBlock::CONTAINS_CIPHERSUITE_PARAMS;

			pcb._security_result.set(SecurityBlock::PCB_integrity_check_value, tag, ibrcommon::AES128Stream::tag_len);
			pcb._ciphersuite_flags |= SecurityBlock::CONTAINS_SECURITY_RESULT;

			// create correlator
			uint64_t correlator = createCorrelatorValue(bundle);

			if (pcbs.size() > 0 || pibs.size() > 0)
				pcb.setCorrelator(correlator);

			// encrypt PCBs and PIBs
			for (std::list<const PayloadConfidentialBlock*>::const_iterator it = pcbs.begin(); it != pcbs.end(); it++)
				SecurityBlock::encryptBlock<PayloadConfidentialBlock>(bundle, (dtn::data::Block&)**it, salt, ephemeral_key).setCorrelator(correlator);

			for (std::list<const PayloadIntegrityBlock*>::const_iterator it = pibs.begin(); it != pibs.end(); it++)
				SecurityBlock::encryptBlock<PayloadConfidentialBlock>(bundle, (dtn::data::Block&)**it, salt, ephemeral_key).setCorrelator(correlator);
		}

		void PayloadConfidentialBlock::decrypt(dtn::data::Bundle& bundle, const dtn::security::SecurityKey &long_key)
		{
			// list of block to delete if the process is successful
			std::list<const dtn::data::Block*> erasure_list;
			
			// load the RSA key
			RSA *rsa_key = long_key.getRSA();

			try {
				// array for the current symmetric AES key
				unsigned char key[ibrcommon::AES128Stream::key_size_in_bytes];

				// correlator of the first PCB
				uint64_t correlator = 0;
				bool decrypt_related = false;

				// get all blocks of this bundle
				const std::list<const dtn::data::Block*> blocks = bundle.getBlocks();

				// iterate through all blocks
				for (std::list<const dtn::data::Block* >::const_iterator it = blocks.begin(); it != blocks.end(); it++)
				{
					try {
						dynamic_cast<const PayloadIntegrityBlock&>(**it);

						// add this block to the erasure list for later deletion
						erasure_list.push_back(*it);
					} catch (const std::bad_cast&) { };

					try {
						const PayloadConfidentialBlock &pcb = dynamic_cast<const PayloadConfidentialBlock&>(**it);

						// get salt and key
						uint32_t salt = getSalt(pcb._ciphersuite_params);

						// decrypt related blocks
						if (decrypt_related)
						{
							// try to decrypt the block
							try {
								decryptBlock(bundle, (dtn::security::SecurityBlock&)**it, salt, key);

								// success! add this block to the erasue list
								erasure_list.push_back(*it);
							} catch (const ibrcommon::Exception&) {
								IBRCOMMON_LOGGER(critical) << "tag verfication failed, reversing decryption..." << IBRCOMMON_LOGGER_ENDL;
								decryptBlock(bundle, (dtn::security::SecurityBlock&)**it, salt, key);

								// abort the decryption and discard the bundle?
								throw ibrcommon::Exception("decrypt of correlated block reversed, tag verfication failed");
							}
						}
						// if security destination does match the key, then try to decrypt the payload
						else if (pcb.isSecurityDestination(bundle, long_key.reference) &&
							(pcb._ciphersuite_id == SecurityBlock::PCB_RSA_AES128_PAYLOAD_PIB_PCB))
						{
							// try to decrypt the symmetric AES key
							if (!getKey(pcb._ciphersuite_params, key, ibrcommon::AES128Stream::key_size_in_bytes, rsa_key))
							{
								IBRCOMMON_LOGGER(critical) << "could not get symmetric key decrypted" << IBRCOMMON_LOGGER_ENDL;
								throw ibrcommon::Exception("decrypt failed - could not get symmetric key decrypted");
							}

							// try to decrypt the payload
							if (!decryptPayload(bundle, key, salt))
							{
								// reverse decryption
								IBRCOMMON_LOGGER(critical) << "tag verfication failed, reversing decryption..." << IBRCOMMON_LOGGER_ENDL;
								decryptPayload(bundle, key, salt);
								throw ibrcommon::Exception("decrypt reversed - tag verfication failed");
							}

							// success! add this block to the erasue list
							erasure_list.push_back(*it);

							// check if first PCB has a correlator
							if (pcb._ciphersuite_flags & CONTAINS_CORRELATOR)
							{
								// ... and decrypt all correlated block with the same key
								decrypt_related = true;

								// store the correlator
								correlator = pcb._correlator;
							}
							else
							{
								// no correlated blocks should exists
								// stop here with decryption
								break;
							}
						}
						else
						{
							// exit here, because we can not decrypt the first PCB.
							throw ibrcommon::Exception("unable to decrypt the first PCB");
						}
					} catch (const std::bad_cast&) { };
				}

				// delete all block in the erasure list
				for (std::list<const dtn::data::Block* >::const_iterator it = erasure_list.begin(); it != erasure_list.end(); it++)
				{
					bundle.remove(**it);
				}
			} catch (const std::exception&) {
				long_key.free(rsa_key);
				throw;
			}

			long_key.free(rsa_key);
		}

		bool PayloadConfidentialBlock::decryptPayload(dtn::data::Bundle& bundle, const unsigned char ephemeral_key[ibrcommon::AES128Stream::key_size_in_bytes], const uint32_t salt)
		{
			// TODO handle fragmentation
			PayloadConfidentialBlock& pcb = bundle.getBlock<PayloadConfidentialBlock>();
			dtn::data::PayloadBlock& plb = bundle.getBlock<dtn::data::PayloadBlock>();

			// the array for the extracted iv
			unsigned char iv[ibrcommon::AES128Stream::iv_len];
			pcb._ciphersuite_params.get(SecurityBlock::initialization_vector, iv, ibrcommon::AES128Stream::iv_len);

			// the array for the extracted tag
			unsigned char tag[ibrcommon::AES128Stream::tag_len];
			pcb._security_result.get(SecurityBlock::PCB_integrity_check_value, tag, ibrcommon::AES128Stream::tag_len);

			// get the reference to the corresponding BLOB object
			ibrcommon::BLOB::Reference blobref = plb.getBLOB();

			// decrypt the payload and get the integrity signature (tag)
			{
				ibrcommon::BLOB::iostream stream = blobref.iostream();
				ibrcommon::AES128Stream decrypt(ibrcommon::CipherStream::CIPHER_DECRYPT, *stream, ephemeral_key, salt, iv);
				((ibrcommon::CipherStream&)decrypt).decrypt(*stream);

				// get the decrypt tag
				if (!decrypt.verify(tag))
				{
					IBRCOMMON_LOGGER(error) << "integrity signature of the decrypted payload is invalid" << IBRCOMMON_LOGGER_ENDL;
					return false;
				}
			}

			return true;
		}
	}
}
