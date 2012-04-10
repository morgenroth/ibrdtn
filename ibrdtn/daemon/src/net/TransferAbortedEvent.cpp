/*
 * TransferAbortedEvent.cpp
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
 */

#include "net/TransferAbortedEvent.h"

namespace dtn
{
	namespace net
	{
		TransferAbortedEvent::TransferAbortedEvent(const dtn::data::EID &peer, const dtn::data::Bundle &bundle, const AbortReason r)
		 : reason(r), _peer(peer), _bundle(bundle)
		{
		}

		TransferAbortedEvent::TransferAbortedEvent(const dtn::data::EID &peer, const dtn::data::BundleID &id, const AbortReason r)
		 : reason(r), _peer(peer), _bundle(id)
		{
		}

		TransferAbortedEvent::~TransferAbortedEvent()
		{

		}

		void TransferAbortedEvent::raise(const dtn::data::EID &peer, const dtn::data::Bundle &bundle, const AbortReason r)
		{
			// raise the new event
			dtn::core::Event::raiseEvent( new TransferAbortedEvent(peer, bundle, r) );
		}

		void TransferAbortedEvent::raise(const dtn::data::EID &peer, const dtn::data::BundleID &id, const AbortReason r)
		{
			// raise the new event
			dtn::core::Event::raiseEvent( new TransferAbortedEvent(peer, id, r) );
		}

		const std::string TransferAbortedEvent::getName() const
		{
			return TransferAbortedEvent::className;
		}

		const dtn::data::EID& TransferAbortedEvent::getPeer() const
		{
			return _peer;
		}

		const dtn::data::BundleID& TransferAbortedEvent::getBundleID() const
		{
			return _bundle;
		}

		const std::string TransferAbortedEvent::getReason(const AbortReason reason)
		{
			switch (reason)
			{
			case REASON_UNDEFINED:
				return "undefined";

			case REASON_CONNECTION_DOWN:
				return "connection went down";

			case REASON_REFUSED:
				return "bundle has been refused";

			case REASON_RETRY_LIMIT_REACHED:
				return "retry limit reached";

			case REASON_BUNDLE_DELETED:
				return "bundle has been deleted";
			}

			return "undefined";
		}

		string TransferAbortedEvent::toString() const
		{
			return className + ": transfer of bundle " + _bundle.toString() + " to " + _peer.getString() + " aborted. (" + getReason(reason) + ")";
		}

		const std::string TransferAbortedEvent::className = "TransferAbortedEvent";
	}
}
