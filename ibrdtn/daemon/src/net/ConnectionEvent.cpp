/*
 * ConnectionEvent.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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

#include "net/ConnectionEvent.h"
#include "core/EventDispatcher.h"

namespace dtn
{
	namespace net
	{
		ConnectionEvent::ConnectionEvent(State s, const dtn::core::Node &n)
		 : state(s), peer(n.getEID()), node(n)
		{

		}

		ConnectionEvent::~ConnectionEvent()
		{

		}

		void ConnectionEvent::raise(State s, const dtn::core::Node &n)
		{
			// raise the new event
			dtn::core::EventDispatcher<ConnectionEvent>::raise( new ConnectionEvent(s, n) );
		}

		const string ConnectionEvent::getName() const
		{
			return ConnectionEvent::className;
		}


		std::string ConnectionEvent::getMessage() const
		{
			switch (state)
			{
			case CONNECTION_UP:
				return "connection up " + peer.getString();
			case CONNECTION_DOWN:
				return "connection down " + peer.getString();
			case CONNECTION_TIMEOUT:
				return "connection timeout " + peer.getString();
			case CONNECTION_SETUP:
				return "connection setup " + peer.getString();
			}

		}

		const string ConnectionEvent::className = "ConnectionEvent";
	}
}
