/*
 * KeyExchangeEvent.cpp
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

#include "security/exchange/KeyExchangeEvent.h"
#include "core/EventDispatcher.h"

#include <sstream>

namespace dtn
{
	namespace security
	{
		KeyExchangeEvent::KeyExchangeEvent(const dtn::data::EID &eid, const dtn::security::KeyExchangeData &data)
		 : _eid(eid), _data(data)
		{}

		KeyExchangeEvent::~KeyExchangeEvent()
		{}

		const dtn::data::EID KeyExchangeEvent::getEID() const
		{
			return _eid;
		}

		const dtn::security::KeyExchangeData KeyExchangeEvent::getData() const
		{
			return _data;
		}

		const std::string KeyExchangeEvent::getName() const
		{
			return KeyExchangeEvent::className;
		}

		std::string KeyExchangeEvent::getMessage() const
		{
			if (_data.getAction() == dtn::security::KeyExchangeData::PASSWORD_REQUEST)
			{
				return "Please enter the password";
			}
			else if (_data.getAction() == dtn::security::KeyExchangeData::HASH_COMMIT)
			{
				return "Please compare the hash values: " + _data.str();
			}
			else if (_data.getAction() == dtn::security::KeyExchangeData::NEWKEY_FOUND)
			{
				return "A new key was found. Please select a key.";
			}
			else
			{
				std::stringstream sstm;
				sstm << "peer: " << _eid.getString() << " with " << _data.toString();
				return sstm.str();
			}
		}

		void KeyExchangeEvent::raise(const dtn::data::EID &eid, const dtn::security::KeyExchangeData &data)
		{
			// raise the new event
			dtn::core::EventDispatcher<KeyExchangeEvent>::queue( new KeyExchangeEvent(eid.getNode(), data) );
		}

		const string KeyExchangeEvent::className = "KeyExchangeEvent";
	} /* namespace security */
} /* namespace dtn */
