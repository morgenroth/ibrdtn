/*
 * BundlePurgeEvent.cpp
 *
 *  Created on: 13.04.2012
 *      Author: morgenro
 */

#include "core/BundlePurgeEvent.h"

namespace dtn
{
	namespace core
	{
		BundlePurgeEvent::BundlePurgeEvent(const dtn::data::BundleID &id, dtn::data::StatusReportBlock::REASON_CODE r)
		 : bundle(id), reason(r)
		{
		}

		BundlePurgeEvent::~BundlePurgeEvent()
		{
		}

		void BundlePurgeEvent::raise(const dtn::data::BundleID &id, dtn::data::StatusReportBlock::REASON_CODE reason)
		{
			// raise the new event
			raiseEvent( new BundlePurgeEvent(id, reason) );
		}

		const std::string BundlePurgeEvent::getName() const
		{
			return BundlePurgeEvent::className;
		}

		std::string BundlePurgeEvent::toString() const
		{
			return className + ": purging bundle " + bundle.toString();
		}

		const std::string BundlePurgeEvent::className = "BundlePurgeEvent";
	} /* namespace core */
} /* namespace dtn */
