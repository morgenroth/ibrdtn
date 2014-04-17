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

namespace dtn
{
	namespace security
	{
		class SecurityKeyManager : public dtn::daemon::Configuration::OnChangeListener
		{
			static const std::string TAG;

		public:
			static SecurityKeyManager& getInstance();

			virtual ~SecurityKeyManager();

			/**
			 * Listen for changes of the configuration
			 */
			virtual void onConfigurationChanged(const dtn::daemon::Configuration &conf) throw ();

			bool hasKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC) const;
			dtn::security::SecurityKey get(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC) const throw (SecurityKey::KeyNotFoundException);
			void store(const dtn::data::EID &ref, const std::string &data, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC);

		private:
			SecurityKeyManager();

			static const std::string hash(const dtn::data::EID &eid);

			/**
			 * Create a fresh RSA key pair
			 */
			void createRSA(const dtn::data::EID &ref, const int bits = 2048);

			ibrcommon::File _path;
			ibrcommon::File _ca;
			ibrcommon::File _key;
		};
	}
}

#endif /* SECURITYKEYMANAGER_H_ */
