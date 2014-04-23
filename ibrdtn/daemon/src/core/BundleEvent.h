/*
 * BundleEvent.h
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
			
			std::string getMessage() const;

			static void raise(const dtn::data::MetaBundle &bundle, EventBundleAction action, dtn::data::StatusReportBlock::REASON_CODE reason = dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);

		private:
			BundleEvent(const dtn::data::MetaBundle &bundle, EventBundleAction action, dtn::data::StatusReportBlock::REASON_CODE reason = dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);
			const dtn::data::MetaBundle m_bundle;
			EventBundleAction m_action;
			dtn::data::StatusReportBlock::REASON_CODE m_reason;
		};
	}
}

#endif /* BUNDLEEVENT_H_ */
