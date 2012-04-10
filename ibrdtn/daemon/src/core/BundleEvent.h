/*
 * BundleEvent.h
 *
 *  Created on: 06.03.2009
 *      Author: morgenro
 */

#ifndef BUNDLEEVENT_H_
#define BUNDLEEVENT_H_

#include <string>
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/MetaBundle.h"
#include "core/Event.h"
#include "ibrdtn/data/StatusReportBlock.h"

namespace dtn
{
	namespace core
	{
		enum EventBundleAction
		{
			BUNDLE_DELETED = 0,
			BUNDLE_CUSTODY_ACCEPTED = 1,
			BUNDLE_FORWARDED = 2,
			BUNDLE_DELIVERED = 3,
			BUNDLE_RECEIVED = 4,
			BUNDLE_STORED = 5
		};

		class BundleEvent : public Event
		{
		public:
			virtual ~BundleEvent();

			dtn::data::StatusReportBlock::REASON_CODE getReason() const;
			EventBundleAction getAction() const;
			const dtn::data::MetaBundle& getBundle() const;
			const std::string getName() const;
			
			std::string toString() const;

			static void raise(const dtn::data::MetaBundle &bundle, EventBundleAction action, dtn::data::StatusReportBlock::REASON_CODE reason = dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);

			static const std::string className;

		private:
			BundleEvent(const dtn::data::MetaBundle &bundle, EventBundleAction action, dtn::data::StatusReportBlock::REASON_CODE reason = dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);
			const dtn::data::MetaBundle m_bundle;
			EventBundleAction m_action;
			dtn::data::StatusReportBlock::REASON_CODE m_reason;
		};
	}
}

#endif /* BUNDLEEVENT_H_ */
