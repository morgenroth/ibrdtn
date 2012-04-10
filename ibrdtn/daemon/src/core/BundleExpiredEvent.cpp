/*
 * BundleExpiredEvent.cpp
 *
 *  Created on: 15.02.2010
 *      Author: morgenro
 */

#include "core/BundleExpiredEvent.h"
#include "core/BundleCore.h"

namespace dtn
{
	namespace core
	{
		BundleExpiredEvent::BundleExpiredEvent(const dtn::data::BundleID &bundle)
		 : _bundle(bundle)
		{

		}

		BundleExpiredEvent::BundleExpiredEvent(const dtn::data::Bundle &bundle)
		 : _bundle(bundle)
		{

		}

		BundleExpiredEvent::~BundleExpiredEvent()
		{

		}

		void BundleExpiredEvent::raise(const dtn::data::Bundle &bundle)
		{
			// raise the new event
			raiseEvent( new BundleExpiredEvent(bundle) );
		}

		void BundleExpiredEvent::raise(const dtn::data::BundleID &bundle)
		{
			// raise the new event
			raiseEvent( new BundleExpiredEvent(bundle) );
		}

		const string BundleExpiredEvent::getName() const
		{
			return BundleExpiredEvent::className;
		}

		string BundleExpiredEvent::toString() const
		{
			return className + ": Bundle has been expired " + _bundle.toString();
		}

		const string BundleExpiredEvent::className = "BundleExpiredEvent";
	}
}
