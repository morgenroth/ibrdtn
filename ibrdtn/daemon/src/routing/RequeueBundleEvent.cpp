/*
 * RequeueBundleEvent.cpp
 *
 *  Created on: 15.02.2010
 *      Author: morgenro
 */

#include "routing/RequeueBundleEvent.h"
#include "core/BundleCore.h"

namespace dtn
{
	namespace routing
	{
		RequeueBundleEvent::RequeueBundleEvent(const dtn::data::EID peer, const dtn::data::BundleID &id)
		 : _peer(peer), _bundle(id)
		{

		}

		RequeueBundleEvent::~RequeueBundleEvent()
		{

		}

		void RequeueBundleEvent::raise(const dtn::data::EID peer, const dtn::data::BundleID &id)
		{
			// raise the new event
			raiseEvent( new RequeueBundleEvent(peer, id) );
		}

		const string RequeueBundleEvent::getName() const
		{
			return RequeueBundleEvent::className;
		}

		string RequeueBundleEvent::toString() const
		{
			return className + ": Bundle requeued " + _bundle.toString();
		}

		const string RequeueBundleEvent::className = "RequeueBundleEvent";
	}
}
