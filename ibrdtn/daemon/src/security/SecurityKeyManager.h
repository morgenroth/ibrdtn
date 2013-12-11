/*
 * SecurityKeyManager.h
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

#ifndef SECURITYKEYMANAGER_H_
#define SECURITYKEYMANAGER_H_

#include "Configuration.h"
#include <ibrdtn/security/SecurityKey.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/BundleString.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrcommon/data/File.h>
#include <iostream>

#include <openssl/rsa.h>

namespace dtn
{
	namespace security
	{
		class SecurityKeyManager : public dtn::daemon::Configuration::OnChangeListener
		{
		public:
			class KeyNotFoundException : public ibrcommon::Exception
			{
			public:
				KeyNotFoundException(std::string what = "Requested key not found.") : ibrcommon::Exception(what)
				{};

				virtual ~KeyNotFoundException() throw() {};
			};

			static const std::string TAG;

			static SecurityKeyManager& getInstance();

			virtual ~SecurityKeyManager();

			/**
			 * Listen for changes of the configuration
			 */
			virtual void onConfigurationChanged(const dtn::daemon::Configuration &conf) throw ();

			bool hasKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC) const;
			dtn::security::SecurityKey get(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC) const throw (SecurityKeyManager::KeyNotFoundException);
			void store(const dtn::data::EID &ref, const std::string &data, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC);

		private:
			SecurityKeyManager();

			/**
			Reads a private key into rsa
			@param filename the file where the key is stored
			@param rsa the rsa file
			@return zero if all is ok, something different from zero when something
			failed
			*/
			static int read_private_key(const ibrcommon::File &file, RSA ** rsa);

			/**
			Reads a public key into rsa
			@param filename the file where the key is stored
			@param rsa the rsa file
			@return zero if all is ok, something different from zero when something
			failed
			*/
			static int read_public_key(const ibrcommon::File &file, RSA ** rsa);


			static const std::string hash(const dtn::data::EID &eid);

			ibrcommon::File _path;
			ibrcommon::File _ca;
			ibrcommon::File _key;
		};
	}
}

#endif /* SECURITYKEYMANAGER_H_ */
