/*
 * DHProtocol.h
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

#ifndef DHPROTOCOL_H_
#define DHPROTOCOL_H_

#include "security/exchange/KeyExchangeProtocol.h"
#include "security/exchange/KeyExchangeSession.h"
#include <openssl/dh.h>
#include <openssl/bn.h>
#include <iostream>

namespace dtn
{
	namespace security
	{
		class DHProtocol : public dtn::security::KeyExchangeProtocol
		{
			static const std::string TAG;

		public:
			DHProtocol(KeyExchangeManager &manager);
			virtual ~DHProtocol();

			virtual KeyExchangeSession* createSession(const dtn::data::EID &peer, unsigned int uniqueId);

			virtual void initialize();

			virtual void begin(KeyExchangeSession &session, KeyExchangeData &data);
			virtual void step(KeyExchangeSession &session, KeyExchangeData &data);

		private:
			class DHState : public dtn::security::KeyExchangeSession::SessionState
			{
			public:
				DHState();
				virtual ~DHState();
				DH* dh;
				std::string secret;
			};

			static void write(std::ostream &stream, const BIGNUM* bn);
			static void read(std::istream &stream, BIGNUM **bn);

			void generate_params();

			ibrcommon::File _dh_params_file;
			DH* _dh_params;
			bool _auto_generate_params;
		};
	} /* namespace security */
} /* namespace dtn */

#endif /* DHPROTOCOL_H_ */
