/*
 * HashProtocol.cpp
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

#include "security/exchange/HashProtocol.h"
#include "security/exchange/KeyExchangeEvent.h"
#include "security/SecurityKeyManager.h"
#include "core/BundleCore.h"
#include <iomanip>

#include <openssl/sha.h>
#include <openssl/rand.h>
#define RANDOM_NUMBER_LENGTH 256

namespace dtn
{
	namespace security
	{
		HashProtocol::HashState::~HashState()
		{
		}

		HashProtocol::HashProtocol(KeyExchangeManager &manager)
		 : KeyExchangeProtocol(manager, 3)
		{
		}

		HashProtocol::~HashProtocol()
		{
		}

		KeyExchangeSession* HashProtocol::createSession(const dtn::data::EID &peer, unsigned int uniqueId)
		{
			return new KeyExchangeSession(getProtocol(), peer, uniqueId, new HashState());
		}

		void HashProtocol::begin(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// get session state
			HashState &state = session.getState<HashState>();

			state.random = std::vector<char>(RANDOM_NUMBER_LENGTH);

			if(!RAND_bytes((unsigned char*)&state.random[0], RANDOM_NUMBER_LENGTH))
			{
				throw ibrcommon::Exception("Error while generating random number");
			}

			// get local public key
			const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

			KeyExchangeData request(KeyExchangeData::REQUEST, session);
			sha256(request, std::string(state.random.begin(), state.random.end()) + pkey.getData());

			manager.submit(session, request);
		}

		void HashProtocol::step(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// get session state
			HashState &state = session.getState<HashState>();

			switch (data.getStep())
			{
				case 0:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						// copy commitment from message to session
						const std::string data_str = data.str();
						state.commitment = std::vector<char>(data_str.begin(), data_str.end());

						// generate random bytes
						state.random = std::vector<char>(RANDOM_NUMBER_LENGTH);
						if(!RAND_bytes((unsigned char*)&state.random[0], RANDOM_NUMBER_LENGTH))
						{
							throw ibrcommon::Exception("Error while generating random number");
						}

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						KeyExchangeData response(KeyExchangeData::RESPONSE, session);
						sha256(response, std::string(&state.random[0], RANDOM_NUMBER_LENGTH) + pkey.getData());

						manager.submit(session, response);
					}
					else
					{
						const std::string data_str = data.str();
						state.commitment = std::vector<char>(data_str.begin(), data_str.end());

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						KeyExchangeData request(KeyExchangeData::REQUEST, session);
						request.setStep(1);

						const dtn::data::BundleString rnd_data(std::string(state.random.begin(), state.random.end()));
						request << rnd_data;

						const dtn::data::BundleString publicK(pkey.getData());
						request << publicK;

						manager.submit(session, request);
					}
					break;
				}

				case 1:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						dtn::data::BundleString r;
						data >> r;

						dtn::data::BundleString pub;
						data >> pub;

						//test commitment
						std::string commitment = std::string(&state.commitment[0], 2*SHA256_DIGEST_LENGTH);

						std::stringstream controlStream;
						sha256(controlStream, r + pub);
						if (commitment != controlStream.str())
						{
							throw ibrcommon::Exception("Error while comparing commitment");
						}

						// write key in tmp file
						session.putKey(pub, SecurityKey::KEY_PUBLIC, SecurityKey::MEDIUM);

						const std::string random(&state.random[0], RANDOM_NUMBER_LENGTH);

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						KeyExchangeData response(KeyExchangeData::RESPONSE, session);
						response.setStep(1);

						const dtn::data::BundleString rnd_data(std::string(state.random.begin(), state.random.end()));
						response << rnd_data;

						const dtn::data::BundleString publicK(pkey.getData());
						response << publicK;

						manager.submit(session, response);

						KeyExchangeData event(KeyExchangeData::HASH_COMMIT, session);
						sha256(event, r + pub + random + pkey.getData());
						KeyExchangeEvent::raise(session.getPeer(), event);
					}
					else
					{
						dtn::data::BundleString r;
						data >> r;

						dtn::data::BundleString pub;
						data >> pub;

						//test commitment
						const std::string commitment = std::string(&state.commitment[0], 2*SHA256_DIGEST_LENGTH);
						std::stringstream controlStream;
						sha256(controlStream, r + pub);

						if (commitment != controlStream.str())
						{
							throw ibrcommon::Exception("Error while comparing commitment");
						}

						// write key in tmp file
						session.putKey(pub, SecurityKey::KEY_PUBLIC, SecurityKey::MEDIUM);

						const std::string random(state.random.begin(), state.random.end());

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						KeyExchangeData event(KeyExchangeData::HASH_COMMIT, session);
						sha256(event, random + pkey.getData() + r + pub);
						event.setAction(KeyExchangeData::HASH_COMMIT);
						KeyExchangeEvent::raise(session.getPeer(), event);
					}
					break;
				}
			}
		}

		void HashProtocol::sha256(std::ostream &stream, const std::string &data)
		{
		    unsigned char hash[SHA256_DIGEST_LENGTH];
		    SHA256((const unsigned char*) data.c_str(), data.size(), hash);

			for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
			{
				stream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
			}
		}
	} /* namespace security */
} /* namespace dtn */
