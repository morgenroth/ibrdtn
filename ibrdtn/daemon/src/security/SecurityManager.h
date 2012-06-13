/*
 * SecurityManager.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef _SECURITY_MANAGER_H_
#define _SECURITY_MANAGER_H_

#include "Configuration.h"
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/security/BundleAuthenticationBlock.h>
#include <ibrdtn/security/PayloadIntegrityBlock.h>
#include <ibrdtn/security/PayloadConfidentialBlock.h>
#include <ibrdtn/security/ExtensionSecurityBlock.h>
#include <map>

namespace dtn
{
	namespace security
	{
		/**
		 * Decrypts or encrypts and signs or verifies bundles, which go in or out.
		 * The rules are read from the configuration and the keys needed for
		 * operation must be in the same directory as the configuration or be
		 * retrievable from the net.
		*/
		class SecurityManager
		{
			public:
				class KeyMissingException : public ibrcommon::Exception
				{
				public:
					KeyMissingException(std::string what = "Key for this operation is not available.") : ibrcommon::Exception(what)
					{};

					virtual ~KeyMissingException() throw() {};
				};

				class EncryptException : public ibrcommon::Exception
				{
				public:
					EncryptException(std::string what = "Encryption failed.") : ibrcommon::Exception(what)
					{};

					virtual ~EncryptException() throw() {};
				};

				class DecryptException : public ibrcommon::Exception
				{
				public:
					DecryptException(std::string what = "Decryption failed.") : ibrcommon::Exception(what)
					{};

					virtual ~DecryptException() throw() {};
				};

				class VerificationFailedException : public ibrcommon::Exception
				{
				public:
					VerificationFailedException(std::string what = "Verification failed.") : ibrcommon::Exception(what)
					{};

					virtual ~VerificationFailedException() throw() {};
				};

				/**
				 * Returns a singleton instance of this class.
				 * @return a reference to this class singleton
				 */
				static SecurityManager& getInstance();

				/**
				 * This method signs the bundle with the own private key. If no key is
				 * available a KeyMissingException is thrown.
				 * @param bundle A bundle to sign.
				 */
				void sign(dtn::data::Bundle &bundle) const throw (KeyMissingException);
				void auth(dtn::data::Bundle &bundle) const throw (KeyMissingException);

				/**
				 * This method should be called early as possible. It triggers a procedure to
				 * prepare the public key of some EID for later usage.
				 * @param eid The EID of the owner of the requested public key.
				 */
				void prefetchKey(const dtn::data::EID &eid);

				/**
				 * This method verifies the bundle and removes all auth or integrity block
				 * if they could validated.
				 * @param bundle The bundle to verify.
				 */
				void verify(dtn::data::Bundle &bundle) const throw (VerificationFailedException);
				void verifyBAB(dtn::data::Bundle &bundle) const throw (VerificationFailedException);
				void verifyPIB(dtn::data::Bundle &bundle) const throw (VerificationFailedException);

				/**
				 * This method do a fast verify with the bundle. It do not change anything in it.
				 * A missing key should not lead to an exception, because this method is
				 * called on each received and we need to support multihop without key
				 * knowledge too.
				 * @param bundle The bundle to verify.
				 */
				void fastverify(const dtn::data::Bundle &bundle) const throw (VerificationFailedException);

				/**
				 * This method decrypts encrypted payload of a bundle. It is necessary to
				 * remove all integrity or auth block before the payload can decrypted.
				 * @param bundle
				 */
				void decrypt(dtn::data::Bundle &bundle) const throw (DecryptException, KeyMissingException);

				/**
				 * This method encrypts the payload of a given bundle.
				 * If the bundle already contains integrity or auth block a EcryptException
				 * is thrown.
				 * @param bundle
				 */
				void encrypt(dtn::data::Bundle &bundle) const throw (EncryptException, KeyMissingException);

			protected:
				/**
				need a list of nodes, their security blocks type and the key
				for private and public keys
				*/
				SecurityManager();

				virtual ~SecurityManager();

			private:
				bool _accept_only_bab;
				bool _accept_only_pib;
		};
	}
}

#endif /* _SECURITY_MANAGER_H_ */
