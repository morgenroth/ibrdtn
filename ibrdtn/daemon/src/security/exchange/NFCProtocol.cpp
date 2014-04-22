/*
 * NFCProtocol.cpp
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

#include "security/exchange/NFCProtocol.h"

namespace dtn
{
	namespace security
	{
		NFCProtocol::NFCProtocol(KeyExchangeManager &manager)
		 : KeyExchangeProtocol(manager, 5)
		{
		}

		NFCProtocol::~NFCProtocol()
		{
		}

		void NFCProtocol::begin(KeyExchangeSession &session, KeyExchangeData &data)
		{
			// store received key in the session
			session.putKey(data.str(), SecurityKey::KEY_PUBLIC, SecurityKey::HIGH);

			// finish the key-exchange
			manager.finish(session);
		}

		void NFCProtocol::step(KeyExchangeSession &session, KeyExchangeData &data)
		{
		}
	} /* namespace security */
} /* namespace dtn */
