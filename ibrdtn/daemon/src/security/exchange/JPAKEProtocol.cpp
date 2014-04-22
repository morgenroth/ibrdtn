/*
 * JPAKEProtocol.cpp
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

#include "security/exchange/JPAKEProtocol.h"
#include "security/exchange/KeyExchangeEvent.h"
#include "security/SecurityKeyManager.h"
#include "core/BundleCore.h"
#include <ibrcommon/ssl/HMacStream.h>

#include <openssl/sha.h>

#define JPAKE_PRIME_LENGTH 1024


namespace dtn
{
	namespace security
	{
		JPAKEProtocol::JPAKEState::JPAKEState()
		 : ctx(NULL)
		{
		}

		JPAKEProtocol::JPAKEState::~JPAKEState()
		{
			if (ctx) JPAKE_CTX_free(ctx);
		}

		JPAKEProtocol::JPAKEProtocol(KeyExchangeManager &manager)
		 : KeyExchangeProtocol(manager, 2)
		{
		}

		JPAKEProtocol::~JPAKEProtocol()
		{
		}

		KeyExchangeSession* JPAKEProtocol::createSession(const dtn::data::EID &peer, unsigned int uniqueId)
		{
			return new KeyExchangeSession(getProtocol(), peer, uniqueId, new JPAKEState());
		}

		void JPAKEProtocol::begin(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// get session state
			JPAKEState &state = session.getState<JPAKEState>();

			// JPAKE_STEP1 erzeugen
			BIGNUM* p;
			p = BN_new();
			//BN_generate_prime_ex(p, JPAKE_PRIME_LENGTH, 1, NULL, NULL, NULL);

			BN_hex2bn(&p, "F9E5B365665EA7A05A9C534502780FEE6F1AB5BD4F49947FD036DBD7E905269AF46EF28B0FC07487EE4F5D20FB3C0AF8E700F3A2FA3414970CBED44FEDFF80CE78D800F184BB82435D137AADA2C6C16523247930A63B85661D1FC817A51ACD96168E95898A1F83A79FFB529368AA7833ABD1B0C3AEDDB14D2E1A2F71D99F763F");

			BIGNUM* q;
			q = BN_new();
			BN_rshift1(q, p);

			const std::string password = data.str();

			BIGNUM* secret = BN_new();
			BN_bin2bn((const unsigned char*)password.c_str(), (int)password.size(), secret);

			BIGNUM* g = BN_new();
			BN_set_word(g, 2);

			state.ctx = JPAKE_CTX_new(toHex(dtn::core::BundleCore::local.getString()).c_str(), toHex(session.getPeer().getString()).c_str(), p, g, q, secret);

			JPAKE_STEP1 send1;
			JPAKE_STEP1_init(&send1);
			JPAKE_STEP1_generate(&send1, state.ctx);

			KeyExchangeData request(KeyExchangeData::REQUEST, session);

			write(request, send1.p1.gx);
			write(request, send1.p1.zkpx.gr);
			write(request, send1.p1.zkpx.b);
			write(request, send1.p2.gx);
			write(request, send1.p2.zkpx.gr);
			write(request, send1.p2.zkpx.b);
			write(request, p);
			write(request, q);

			manager.submit(session, request);

			JPAKE_STEP1_release(&send1);

			BN_free(p);
			BN_free(q);
			BN_free(secret);
			BN_free(g);
		}

		void JPAKEProtocol::step(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// get session state
			JPAKEState &state = session.getState<JPAKEState>();

			switch (data.getStep())
			{
				case 0:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						state.step1 = data.str();

						KeyExchangeData request(KeyExchangeData::PASSWORD_REQUEST, session);
						KeyExchangeEvent::raise(session.getPeer(), request);
					}
					else
					{
						// copy previous step to a stream
						std::stringstream step1(state.step1);

						JPAKE_STEP1 received1;
						JPAKE_STEP1_init(&received1);
						read(step1, &received1.p1.gx);
						read(step1, &received1.p1.zkpx.gr);
						read(step1, &received1.p1.zkpx.b);
						read(step1, &received1.p2.gx);
						read(step1, &received1.p2.zkpx.gr);
						read(step1, &received1.p2.zkpx.b);

						BIGNUM* p = BN_new();
						read(step1, &p);
						BIGNUM* q = BN_new();
						read(step1, &q);
						BIGNUM* secret = BN_new();
						BN_bin2bn((const unsigned char*)data.str().c_str(), (int)data.str().size(), secret);
						BIGNUM* g = BN_new();
						BN_set_word(g, 2);

						state.ctx = JPAKE_CTX_new(toHex(dtn::core::BundleCore::local.getString()).c_str(), toHex(session.getPeer().getString()).c_str(), p, g, q, secret);

						if (!JPAKE_STEP1_process(state.ctx, &received1))
						{
							JPAKE_STEP1_release(&received1);
							BN_free(p);
							BN_free(q);
							BN_free(secret);
							BN_free(g);
							throw ibrcommon::Exception("Error while processing STEP1");
						}

						JPAKE_STEP1_release(&received1);

						JPAKE_STEP1 send1;
						JPAKE_STEP1_init(&send1);
						JPAKE_STEP1_generate(&send1, state.ctx);

						KeyExchangeData response(KeyExchangeData::RESPONSE, session);
						response.setStep(1);

						write(response, send1.p1.gx);
						write(response, send1.p1.zkpx.gr);
						write(response, send1.p1.zkpx.b);
						write(response, send1.p2.gx);
						write(response, send1.p2.zkpx.gr);
						write(response, send1.p2.zkpx.b);

						manager.submit(session, response);

						JPAKE_STEP1_release(&send1);

						BN_free(p);
						BN_free(q);
						BN_free(secret);
						BN_free(g);
					}
					break;
				}

				case 1:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						// nothing to do?
					}
					else
					{
						JPAKE_STEP1 received1;
						JPAKE_STEP1_init(&received1);
						read(data, &received1.p1.gx);
						read(data, &received1.p1.zkpx.gr);
						read(data, &received1.p1.zkpx.b);
						read(data, &received1.p2.gx);
						read(data, &received1.p2.zkpx.gr);
						read(data, &received1.p2.zkpx.b);

						if (!JPAKE_STEP1_process(state.ctx, &received1))
						{
							JPAKE_STEP1_release(&received1);
							throw ibrcommon::Exception("Error while processing STEP1");
						}

						JPAKE_STEP1_release(&received1);

						JPAKE_STEP2 send2;
						JPAKE_STEP2_init(&send2);
						JPAKE_STEP2_generate(&send2, state.ctx);

						KeyExchangeData request(KeyExchangeData::REQUEST, session);
						request.setStep(2);

						write(request, send2.gx);
						write(request, send2.zkpx.gr);
						write(request, send2.zkpx.b);

						manager.submit(session, request);

						JPAKE_STEP2_release(&send2);
					}
					break;
				}

				case 2:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						// Berechne J-PAKE-Parameter und sende jpake_response 2 (2) am anderen
						JPAKE_STEP2 received2;
						JPAKE_STEP2_init(&received2);
						read(data, &received2.gx);
						read(data, &received2.zkpx.gr);
						read(data, &received2.zkpx.b);

						if (!JPAKE_STEP2_process(state.ctx, &received2))
						{
							JPAKE_STEP2_release(&received2);
							throw ibrcommon::Exception("Error while processing STEP2");
						}

						JPAKE_STEP2_release(&received2);

						JPAKE_STEP2 send2;
						JPAKE_STEP2_init(&send2);
						JPAKE_STEP2_generate(&send2, state.ctx);

						KeyExchangeData response(KeyExchangeData::RESPONSE, session);
						response.setStep(2);

						write(response, send2.gx);
						write(response, send2.zkpx.gr);
						write(response, send2.zkpx.b);

						manager.submit(session, response);

						JPAKE_STEP2_release(&send2);
					}
					else
					{
						// Berechne hash und sende control_request (3) an anderen
						JPAKE_STEP2 received2;
						JPAKE_STEP2_init(&received2);
						read(data, &received2.gx);
						read(data, &received2.zkpx.gr);
						read(data, &received2.zkpx.b);

						if (!JPAKE_STEP2_process(state.ctx, &received2))
						{
							JPAKE_STEP2_release(&received2);
							throw ibrcommon::Exception("Error while processing STEP2");
						}

						JPAKE_STEP2_release(&received2);

						JPAKE_STEP3A send3;
						JPAKE_STEP3A_init(&send3);
						JPAKE_STEP3A_generate(&send3, state.ctx);

						KeyExchangeData request(KeyExchangeData::REQUEST, session);
						request.setStep(3);

						request.write((char*) send3.hhk, SHA_DIGEST_LENGTH);

						manager.submit(session, request);

						JPAKE_STEP3A_release(&send3);
					}
					break;
				}

				case 3:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						// Berechne hash, sende control response (3) an anderen und eventuell wrongpassword
						JPAKE_STEP3A received3;
						JPAKE_STEP3A_init(&received3);
						data.read((char*)&received3.hhk, SHA_DIGEST_LENGTH);

						if (!JPAKE_STEP3A_process(state.ctx, &received3))
						{
							JPAKE_STEP3A_release(&received3);
							throw ibrcommon::Exception("Error while processing STEP3A");
						}

						JPAKE_STEP3A_release(&received3);

						JPAKE_STEP3B send3;
						JPAKE_STEP3B_init(&send3);
						JPAKE_STEP3B_generate(&send3, state.ctx);

						KeyExchangeData response(KeyExchangeData::RESPONSE, session);
						response.setStep(3);

						response.write((char*) send3.hk, SHA_DIGEST_LENGTH);

						manager.submit(session, response);

						JPAKE_STEP3B_release(&send3);
					}
					else
					{
						// Berechne hmac-Parameter und sende hmac_request (4) an anderen oder wrong_password
						JPAKE_STEP3B received3;
						JPAKE_STEP3B_init(&received3);
						data.read((char*)&received3.hk, SHA_DIGEST_LENGTH);

						if (!JPAKE_STEP3B_process(state.ctx, &received3))
						{
							JPAKE_STEP3B_release(&received3);
							throw ibrcommon::Exception("Error while processing STEP3B");
						}

						JPAKE_STEP3B_release(&received3);

						std::stringstream secretBuf;
						write(secretBuf, JPAKE_get_shared_key(state.ctx));

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						dtn::data::BundleString publicKey = dtn::data::BundleString(pkey.getData());
						ibrcommon::HMacStream hstream((unsigned char*) secretBuf.str().c_str(), (int) secretBuf.str().size());
						hstream << publicKey << std::flush;

						// bundle erzeugen: EID, hmac/publicKey, request, round 1
						KeyExchangeData request(KeyExchangeData::REQUEST, session);
						request.setStep(4);

						request << dtn::data::BundleString(ibrcommon::HashStream::extract(hstream));
						request << publicKey;

						manager.submit(session, request);
					}
					break;
				}

				case 4:
				{
					if (data.getAction() == KeyExchangeData::REQUEST)
					{
						// Berechne HMAC-Parameter, sende hmac_response (4) an anderen und jpake complete bzw. new key

						std::stringstream secretBuf;
						write(secretBuf,JPAKE_get_shared_key(state.ctx));

						dtn::data::BundleString hmac;
						data >> hmac;
						dtn::data::BundleString publicK;
						data >> publicK;

						ibrcommon::HMacStream hmacstream((unsigned char*) secretBuf.str().c_str(), (int) secretBuf.str().size());
						hmacstream << publicK << std::flush;

						if (hmac != ibrcommon::HMacStream::extract(hmacstream))
						{
							throw ibrcommon::Exception("Error while comparing hmac");
						}

						// write key in tmp file
						session.putKey(publicK, SecurityKey::KEY_PUBLIC, SecurityKey::MEDIUM);

						// get local public key
						const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

						dtn::data::BundleString publicKey = dtn::data::BundleString(pkey.getData());
						ibrcommon::HMacStream hstream((unsigned char*) secretBuf.str().c_str(), (int) secretBuf.str().size());
						hstream << publicKey << std::flush;

						KeyExchangeData response(KeyExchangeData::RESPONSE, session);
						response.setStep(4);

						response << dtn::data::BundleString(ibrcommon::HashStream::extract(hstream));
						response << publicKey;

						manager.submit(session, response);

						manager.finish(session);
					}
					else
					{
						// Sende jpake_complete bzw new key

						std::stringstream secretBuf;
						write(secretBuf, JPAKE_get_shared_key(state.ctx));

						dtn::data::BundleString hmac;
						data >> hmac;

						dtn::data::BundleString publicK;
						data >> publicK;

						ibrcommon::HMacStream hstream((unsigned char*) secretBuf.str().c_str(), (int) secretBuf.str().size());
						hstream << publicK << std::flush;

						if (hmac != ibrcommon::HMacStream::extract(hstream))
						{
							throw ibrcommon::Exception("Error while comparing hmac");
						}

						// write key in tmp file
						session.putKey(publicK, SecurityKey::KEY_PUBLIC, SecurityKey::MEDIUM);

						manager.finish(session);
					}
					break;
				}
			}
		}

		void JPAKEProtocol::write(std::ostream &stream, const BIGNUM* bn)
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

		void JPAKEProtocol::read(std::istream &stream, BIGNUM **bn)
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
