/*
 * EventReceiver.cpp
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
 */

#include "core/EventReceiver.h"
#include "core/EventSwitch.h"

namespace dtn
{
	namespace core
	{
		EventReceiver::~EventReceiver()
		{ };

		void EventReceiver::bindEvent(std::string eventName)
		{
			dtn::core::EventSwitch::registerEventReceiver(eventName, this);
		}

		void EventReceiver::unbindEvent(std::string eventName)
		{
			dtn::core::EventSwitch::unregisterEventReceiver(eventName, this);
		}
	}
}
