/*
 * BundlePurgeEvent.h
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
			std::string getMessage() const;

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
