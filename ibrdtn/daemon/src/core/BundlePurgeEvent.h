/*
 * BundlePurgeEvent.h
 *
 *  Created on: 13.04.2012
 *      Author: morgenro
 */

#ifndef BUNDLEPURGEEVENT_H_
#define BUNDLEPURGEEVENT_H_

#include "core/Event.h"
#include <ibrdtn/data/BundleID.h>
#include <ibrdtn/data/StatusReportBlock.h>

namespace dtn
{
	namespace core
	{
		class BundlePurgeEvent : public Event
		{
		public:
			virtual ~BundlePurgeEvent();

			const std::string getName() const;
			std::string toString() const;

			const dtn::data::BundleID bundle;
			const dtn::data::StatusReportBlock::REASON_CODE reason;

			static void raise(const dtn::data::BundleID &id, dtn::data::StatusReportBlock::REASON_CODE reason = dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);
			static const std::string className;

		private:
			BundlePurgeEvent(const dtn::data::BundleID &id, dtn::data::StatusReportBlock::REASON_CODE reason);
		};
	} /* namespace core */
} /* namespace dtn */
#endif /* BUNDLEPURGEEVENT_H_ */
