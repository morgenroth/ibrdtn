/*
 * TransferAbortedEvent.h
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

#ifndef TRANSFERABORTEDEVENT_H_
#define TRANSFERABORTEDEVENT_H_

#include "core/Event.h"
#include "ibrdtn/data/BundleID.h"
#include "ibrdtn/data/EID.h"
#include <string>

namespace dtn
{
	namespace net
	{
		class TransferAbortedEvent : public dtn::core::Event
		{
		public:
			enum AbortReason
			{
				REASON_UNDEFINED = 0,
				REASON_CONNECTION_DOWN = 1,
				REASON_REFUSED = 2,
				REASON_RETRY_LIMIT_REACHED = 3,
				REASON_BUNDLE_DELETED = 4,
				REASON_REFUSED_BY_FILTER = 5
			};

			virtual ~TransferAbortedEvent();

			const std::string getName() const;

			std::string getMessage() const;

			static void raise(const dtn::data::EID &peer, const dtn::data::BundleID &id, const AbortReason reason = REASON_UNDEFINED);

			const dtn::data::EID& getPeer() const;
			const dtn::data::BundleID& getBundleID() const;

			const AbortReason reason;

		private:
			static const std::string getReason(const AbortReason reason);

			const dtn::data::EID _peer;
			const dtn::data::BundleID _bundle;
			TransferAbortedEvent(const dtn::data::EID &peer, const dtn::data::BundleID &id, const AbortReason reason);
		};
	}
}

#endif /* TRANSFERABORTEDEVENT_H_ */
