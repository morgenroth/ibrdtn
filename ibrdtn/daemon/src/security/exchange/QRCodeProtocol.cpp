/*
 * QRCodeProtocol.cpp
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

#include "security/exchange/QRCodeProtocol.h"
#include "security/exchange/KeyExchangeEvent.h"
#include "security/SecurityKeyManager.h"
#include "core/BundleCore.h"

namespace dtn
{
	namespace security
	{
		QRCodeProtocol::QRCodeProtocol(KeyExchangeManager &manager)
		 : KeyExchangeProtocol(manager, 4)
		{
		}

		QRCodeProtocol::~QRCodeProtocol()
		{
		}

		void QRCodeProtocol::begin(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// get existing security key
			SecurityKey key = SecurityKeyManager::getInstance().get(session.getPeer(), SecurityKey::KEY_PUBLIC);

			// get key fingerprint
			const std::string fingerprint = key.getFingerprint();

			// prepare a response
			KeyExchangeData response(KeyExchangeData::COMPLETE, session);

			if (fingerprint == data.str())
			{
				// store existing key with HIGH trust level
				session.putKey(key.getData(), key.type, SecurityKey::HIGH);

				// finish the key-exchange
				manager.finish(session);
			}
			else
			{
				// signal error
				throw ibrcommon::Exception("fingerprint is missing");
			}
		}

		void QRCodeProtocol::step(KeyExchangeSession &session, KeyExchangeData &data)
		{
			throw ibrcommon::Exception("invalid step");
		}
	} /* namespace security */
} /* namespace dtn */
