/*
 * JPAKEProtocol.h
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

#ifndef JPAKEPROTOCOL_H_
#define JPAKEPROTOCOL_H_

#include "security/exchange/KeyExchangeProtocol.h"
#include "security/exchange/KeyExchangeSession.h"
#include "security/exchange/KeyExchangeData.h"

#include <openssl/jpake.h>

namespace dtn
{
	namespace security
	{
		class JPAKEProtocol : public dtn::security::KeyExchangeProtocol
		{
		public:
			JPAKEProtocol(KeyExchangeManager &manager);
			virtual ~JPAKEProtocol();

			virtual KeyExchangeSession* createSession(const dtn::data::EID &peer, unsigned int uniqueId);

			virtual void begin(KeyExchangeSession &session, KeyExchangeData &data);
			virtual void step(KeyExchangeSession &session, KeyExchangeData &data);

		private:
			class JPAKEState : public dtn::security::KeyExchangeSession::SessionState
			{
			public:
				JPAKEState();
				virtual ~JPAKEState();
				JPAKE_CTX* ctx;
				std::string step1;
			};

			static void write(std::ostream &stream, const BIGNUM* bn);
			static void read(std::istream &stream, BIGNUM **bn);
		};
	} /* namespace security */
} /* namespace dtn */

#endif /* JPAKEPROTOCOL_H_ */
