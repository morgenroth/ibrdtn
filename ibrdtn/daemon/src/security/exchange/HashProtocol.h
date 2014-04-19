/*
 * HashProtocol.h
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

#ifndef HASHPROTOCOL_H_
#define HASHPROTOCOL_H_

#include "security/exchange/KeyExchangeProtocol.h"
#include "security/exchange/KeyExchangeSession.h"

namespace dtn
{
	namespace security
	{
		class HashProtocol : public dtn::security::KeyExchangeProtocol
		{
		public:
			HashProtocol(KeyExchangeManager &manager);
			virtual ~HashProtocol();

			virtual KeyExchangeSession* createSession(const dtn::data::EID &peer, unsigned int uniqueId);

			virtual void begin(KeyExchangeSession &session, KeyExchangeData &data);
			virtual void step(KeyExchangeSession &session, KeyExchangeData &data);

		private:
			class HashState : public dtn::security::KeyExchangeSession::SessionState
			{
			public:
				virtual ~HashState();
				std::vector<char> random;
				std::vector<char> commitment;
			};

			static void sha256(std::ostream &stream, const std::string &str);
		};
	} /* namespace security */
} /* namespace dtn */

#endif /* HASHPROTOCOL_H_ */
