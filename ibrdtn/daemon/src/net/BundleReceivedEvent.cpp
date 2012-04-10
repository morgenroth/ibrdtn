/*
 * BundleReceivedEvent.cpp
 *
 *  Created on: 15.02.2010
 *      Author: morgenro
 */

#include "net/BundleReceivedEvent.h"
#include "core/BundleCore.h"
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace net
	{
		BundleReceivedEvent::BundleReceivedEvent(const dtn::data::EID &p, const dtn::data::Bundle &b, const bool &local)
		 : Event(-1), peer(p), bundle(b), fromlocal(local)
		{

		}

		BundleReceivedEvent::~BundleReceivedEvent()
		{

		}

		void BundleReceivedEvent::raise(const dtn::data::EID &peer, const dtn::data::Bundle &bundle, const bool &local, const bool &wait)
		{
			// raise the new event
			dtn::core::Event::raiseEvent( new BundleReceivedEvent(peer, bundle, local), wait );
		}

		const std::string BundleReceivedEvent::getName() const
		{
			return BundleReceivedEvent::className;
		}

		std::string BundleReceivedEvent::toString() const
		{
			if (fromlocal) {
				return className + ": Bundle received " + bundle.toString() + " (local)";
			} else {
				return className + ": Bundle received " + bundle.toString() + " from " + peer.getString();
			}
		}

		const string BundleReceivedEvent::className = "BundleReceivedEvent";
	}
}
