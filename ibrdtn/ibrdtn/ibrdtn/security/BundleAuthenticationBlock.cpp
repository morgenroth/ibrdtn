#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/security/BundleAuthenticationBlock.h"
#include "ibrdtn/security/StrictSerializer.h"
#include "ibrcommon/ssl/HMacStream.h"
#include <ibrcommon/Logger.h>
#include <cstring>
#include <set>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

namespace dtn
{
	namespace security
	{
		dtn::data::Block* BundleAuthenticationBlock::Factory::create()
		{
			return new BundleAuthenticationBlock();
		}

		BundleAuthenticationBlock::BundleAuthenticationBlock()
		 : SecurityBlock(BUNDLE_AUTHENTICATION_BLOCK, BAB_HMAC)
		{
		}

		BundleAuthenticationBlock::~BundleAuthenticationBlock()
		{
		}

		void BundleAuthenticationBlock::auth(dtn::data::Bundle &bundle, const dtn::security::SecurityKey &key)
		{
			BundleAuthenticationBlock& bab_begin = bundle.push_front<BundleAuthenticationBlock>();
			bab_begin.set(dtn::data::Block::DISCARD_IF_NOT_PROCESSED, true);

			// set security source
			if (key.reference != bundle._source.getNode()) bab_begin.setSecuritySource( key.reference );

			u_int64_t correlator = createCorrelatorValue(bundle);
			bab_begin.setCorrelator(correlator);
			bab_begin.setCiphersuiteId(BAB_HMAC);

			BundleAuthenticationBlock& bab_end = bundle.push_back<BundleAuthenticationBlock>();
			bab_end.set(dtn::data::Block::DISCARD_IF_NOT_PROCESSED, true);

			bab_end.setCorrelator(correlator);
			bab_end._ciphersuite_flags |= CONTAINS_SECURITY_RESULT;

			std::string sizehash_hash = calcMAC(bundle, key);
			bab_end._security_result.set(SecurityBlock::integrity_signature, sizehash_hash);
		}

		void BundleAuthenticationBlock::verify(const dtn::data::Bundle &bundle, const dtn::security::SecurityKey &key) throw (ibrcommon::Exception)
		{
			// store the correlator of the verified BABs
			u_int64_t correlator;

			// verify the babs of the bundle
			verify(bundle, key, correlator);
		}

		void BundleAuthenticationBlock::strip(dtn::data::Bundle &bundle, const dtn::security::SecurityKey &key)
		{
			// store the correlator of the verified BABs
			u_int64_t correlator;

			// verify the babs of the bundle
			verify(bundle, key, correlator);

			// get the list of BABs
			const std::list<const BundleAuthenticationBlock *> babs = bundle.getBlocks<BundleAuthenticationBlock>();

			for (std::list<const BundleAuthenticationBlock *>::const_iterator it = babs.begin(); it != babs.end(); it++)
			{
				const BundleAuthenticationBlock &bab = (**it);

				// if the correlator is already authenticated, then remove the BAB
				if ((bab._ciphersuite_flags & SecurityBlock::CONTAINS_CORRELATOR) && (bab._correlator == correlator))
				{
					bundle.remove(bab);
				}
			}
		}

		void BundleAuthenticationBlock::strip(dtn::data::Bundle& bundle)
		{
			// blocks of a certain type
			const std::list<const BundleAuthenticationBlock *> babs = bundle.getBlocks<BundleAuthenticationBlock>();

			for (std::list<const BundleAuthenticationBlock *>::const_iterator it = babs.begin(); it != babs.end(); it++)
			{
				bundle.remove(*(*it));
			}
		}

		void BundleAuthenticationBlock::verify(const dtn::data::Bundle& bundle, const dtn::security::SecurityKey &key, u_int64_t &correlator) throw (ibrcommon::Exception)
		{
			std::list<const BundleAuthenticationBlock *> babs = bundle.getBlocks<BundleAuthenticationBlock>();

			// get the blocks, with which the key should match
			std::set<u_int64_t> correlators;

			// calculate the MAC of this bundle
			std::string our_hash_string = calcMAC(bundle, key);

			for (std::list<const BundleAuthenticationBlock *>::const_iterator it = babs.begin(); it != babs.end(); it++)
			{
				const BundleAuthenticationBlock &bab = (**it);

				// the bab contains a security result
				if (bab._ciphersuite_flags & CONTAINS_SECURITY_RESULT)
				{
					// is this correlator known?
					if (correlators.find(bab._correlator) == correlators.end()) continue;

					std::string bab_result = bab._security_result.get(SecurityBlock::integrity_signature);
					if (our_hash_string == bab_result)
					{
						// hash matched
						correlator = bab._correlator;
						return;
					}

					IBRCOMMON_LOGGER_DEBUG(15) << "security mac does not match" << IBRCOMMON_LOGGER_ENDL;
				}
				// bab contains no security result but a correlator
				else if (bab._ciphersuite_flags &  CONTAINS_CORRELATOR)
				{
					// currently we only support BAB_HMAC mechanism
					if (bab._ciphersuite_id != SecurityBlock::BAB_HMAC) continue;

					// skip this BAB if the security source do not match the key
					if (!bab.isSecuritySource(bundle, key.reference)) continue;

					// remember it for later check
					correlators.insert(bab._correlator);
				}
			}

			throw ibrcommon::Exception("verification failed");
		}

		std::string BundleAuthenticationBlock::calcMAC(const dtn::data::Bundle& bundle, const dtn::security::SecurityKey &key, const bool with_correlator, const u_int64_t correlator)
		{
			std::string hmac_key = key.getData();
			ibrcommon::HMacStream hms((const unsigned char*)hmac_key.c_str(), hmac_key.length());
			dtn::security::StrictSerializer ss(hms, BUNDLE_AUTHENTICATION_BLOCK, with_correlator, correlator);
			(dtn::data::DefaultSerializer&)ss << bundle;
			hms << std::flush;

			return ibrcommon::HashStream::extract(hms);
		}

		size_t BundleAuthenticationBlock::getSecurityResultSize() const
		{
			// TLV type
			size_t size = 1;
			// length of value length
			size += dtn::data::SDNV::getLength(EVP_MD_size(EVP_sha1()));
			// length of value
			size += EVP_MD_size(EVP_sha1());
			return size;
		}

	}
}
