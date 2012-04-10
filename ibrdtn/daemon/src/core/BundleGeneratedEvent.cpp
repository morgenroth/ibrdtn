/*
 * BundleGeneratedEvent.cpp
 *
 *  Created on: 23.06.2010
 *      Author: morgenro
 */

#include "core/BundleGeneratedEvent.h"
#include "core/BundleCore.h"
#include <ibrcommon/Logger.h>

namespace dtn
{

	namespace core
	{
		BundleGeneratedEvent::BundleGeneratedEvent(const dtn::data::Bundle &b)
		 : bundle(b)
		{

		}

		BundleGeneratedEvent::~BundleGeneratedEvent()
		{

		}

		void BundleGeneratedEvent::raise(const dtn::data::Bundle &bundle)
		{
			// raise the new event
			dtn::core::Event::raiseEvent( new BundleGeneratedEvent(bundle) );
		}

		const string BundleGeneratedEvent::getName() const
		{
			return BundleGeneratedEvent::className;
		}

		string BundleGeneratedEvent::toString() const
		{
			return className + ": Bundle generated " + bundle.toString();
		}

		const string BundleGeneratedEvent::className = "BundleGeneratedEvent";
	}

}
