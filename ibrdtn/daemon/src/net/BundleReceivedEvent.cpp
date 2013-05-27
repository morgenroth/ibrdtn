/*
 * BundleReceivedEvent.cpp
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

#include "net/BundleReceivedEvent.h"
#include "core/BundleCore.h"
#include "core/EventDispatcher.h"
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace net
	{
		// static semaphore to limit the number of received bundles in transit between CL and storage
		ibrcommon::Semaphore BundleReceivedEvent::_sem(5);

		BundleReceivedEvent::BundleReceivedEvent(const dtn::data::EID &p, const dtn::data::Bundle &b, const bool local)
		 : Event(-1), peer(p), bundle(b), fromlocal(local)
		{
			// allocate semaphore slot
			_sem.wait();
		}

		BundleReceivedEvent::~BundleReceivedEvent()
		{
			// free a semaphore slot
			_sem.post();
		}

		void BundleReceivedEvent::raise(const dtn::data::EID &peer, const dtn::data::Bundle &bundle, const bool local)
		{
			// raise the new event
			dtn::core::EventDispatcher<BundleReceivedEvent>::raise( new BundleReceivedEvent(peer, bundle, local), true );
		}

		const std::string BundleReceivedEvent::getName() const
		{
			return BundleReceivedEvent::className;
		}

		std::string BundleReceivedEvent::getMessage() const
		{
			if (fromlocal) {
				return "Bundle received " + bundle.toString() + " (local)";
			} else {
				return "Bundle received " + bundle.toString() + " from " + peer.getString();
			}
		}

		const string BundleReceivedEvent::className = "BundleReceivedEvent";
	}
}
