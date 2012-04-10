/*
 * NodeHandshakeEvent.cpp
 *
 *  Created on: 08.12.2011
 *      Author: morgenro
 */

#include "routing/NodeHandshakeEvent.h"

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
			dtn::core::Event::raiseEvent( new NodeHandshakeEvent(state, peer) );
		}

		const string NodeHandshakeEvent::className = "NodeHandshakeEvent";
	} /* namespace routing */
} /* namespace dtn */
