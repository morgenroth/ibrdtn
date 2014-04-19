/*
 * DHProtocol.cpp
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

#include "security/exchange/DHProtocol.h"
#include "security/SecurityKeyManager.h"
#include "core/BundleCore.h"
#include <ibrdtn/data/BundleString.h>

#include <ibrcommon/ssl/HMacStream.h>

#define DH_KEY_LENGTH 2048

namespace dtn
{
	namespace security
	{
		DHProtocol::DHState::DHState()
		 : dh(NULL)
		{
		}

		DHProtocol::DHState::~DHState()
		{
			if (dh) DH_free(dh);
		}

		DHProtocol::DHProtocol(KeyExchangeManager &manager)
		 : KeyExchangeProtocol(manager, 1)
		{
		}

		DHProtocol::~DHProtocol()
		{
		}

		KeyExchangeSession* DHProtocol::createSession(const dtn::data::EID &peer, unsigned int uniqueId)
		{
			return new KeyExchangeSession(getProtocol(), peer, uniqueId, new DHState());
		}

		void DHProtocol::begin(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// get session state
			DHState &state = session.getState<DHState>();

			// assign new DH
			state.dh = DH_new();

			if (!DH_generate_parameters_ex(state.dh, DH_KEY_LENGTH, DH_GENERATOR_2, NULL))
			{
				throw ibrcommon::Exception("Error while generating DH parameters");
			}

			int codes;
			if (!DH_check(state.dh, &codes))
			{
				throw ibrcommon::Exception("Error while checking DH parameters");
			}

			if (!DH_generate_key(state.dh))
			{
				throw ibrcommon::Exception("Error while generating DH key");
			}

			// prepare request
			KeyExchangeData request(KeyExchangeData::REQUEST, session);

			write(request, state.dh->pub_key);
			write(request, state.dh->p);
			write(request, state.dh->g);

			manager.submit(session, request);
		}

		void DHProtocol::step(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// get session state
			DHState &state = session.getState<DHState>();

			switch (data.getStep())
			{
				case 0:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						BIGNUM* pub_key = BN_new();
						read(data, &pub_key);

						DH* dh = DH_new();
						read(data, &dh->p);
						read(data, &dh->g);

						// DH* erzeugen
						DH_generate_key(dh);

						unsigned char* secret;
						long int length = sizeof(unsigned char) * (DH_size(dh));
						if (NULL == (secret = (unsigned char*) OPENSSL_malloc(length)))
						{
							DH_free(dh);
							BN_free(pub_key);
							throw ibrcommon::Exception("Error while allocating space for secret key");
						}

						DH_compute_key(secret, pub_key, dh);

						state.secret.assign((const char*)secret, length);

						KeyExchangeData response(KeyExchangeData::RESPONSE, session);
						write(response, dh->pub_key);

						manager.submit(session, response);

						DH_free(dh);
						BN_free(pub_key);
					}
					else
					{
						BIGNUM* pub_key = BN_new();
						read(data, &pub_key);

						unsigned char* secret;
						long int length = sizeof(unsigned char) * (DH_size(state.dh));
						if(NULL == (secret = (unsigned char*) OPENSSL_malloc(length)))
						{
							BN_free(pub_key);
							throw ibrcommon::Exception("Error while allocating space for secret key");
						}

						DH_compute_key(secret, pub_key, state.dh);
						state.secret.assign((char*) secret, length);

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						ibrcommon::HMacStream hstream(secret, (int) length);
						hstream << pkey.getData() << std::flush;

						KeyExchangeData response(KeyExchangeData::REQUEST, session);
						response.setStep(1);

						const dtn::data::BundleString hmac(ibrcommon::HashStream::extract(hstream));
						response << hmac;

						const dtn::data::BundleString publicK(pkey.getData());
						response << publicK;

						manager.submit(session, response);

						BN_free(pub_key);
					}
					break;
				}

				case 1:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						dtn::data::BundleString hmac;
						data >> hmac;

						dtn::data::BundleString publicK;
						data >> publicK;

						ibrcommon::HMacStream hmacstream((const unsigned char*) state.secret.c_str(), (int) state.secret.size());
						hmacstream << (const std::string&)publicK << std::flush;

						if (hmac != ibrcommon::HMacStream::extract(hmacstream))
						{
							throw ibrcommon::Exception("Error while comparing hmac");
						}

						// write key in tmp file
						session.putKey(publicK, SecurityKey::KEY_PUBLIC);

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						// create a HMAC from public key
						ibrcommon::HMacStream hstream((const unsigned char*) state.secret.c_str(), (int) state.secret.size());
						hstream << pkey.getData() << std::flush;

						// bundle erzeugen: EID, hmac/publicKey, response, round 1
						KeyExchangeData response(KeyExchangeData::REQUEST, session);
						response.setStep(1);

						const dtn::data::BundleString local_hmac(ibrcommon::HashStream::extract(hstream));
						response << local_hmac;

						const dtn::data::BundleString local_publicK(pkey.getData());
						response << local_publicK;

						// send bundle to peer
						manager.submit(session, response);

						// finish the key-exchange
						manager.finish(session);
					}
					else
					{
						dtn::data::BundleString hmac;
						data >> hmac;

						dtn::data::BundleString publicK;
						data >> publicK;

						ibrcommon::HMacStream hstream((unsigned char*) state.secret.c_str(), (int) state.secret.size());
						hstream << publicK << std::flush;

						if (hmac != ibrcommon::HMacStream::extract(hstream))
						{
							throw ibrcommon::Exception("Error while comparing hmac");
						}

						// write key in tmp file
						session.putKey(publicK, SecurityKey::KEY_PUBLIC);

						// finish the key-exchange
						manager.finish(session);
					}
					break;
				}
			}
		}

		void DHProtocol::write(std::ostream &stream, const BIGNUM* bn)
		{
			unsigned char* buf = new unsigned char[(BN_num_bits(bn) + 1) * sizeof(char)];
			dtn::data::Number length = BN_bn2bin(bn, (unsigned char*)buf);

			if (length > 0)
			{
				stream << length;
				stream.write((char*)buf, length.get<size_t>());
				delete[] buf;
			}
			else
			{
				delete[] buf;
				throw ibrcommon::Exception("Error while parsing BIGNUM to string");
			}
		}

		void DHProtocol::read(std::istream &stream, BIGNUM **bn)
		{
			dtn::data::Number length;
			stream >> length;

			unsigned char* buf = new unsigned char[length.get<size_t>()];
			stream.read((char*)buf, length.get<size_t>());
			*bn = BN_bin2bn((unsigned char*)buf, (int) length.get<size_t>(), NULL);
			delete[] buf;

			if (*bn == NULL)
			{
				throw ibrcommon::Exception("Error while parsing string to BIGNUM");
			}
		}
	} /* namespace security */
} /* namespace dtn */
