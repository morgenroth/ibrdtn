/*
 * NoneProtocol.cpp
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

#include "security/exchange/NoneProtocol.h"
#include "security/SecurityKeyManager.h"
#include "core/BundleCore.h"

namespace dtn
{
	namespace security
	{
		NoneProtocol::NoneProtocol(KeyExchangeManager &manager)
		 : KeyExchangeProtocol(manager, 0)
		{
		}

		NoneProtocol::~NoneProtocol()
		{
		}

		void NoneProtocol::begin(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// prepare request
			KeyExchangeData request(KeyExchangeData::REQUEST, session);

			// get local public key
			const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

			// set request parameters
			request.str(pkey.getData());

			// send request to the other peer
			manager.submit(session, request);
		}

		void NoneProtocol::step(KeyExchangeSession &session, KeyExchangeData &data)
		{
			if (data.getAction() == KeyExchangeData::REQUEST)
			{
				// write key in tmp file
				session.putKey(data.str(), SecurityKey::KEY_PUBLIC, SecurityKey::LOW);

				// prepare response
				KeyExchangeData response(KeyExchangeData::RESPONSE, session);

				// get local public key
				const SecurityKey pkey = SecurityKeyManager::getInstance().get(dtn::core::BundleCore::local, SecurityKey::KEY_PUBLIC);

				// reply with the own key
				response.str(pkey.getData());

				// send bundle to peer
				manager.submit(session, response);

				// finish the key-exchange
				manager.finish(session);
			}
			else if (data.getAction() == KeyExchangeData::RESPONSE)
			{
				// write key to session
				session.putKey(data.str(), SecurityKey::KEY_PUBLIC, SecurityKey::LOW);

				// finish the key-exchange
				manager.finish(session);
			}
		}
	} /* namespace security */
} /* namespace dtn */
