/*
 * KeyExchangeSession.cpp
 *
 * Copyright (C) 2014 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *             Thomas Schrader <schrader.thomas@gmail.com>
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

#include "security/exchange/KeyExchangeSession.h"
#include "security/SecurityKeyManager.h"
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace security
	{
		const std::string KeyExchangeSession::TAG = "KeyExchangeSession";

		KeyExchangeSession::SessionState::~SessionState()
		{
		}

		KeyExchangeSession::KeyExchangeSession(int protocol, const dtn::data::EID &peer, unsigned int uniqueId, SessionState *state)
		: _protocol(protocol), _unique_id(uniqueId), _peer(peer.getNode()), _state(state), _expiration(0)
		{
			// generate session key
			_session_key = getSessionKey(_peer, _unique_id);

			// set a expiration time (10 minutes)
			_expiration = dtn::utils::Clock::getMonotonicTimestamp() + 600;
		}

		KeyExchangeSession::~KeyExchangeSession()
		{
			// clear session state
			if (_state != NULL) delete _state;

			// delete all session files
			clearKeys();
		}

		dtn::data::Timestamp KeyExchangeSession::getExpiration() const
		{
			return _expiration;
		}

		void KeyExchangeSession::touch()
		{
			// set new expiration time
			_expiration = dtn::utils::Clock::getMonotonicTimestamp() + 600;
		}

		std::string KeyExchangeSession::getSessionKey(const dtn::data::EID &peer, unsigned int uniqueId)
		{
			// generate session key
			std::stringstream sstm;
			sstm << uniqueId << "." << peer.getString();

			return sstm.str();
		}

		const dtn::data::EID& KeyExchangeSession::getPeer() const
		{
			return _peer;
		}

		int KeyExchangeSession::getProtocol() const
		{
			return _protocol;
		}

		unsigned int KeyExchangeSession::getUniqueId() const
		{
			return _unique_id;
		}

		const std::string& KeyExchangeSession::getSessionKey() const
		{
			return _session_key;
		}

		dtn::security::SecurityKey KeyExchangeSession::getKey(const dtn::security::SecurityKey::KeyType type) const throw (SecurityKey::KeyNotFoundException)
		{
			dtn::security::SecurityKey keydata;
			keydata.reference = _peer;
			keydata.type = type;

			std::stringstream prefix;
			prefix << getSessionKey() << ".";

			// get keyfile path
			const ibrcommon::File keyfile = SecurityKeyManager::getInstance().getKeyFile(prefix.str(), _peer, type);

			switch (type)
			{
				case SecurityKey::KEY_SHARED:
				{
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

						IBRCOMMON_LOGGER_TAG(TAG, warning) << "Key file for " << _peer.getString() << " (" << keyfile.getPath() << ") not found" << IBRCOMMON_LOGGER_ENDL;
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
					if (!keyfile.exists())
					{
						IBRCOMMON_LOGGER_TAG(TAG, warning) << "Key file for " << _peer.getString() << " (" << keyfile.getPath() << ") not found" << IBRCOMMON_LOGGER_ENDL;
						throw SecurityKey::KeyNotFoundException();
					}


					keydata.file = keyfile;
					keydata.lastupdate = DTNTime(keyfile.lastmodify(), 0);
					break;
				}
			}

			return keydata;
		}

		void KeyExchangeSession::putKey(const std::string &data, const dtn::security::SecurityKey::KeyType type)
		{
			std::stringstream prefix;
			prefix << getSessionKey() << ".";

			// get keyfile path
			ibrcommon::File keyfile = SecurityKeyManager::getInstance().getKeyFile(prefix.str(), _peer, type);

			// delete if already exists
			if (keyfile.exists()) keyfile.remove();

			std::ofstream keystream(keyfile.getPath().c_str());
			keystream << data;
			keystream.close();
		}

		void KeyExchangeSession::clearKeys()
		{
			std::list<dtn::security::SecurityKey::KeyType> types;
			types.push_back(dtn::security::SecurityKey::KEY_SHARED);
			types.push_back(dtn::security::SecurityKey::KEY_PUBLIC);
			types.push_back(dtn::security::SecurityKey::KEY_PRIVATE);

			for (std::list<dtn::security::SecurityKey::KeyType>::const_iterator it = types.begin(); it != types.end(); ++it)
			{
				std::stringstream prefix;
				prefix << getSessionKey() << ".";

				// get keyfile path
				ibrcommon::File keyfile = SecurityKeyManager::getInstance().getKeyFile(prefix.str(), _peer, *it);

				// delete if exists
				if (keyfile.exists()) keyfile.remove();
			}
		}
	} /* namespace security */
} /* namespace dtn */
