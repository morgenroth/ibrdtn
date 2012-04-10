#include "security/SecurityManager.h"
#include "security/SecurityKeyManager.h"
#include "core/BundleCore.h"
#include "routing/QueueBundleEvent.h"
#include <ibrcommon/Logger.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

namespace dtn
{
	namespace security
	{
		SecurityManager& SecurityManager::getInstance()
		{
			static SecurityManager sec_man;
			return sec_man;
		}

		SecurityManager::SecurityManager()
		: _accept_only_bab(false), _accept_only_pib(false)
		{
		}

		SecurityManager::~SecurityManager()
		{
		}

		void SecurityManager::auth(dtn::data::Bundle &bundle) const throw (KeyMissingException)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "auth bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

			try {
				// try to load the local key
				const SecurityKey key = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_SHARED);

				// sign the bundle with BABs
				dtn::security::BundleAuthenticationBlock::auth(bundle, key);
			} catch (const SecurityKeyManager::KeyNotFoundException &ex) {
				throw KeyMissingException(ex.what());
			}
		}

		void SecurityManager::sign(dtn::data::Bundle &bundle) const throw (KeyMissingException)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "sign bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

			try {
				// try to load the local key
				const SecurityKey key = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PRIVATE);

				// sign the bundle with PIB
				dtn::security::PayloadIntegrityBlock::sign(bundle, key, bundle._destination.getNode());
			} catch (const SecurityKeyManager::KeyNotFoundException &ex) {
				throw KeyMissingException(ex.what());
			}
		}

		void SecurityManager::prefetchKey(const dtn::data::EID &eid)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "prefetch key for: " << eid.getString() << IBRCOMMON_LOGGER_ENDL;

			// prefetch the key for this EID
			SecurityKeyManager::getInstance().prefetchKey(eid, SecurityKey::KEY_PUBLIC);
		}

		void SecurityManager::verify(dtn::data::Bundle &bundle) const throw (VerificationFailedException)
		{
			verifyBAB(bundle);
			verifyPIB(bundle);
		}

		void SecurityManager::verifyPIB(dtn::data::Bundle &bundle) const throw (VerificationFailedException)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "verify signed bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

			// get all PIBs of this bundle
			std::list<const dtn::security::PayloadIntegrityBlock*> pibs = bundle.getBlocks<dtn::security::PayloadIntegrityBlock>();

			for (std::list<const dtn::security::PayloadIntegrityBlock*>::iterator it = pibs.begin(); it != pibs.end(); it++)
			{
				const dtn::security::PayloadIntegrityBlock& pib = (**it);

				try {
					const SecurityKey key = SecurityKeyManager::getInstance().get(pib.getSecuritySource(bundle), SecurityKey::KEY_PUBLIC);

					if (pib.isSecurityDestination(bundle, dtn::core::BundleCore::local))
					{
						try {
							dtn::security::PayloadIntegrityBlock::strip(bundle, key);

							// set the verify bit, after verification
							bundle.set(dtn::data::Bundle::DTNSEC_STATUS_VERIFIED, true);

							IBRCOMMON_LOGGER_DEBUG(5) << "Bundle from " << bundle._source.getString() << " successfully verified using PayloadIntegrityBlock" << IBRCOMMON_LOGGER_ENDL;
							return;
						} catch (const ibrcommon::Exception&) {
							throw VerificationFailedException();
						}
					}
					else
					{
						try {
							dtn::security::PayloadIntegrityBlock::verify(bundle, key);

							// set the verify bit, after verification
							bundle.set(dtn::data::Bundle::DTNSEC_STATUS_VERIFIED, true);

							IBRCOMMON_LOGGER_DEBUG(5) << "Bundle from " << bundle._source.getString() << " successfully verified using PayloadIntegrityBlock" << IBRCOMMON_LOGGER_ENDL;
						} catch (const ibrcommon::Exception&) {
							throw VerificationFailedException();
						}
					}
				} catch (const ibrcommon::Exception&) {
					// key not found?
				}
			}
		}

		void SecurityManager::verifyBAB(dtn::data::Bundle &bundle) const throw (VerificationFailedException)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "verify authenticated bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

			// get all BABs of this bundle
			std::list <const dtn::security::BundleAuthenticationBlock* > babs = bundle.getBlocks<dtn::security::BundleAuthenticationBlock>();

			for (std::list <const dtn::security::BundleAuthenticationBlock* >::iterator it = babs.begin(); it != babs.end(); it++)
			{
				const dtn::security::BundleAuthenticationBlock& bab = (**it);

				// look for the right BAB-factory
				const dtn::data::EID node = bab.getSecuritySource(bundle);

				try {
					// try to load the key of the BAB
					const SecurityKey key = SecurityKeyManager::getInstance().get(node, SecurityKey::KEY_SHARED);

					// verify the bundle
					dtn::security::BundleAuthenticationBlock::verify(bundle, key);

					// strip all BAB of this bundle
					dtn::security::BundleAuthenticationBlock::strip(bundle);

					// set the verify bit, after verification
					bundle.set(dtn::data::Bundle::DTNSEC_STATUS_AUTHENTICATED, true);

					// at least one BAB has been authenticated, we're done!
					break;
				} catch (const SecurityKeyManager::KeyNotFoundException&) {
					// no key for this node found
				} catch (const ibrcommon::Exception &ex) {
					// verification failed
					throw SecurityManager::VerificationFailedException(ex.what());
				}
			}
		}

		void SecurityManager::fastverify(const dtn::data::Bundle &bundle) const throw (VerificationFailedException)
		{
			// do a fast verify without manipulating the bundle
			const dtn::daemon::Configuration::Security &secconf = dtn::daemon::Configuration::getInstance().getSecurity();

			if (secconf.getLevel() & dtn::daemon::Configuration::Security::SECURITY_LEVEL_ENCRYPTED)
			{
				// check if the bundle is encrypted and throw an exception if not
				//throw VerificationFailedException("Bundle is not encrypted");
				IBRCOMMON_LOGGER_DEBUG(10) << "encryption required, verify bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

				const std::list<const dtn::security::PayloadConfidentialBlock* > pcbs = bundle.getBlocks<dtn::security::PayloadConfidentialBlock>();
				if (pcbs.size() == 0) throw VerificationFailedException("No PCB available!");
			}

			if (secconf.getLevel() & dtn::daemon::Configuration::Security::SECURITY_LEVEL_AUTHENTICATED)
			{
				// check if the bundle is signed and throw an exception if not
				//throw VerificationFailedException("Bundle is not signed");
				IBRCOMMON_LOGGER_DEBUG(10) << "authentication required, verify bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

				const std::list<const dtn::security::BundleAuthenticationBlock* > babs = bundle.getBlocks<dtn::security::BundleAuthenticationBlock>();
				if (babs.size() == 0) throw VerificationFailedException("No BAB available!");
			}
		}

		void SecurityManager::decrypt(dtn::data::Bundle &bundle) const throw (DecryptException, KeyMissingException)
		{
			// check if the bundle has to be decrypted, return when not
			if (bundle.getBlocks<dtn::security::PayloadConfidentialBlock>().size() <= 0) return;

			// decrypt
			try {
				IBRCOMMON_LOGGER_DEBUG(10) << "decrypt bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

				// get the encryption key
				dtn::security::SecurityKey key = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, dtn::security::SecurityKey::KEY_PRIVATE);

				// encrypt the payload of the bundle
				dtn::security::PayloadConfidentialBlock::decrypt(bundle, key);

				bundle.set(dtn::data::Bundle::DTNSEC_STATUS_CONFIDENTIAL, true);
			} catch (const ibrcommon::Exception &ex) {
				throw DecryptException(ex.what());
			}
		}

		void SecurityManager::encrypt(dtn::data::Bundle &bundle) const throw (EncryptException, KeyMissingException)
		{
			try {
				IBRCOMMON_LOGGER_DEBUG(10) << "encrypt bundle: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

				// get the encryption key
				dtn::security::SecurityKey key = SecurityKeyManager::getInstance().get(bundle._destination, dtn::security::SecurityKey::KEY_PUBLIC);

				// encrypt the payload of the bundle
				dtn::security::PayloadConfidentialBlock::encrypt(bundle, key, dtn::core::BundleCore::local);
			} catch (const ibrcommon::Exception &ex) {
				throw EncryptException(ex.what());
			}
		}
	}
}
