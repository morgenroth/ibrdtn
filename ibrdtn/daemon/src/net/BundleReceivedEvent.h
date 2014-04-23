/*
 * BundleReceivedEvent.h
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

#ifndef BUNDLERECEIVEDEVENT_H_
#define BUNDLERECEIVEDEVENT_H_

#include "core/Event.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/EID.h"
#include <ibrcommon/thread/Semaphore.h>

namespace dtn
{
	namespace net
	{
		class BundleReceivedEvent : public dtn::core::Event
		{
		public:
			virtual ~BundleReceivedEvent();

			const string getName() const;

			std::string getMessage() const;

			static void raise(const dtn::data::EID &peer, const dtn::data::Bundle &bundle, const bool local = false);

			const dtn::data::EID peer;
			const dtn::data::Bundle bundle;
			const bool fromlocal;

		private:
			BundleReceivedEvent(const dtn::data::EID &peer, const dtn::data::Bundle &bundle, const bool local);

			static ibrcommon::Semaphore _sem;
		};
	}
}


#endif /* BUNDLERECEIVEDEVENT_H_ */
