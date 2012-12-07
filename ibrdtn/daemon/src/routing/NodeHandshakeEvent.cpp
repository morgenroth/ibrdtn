/*
 * NodeHandshakeEvent.cpp
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

#include "routing/NodeHandshakeEvent.h"
#include "core/EventDispatcher.h"

namespace dtn
{
	namespace routing
	{
		NodeHandshakeEvent::NodeHandshakeEvent(HANDSHAKE_STATE s, const dtn::data::EID &p)
		 : state(s), peer(p)
		{
		}

		NodeHandshakeEvent::~NodeHandshakeEvent()
		{
		}

		const std::string NodeHandshakeEvent::getName() const
		{
			return NodeHandshakeEvent::className;
		}

		std::string NodeHandshakeEvent::toString() const
		{
			switch (state)
			{
			case HANDSHAKE_REPLIED:
				return getName() + ": replied to " + peer.getString();
				break;
			case HANDSHAKE_COMPLETED:
				return getName() + ": completed with " + peer.getString();
				break;
			case HANDSHAKE_UPDATED:
				return getName() + ": updated of " + peer.getString();
				break;
			default:
				return getName() + ": " + peer.getString();
			}
		}

		void NodeHandshakeEvent::raiseEvent(HANDSHAKE_STATE state, const dtn::data::EID &peer)
		{
			dtn::core::EventDispatcher<NodeHandshakeEvent>::raise( new NodeHandshakeEvent(state, peer) );
		}

		const string NodeHandshakeEvent::className = "NodeHandshakeEvent";
	} /* namespace routing */
} /* namespace dtn */
