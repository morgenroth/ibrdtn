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
					IBRCOMMON_LOGGER_TAG(SecurityKeyManager::TAG, info) << "generate a new local key-pair" << IBRCOMMON_LOGGER_ENDL;

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
			std::string value = eid.getNode().getString();
			std::stringstream ss;
			for (std::string::const_iterator iter = value.begin(); iter != value.end(); ++iter)
			{
				ss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << (int)(*iter);
			}
			return ss.str();
		}

		bool SecurityKeyManager::hasKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType) const
		{
			const ibrcommon::File keyfile = _path.get(hash(ref.getNode()) + ".pem");
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
		}

		dtn::security::SecurityKey SecurityKeyManager::get(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type) const throw (SecurityKey::KeyNotFoundException)
		{
			dtn::security::SecurityKey keydata;
			keydata.reference = ref.getNode();
			keydata.type = type;

			switch (type)
			{
				case SecurityKey::KEY_SHARED:
				{
					// read a symmetric key required for BAB signing
					const ibrcommon::File keyfile = _path.get(hash(keydata.reference) + ".mac");

					if (!keyfile.exists())
					{
						// get the default shared key
						const ibrcommon::File default_key = dtn::daemon::Configuration::getInstance().getSecurity().getBABDefaultKey();

						if (default_key.exists())
						{
							keydata.file = default_key;
							keydata.lastupdate = DTNTime(default_key.lastmodify(), 0);
							break;
						}

						IBRCOMMON_LOGGER_TAG(SecurityKeyManager::TAG, warning) << "Key file for " << ref.getString() << " (" << keyfile.getPath() << ") not found" << IBRCOMMON_LOGGER_ENDL;
						throw SecurityKey::KeyNotFoundException();
					}

					keydata.file = keyfile;
					keydata.lastupdate = DTNTime(keyfile.lastmodify(), 0);
					break;
				}

				case SecurityKey::KEY_UNSPEC:
				case SecurityKey::KEY_PUBLIC:
				case SecurityKey::KEY_PRIVATE:
				{
					const ibrcommon::File keyfile = _path.get(hash(keydata.reference) + ".pem");

					if (!keyfile.exists())
					{
						IBRCOMMON_LOGGER_TAG(SecurityKeyManager::TAG, warning) << "Key file for " << ref.getString() << " (" << keyfile.getPath() << ") not found" << IBRCOMMON_LOGGER_ENDL;
						throw SecurityKey::KeyNotFoundException();
					}


					keydata.file = keyfile;
					keydata.lastupdate = DTNTime(keyfile.lastmodify(), 0);
					break;
				}
			}

			return keydata;
		}

		void SecurityKeyManager::store(const dtn::security::SecurityKey &key)
		{
			ifstream stream(key.file.getPath().c_str(), std::iostream::in);
			std::stringstream ss;
			ss << stream.rdbuf();
			store(key.reference, ss.str(), key.type);
		}

		void SecurityKeyManager::store(const dtn::data::EID &ref, const std::string &data, const dtn::security::SecurityKey::KeyType type)
		{
			ibrcommon::File keyfile = _path.get(hash(ref.getNode()) + ".pem");

			// delete if already exists
			if (keyfile.exists()) keyfile.remove();

			std::ofstream keystream(keyfile.getPath().c_str());
			keystream << data;
			keystream.close();
		}
	}
}
