/*
 * SecurityKeyManager.cpp
 *
 *  Created on: 02.12.2010
 *      Author: morgenro
 */

#include "Configuration.h"
#include "security/SecurityKeyManager.h"
#include <ibrcommon/Logger.h>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <openssl/pem.h>
#include <openssl/err.h>

namespace dtn
{
	namespace security
	{
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

		void SecurityKeyManager::initialize(const ibrcommon::File &path, const ibrcommon::File &ca, const ibrcommon::File &key)
		{
			IBRCOMMON_LOGGER(info) << "security key manager initialized; path: " << path.getPath() << IBRCOMMON_LOGGER_ENDL;

			// store all paths locally
			_path = path; _key = key; _ca = ca;
		}

		const std::string SecurityKeyManager::hash(const dtn::data::EID &eid)
		{
			std::string value = eid.getNode().getString();
			std::stringstream ss;
			for (std::string::const_iterator iter = value.begin(); iter != value.end(); iter++)
			{
				ss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << (int)(*iter);
			}
			return ss.str();
		}

		void SecurityKeyManager::prefetchKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type)
		{
		}

		bool SecurityKeyManager::hasKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type) const
		{
			const ibrcommon::File keyfile = _path.get(hash(ref.getNode()) + ".pem");
			return keyfile.exists();
		}

		dtn::security::SecurityKey SecurityKeyManager::get(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type) const throw (SecurityKeyManager::KeyNotFoundException)
		{
			dtn::security::SecurityKey keydata;
			keydata.reference = ref.getNode();
			keydata.type = type;

			switch (type)
			{
				case SecurityKey::KEY_SHARED:
				{
					// read a symmetric key required for BAB signing
					const ibrcommon::File keyfile = _path.get(hash(ref.getNode()) + ".mac");

					if (!keyfile.exists())
					{
						// get the default shared key
						const ibrcommon::File default_key = dtn::daemon::Configuration::getInstance().getSecurity().getBABDefaultKey();

						if (default_key.exists())
						{
							keydata.file = default_key;
							keydata.lastupdate = default_key.lastmodify();
							break;
						}

						IBRCOMMON_LOGGER(warning) << "Key file for " << ref.getString() << " (" << keyfile.getPath() << ") not found" << IBRCOMMON_LOGGER_ENDL;
						throw SecurityKeyManager::KeyNotFoundException();
					}

					keydata.file = keyfile;
					keydata.lastupdate = keyfile.lastmodify();
					break;
				}

				case SecurityKey::KEY_UNSPEC:
				case SecurityKey::KEY_PUBLIC:
				case SecurityKey::KEY_PRIVATE:
				{
					const ibrcommon::File keyfile = _path.get(hash(ref.getNode()) + ".pem");

					if (!keyfile.exists())
					{
						IBRCOMMON_LOGGER(warning) << "Key file for " << ref.getString() << " (" << keyfile.getPath() << ") not found" << IBRCOMMON_LOGGER_ENDL;
						throw SecurityKeyManager::KeyNotFoundException();
					}


					keydata.file = keyfile;
					keydata.lastupdate = keyfile.lastmodify();
					break;
				}
			}

			return keydata;
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
