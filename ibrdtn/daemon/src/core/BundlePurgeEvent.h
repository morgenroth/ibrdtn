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
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/StatusReportBlock.h>

namespace dtn
{
	namespace core
	{
		class BundlePurgeEvent : public Event
		{
		public:
			enum REASON_CODE
			{
				DELIVERED = 0,
				NO_ROUTE_KNOWN = 1,
				ACK_RECIEVED = 2
			};

			virtual ~BundlePurgeEvent();

			const std::string getName() const;
			std::string getMessage() const;

			const dtn::data::MetaBundle bundle;
			const REASON_CODE reason;

			static void raise(const dtn::data::MetaBundle &meta, REASON_CODE reason = DELIVERED);

		private:
			BundlePurgeEvent(const dtn::data::MetaBundle &meta, REASON_CODE reason);
		};
	} /* namespace core */
} /* namespace dtn */
#endif /* BUNDLEPURGEEVENT_H_ */
