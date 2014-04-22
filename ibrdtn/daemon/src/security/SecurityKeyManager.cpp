/*
 * SecurityKeyManager.cpp
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

#include "Configuration.h"
#include "core/BundleCore.h"
#include "security/SecurityKeyManager.h"
#include <ibrdtn/data/DTNTime.h>
#include <ibrcommon/Logger.h>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

namespace dtn
{
	namespace security
	{
		const std::string SecurityKeyManager::TAG = "SecurityKeyManager";

		SecurityKeyManager& SecurityKeyManager::getInstance()
		{
			static SecurityKeyManager instance;
			return instance;
		}

		SecurityKeyManager::SecurityKeyManager()
		{
		}

		SecurityKeyManager::~SecurityKeyManager()
		{
		}

		void SecurityKeyManager::onConfigurationChanged(const dtn::daemon::Configuration &conf) throw ()
		{
			const dtn::daemon::Configuration::Security &sec = conf.getSecurity();

			if (sec.enabled())
			{
				IBRCOMMON_LOGGER_TAG(SecurityKeyManager::TAG, info) << "initialized; path: " << sec.getPath().getPath() << IBRCOMMON_LOGGER_ENDL;

				// store all paths locally
				_path = sec.getPath();
				_key = sec.getKey();
				_ca = sec.getCertificate();

				// check if there is a local key
				if (!dtn::core::BundleCore::local.isNone() && !hasKey(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC))
				{
					IBRCOMMON_LOGGER_TAG(SecurityKeyManager::TAG, info) << "generate a new local key-pair for " << dtn::core::BundleCore::local.getString() << IBRCOMMON_LOGGER_ENDL;

					// generate a new local key
					createRSA(dtn::core::BundleCore::local);
				}
			}
			else
			{
				_path = ibrcommon::File();
				_key = ibrcommon::File();
				_ca = ibrcommon::File();
			}
		}

		const std::string SecurityKeyManager::hash(const dtn::data::EID &eid)
		{
			return hash(eid.getNode().getString());
		}

		const std::string SecurityKeyManager::hash(const std::string &value)
		{
			std::stringstream ss;
			for (std::string::const_iterator iter = value.begin(); iter != value.end(); ++iter)
			{
				unsigned char c = (*iter);
				ss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << (unsigned int)c;
			}
			return ss.str();
		}

		bool SecurityKeyManager::hasKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type) const
		{
			ibrcommon::File keyfile = getKeyFile(ref, type);
			return keyfile.exists();
		}

		void SecurityKeyManager::createRSA(const dtn::data::EID &ref, const int bits)
		{
			const ibrcommon::File keyfile = _path.get(hash(ref.getNode()) + ".pem");
			RSA* rsa = RSA_new();
			BIGNUM* e = BN_new();

			BN_set_word(e, 65537);

			RSA_generate_key_ex(rsa, bits, e, NULL);

			BN_free(e);
			e = NULL;

			FILE * rsa_key_file = fopen(keyfile.getPath().c_str(), "w");

			if (!rsa_key_file) {
				IBRCOMMON_LOGGER_TAG(SecurityKeyManager::TAG, error) << "Failed to open " << _path.getPath() << IBRCOMMON_LOGGER_ENDL;
				RSA_free(rsa);
				return;
			}

			PEM_write_RSA_PUBKEY(rsa_key_file, rsa);
			PEM_write_RSAPrivateKey(rsa_key_file, rsa, NULL, NULL, 0, NULL, NULL);
			fclose(rsa_key_file);

			RSA_free(rsa);

			// set trust-level to high
			SecurityKey key = get(ref, SecurityKey::KEY_PRIVATE);
			key.trustlevel = SecurityKey::HIGH;
			store(key);
		}

		dtn::security::SecurityKey SecurityKeyManager::get(const std::string &prefix, const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type) const throw (SecurityKey::KeyNotFoundException)
		{
			dtn::security::SecurityKey keydata;
			keydata.reference = ref.getNode();
			keydata.type = type;
			keydata.file = getKeyFile(prefix, ref, type);

			// load security key
			load(keydata);

			return keydata;
		}

		dtn::security::SecurityKey SecurityKeyManager::get(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type) const throw (SecurityKey::KeyNotFoundException)
		{
			dtn::security::SecurityKey keydata;
			keydata.reference = ref.getNode();
			keydata.type = type;
			keydata.file = getKeyFile(keydata.reference, type);

			// load security key
			load(keydata);

			return keydata;
		}

		void SecurityKeyManager::load(dtn::security::SecurityKey &keydata) const
		{
			if ((keydata.type == SecurityKey::KEY_SHARED) && !keydata.file.exists())
			{
				// get the default shared key
				ibrcommon::File default_key = dtn::daemon::Configuration::getInstance().getSecurity().getBABDefaultKey();

				if (default_key.exists()) {
					keydata.file = default_key;
				}
			}

			// throw exception if key-file does not exists
			if (!keydata.file.exists())
			{
				std::stringstream ss;
				ss << "Key file for " << keydata.reference.getString() << " (" << keydata.file.getPath() << ") not found";
				throw SecurityKey::KeyNotFoundException(ss.str());
			}

			// load meta-data
			if (keydata.getMetaFilename().exists())
			{
				std::ifstream metastream(keydata.getMetaFilename().getPath().c_str(), std::ios::in);
				metastream >> keydata;
			}
		}

		void SecurityKeyManager::store(const dtn::security::SecurityKey &key)
		{
			std::stringstream ss;
			{
				ifstream stream(key.file.getPath().c_str(), std::iostream::in);
				ss << stream.rdbuf();
			}
			store(key, ss.str());
		}

		void SecurityKeyManager::store(const dtn::security::SecurityKey &key, const std::string &data)
		{
			dtn::security::SecurityKey keydata = key;

			// get the path for the key
			keydata.file = getKeyFile(keydata.reference, keydata.type);

			std::ofstream keystream(keydata.file.getPath().c_str(), std::ios::out | std::ios::trunc);
			keystream << data;
			keystream.close();

			// store meta-data
			std::ofstream metastream(keydata.getMetaFilename().getPath().c_str(), std::ios::out | std::ios::trunc);
			metastream << key;
		}

		void SecurityKeyManager::store(const std::string &prefix, const dtn::security::SecurityKey &key, const std::string &data)
		{
			dtn::security::SecurityKey keydata = key;

			// get the path for the key
			keydata.file = getKeyFile(prefix, keydata.reference, keydata.type);

			std::ofstream keystream(keydata.file.getPath().c_str(), std::ios::out | std::ios::trunc);
			keystream << data;
			keystream.close();

			// store meta-data
			std::ofstream metastream(keydata.getMetaFilename().getPath().c_str(), std::ios::out | std::ios::trunc);
			metastream << key;
		}

		void SecurityKeyManager::remove(const SecurityKey &key)
		{
			// remove key file
			ibrcommon::File keyfile = key.file;
			keyfile.remove();

			// remove meta file
			key.getMetaFilename().remove();
		}

		const ibrcommon::File SecurityKeyManager::getKeyFile(const dtn::data::EID &peer, const dtn::security::SecurityKey::KeyType type) const
		{
			switch (type)
			{
				case SecurityKey::KEY_SHARED:
					return _path.get(hash(peer) + ".mac");

				case SecurityKey::KEY_UNSPEC:
				case SecurityKey::KEY_PUBLIC:
				case SecurityKey::KEY_PRIVATE:
					return _path.get(hash(peer) + ".pem");

				default:
					return _path.get(hash(peer) + ".key");
			}
		}

		const ibrcommon::File SecurityKeyManager::getKeyFile(const std::string &prefix, const dtn::data::EID &peer, const dtn::security::SecurityKey::KeyType type) const
		{
			switch (type)
			{
				case SecurityKey::KEY_SHARED:
					return _path.get(hash(prefix) + "." + hash(peer) + ".mac");

				case SecurityKey::KEY_UNSPEC:
				case SecurityKey::KEY_PUBLIC:
				case SecurityKey::KEY_PRIVATE:
					return _path.get(hash(prefix) + "." + hash(peer) + ".pem");

				default:
					return _path.get(hash(prefix) + "." + hash(peer) + ".key");
			}
		}
	}
}
