/*
 * EventDebugger.cpp
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

#include "core/EventDebugger.h"
#include "core/EventDispatcher.h"

#include "core/NodeEvent.h"
#include "core/CustodyEvent.h"
#include "core/BundleEvent.h"
#include "core/GlobalEvent.h"
#include "core/BundlePurgeEvent.h"
#include "core/BundleExpiredEvent.h"
#include "core/BundleGeneratedEvent.h"
#include "core/TimeAdjustmentEvent.h"

#include "net/ConnectionEvent.h"
#include "net/TransferAbortedEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/BundleReceivedEvent.h"

#include "routing/StaticRouteChangeEvent.h"
#include "routing/QueueBundleEvent.h"
#include "routing/RequeueBundleEvent.h"

#include <ibrcommon/Logger.h>
#include <iostream>

namespace dtn
{
	namespace core
	{
		EventDebugger::EventDebugger()
		{
		}

		EventDebugger::~EventDebugger()
		{
		}

		void EventDebugger::componentUp() throw ()
		{
			dtn::core::EventDispatcher<dtn::core::CustodyEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::BundleEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::NodeEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::GlobalEvent>::add(this);
			
			dtn::core::EventDispatcher<dtn::core::BundlePurgeEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::BundleExpiredEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::BundleGeneratedEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::TimeAdjustmentEvent>::add(this);

			dtn::core::EventDispatcher<dtn::net::ConnectionEvent>::add(this);
			dtn::core::EventDispatcher<dtn::net::TransferAbortedEvent>::add(this);
			dtn::core::EventDispatcher<dtn::net::TransferCompletedEvent>::add(this);
			dtn::core::EventDispatcher<dtn::net::BundleReceivedEvent>::add(this);

			dtn::core::EventDispatcher<dtn::routing::StaticRouteChangeEvent>::add(this);
			dtn::core::EventDispatcher<dtn::routing::QueueBundleEvent>::add(this);
			dtn::core::EventDispatcher<dtn::routing::RequeueBundleEvent>::add(this);

			/**
				NodeHandshakeEvent
				TimeEvent
				CertificateManagerInitEvent
			*/
		}

		void EventDebugger::componentDown() throw ()
		{
			dtn::core::EventDispatcher<dtn::core::CustodyEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::BundleEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::NodeEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::GlobalEvent>::remove(this);
			
			dtn::core::EventDispatcher<dtn::core::BundlePurgeEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::BundleExpiredEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::BundleGeneratedEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::TimeAdjustmentEvent>::remove(this);

			dtn::core::EventDispatcher<dtn::net::ConnectionEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::net::TransferAbortedEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::net::TransferCompletedEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::net::BundleReceivedEvent>::remove(this);

			dtn::core::EventDispatcher<dtn::routing::StaticRouteChangeEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::routing::QueueBundleEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::routing::RequeueBundleEvent>::remove(this);
		}

		void EventDebugger::raiseEvent(const Event *evt) throw ()
		{
			// print event
			if (evt->isLoggable())
			{
				IBRCOMMON_LOGGER_TAG(evt->getName(), notice) << evt->getMessage() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		const std::string EventDebugger::getName() const
		{
			return "EventDebugger";
		}
	}
}
