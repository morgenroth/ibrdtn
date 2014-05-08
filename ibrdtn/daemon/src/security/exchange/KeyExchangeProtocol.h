/*
 * KeyExchangeProtocol.h
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

#ifndef KEYEXCHANGEPROTOCOL_H_
#define KEYEXCHANGEPROTOCOL_H_

#include "security/exchange/KeyExchangeSession.h"
#include "security/exchange/KeyExchangeData.h"
#include <map>

namespace dtn
{
	namespace security
	{
		class KeyExchangeManager
		{
		public:
			virtual ~KeyExchangeManager() = 0;
			virtual void submit(KeyExchangeSession &session, const KeyExchangeData &data) = 0;
			virtual void finish(KeyExchangeSession &session) = 0;
		};

		class KeyExchangeProtocol
		{
		public:
			KeyExchangeProtocol(KeyExchangeManager &manager, int protocol_id);
			virtual ~KeyExchangeProtocol();

			int getProtocol() const;

			/**
			 * Add this instance to a map
			 */
			void add(std::map<int, KeyExchangeProtocol*> &list);

			/**
			 * Create a new session for the given peer
			 */
			virtual KeyExchangeSession* createSession(const dtn::data::EID &peer, unsigned int uniqueId);

			virtual void initialize() {};

			virtual void begin(KeyExchangeSession &session, KeyExchangeData &data) = 0;
			virtual void step(KeyExchangeSession &session, KeyExchangeData &data) = 0;

			static void sha256(std::ostream &stream, const std::string &data);
			static std::string toHex(const std::string &data);

		protected:
			KeyExchangeManager &manager;

		private:
			const int _protocol_id;
		};
	}
}

#endif /* KEYEXCHANGEPROTOCOL_H_ */
