/*
 * P2PDialupEvent.cpp
 *
 *  Created on: 25.02.2013
 *      Author: morgenro
 */

#include "net/P2PDialupEvent.h"
#include "core/EventDispatcher.h"

namespace dtn
{
	namespace net
	{
		P2PDialupEvent::P2PDialupEvent(p2p_event_type t, const ibrcommon::vinterface &i)
		 : type(t), iface(i)
		{

		}

		P2PDialupEvent::~P2PDialupEvent()
		{

		}

		void P2PDialupEvent::raise(p2p_event_type t, const ibrcommon::vinterface &i)
		{
			// raise the new event
			dtn::core::EventDispatcher<P2PDialupEvent>::queue( new P2PDialupEvent(t, i) );
		}

		const std::string P2PDialupEvent::getName() const
		{
			return "P2PDialupEvent";
		}


		std::string P2PDialupEvent::getMessage() const
		{
			switch (type)
			{
			case INTERFACE_UP:
				return "interface " + iface.toString() + " up";
			case INTERFACE_DOWN:
				return "interface " + iface.toString() + " down";
			}

			return "unknown event";
		}
	} /* namespace net */
} /* namespace dtn */
