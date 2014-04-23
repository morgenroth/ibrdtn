/*
 * CustodyEvent.h
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

#ifndef CUSTODYEVENT_H_
#define CUSTODYEVENT_H_

#include <string>
#include "core/Event.h"

#include <ibrdtn/data/MetaBundle.h>

namespace dtn
{
	namespace core
	{
		enum EventCustodyAction
		{
			CUSTODY_REJECT = 0,
			CUSTODY_ACCEPT = 1
		};

		class CustodyEvent : public Event
		{
		public:
			virtual ~CustodyEvent();

			EventCustodyAction getAction() const;
			const dtn::data::MetaBundle& getBundle() const;
			const std::string getName() const;

			std::string getMessage() const;

			static void raise(const dtn::data::MetaBundle &bundle, const EventCustodyAction action);

		private:
			CustodyEvent(const dtn::data::MetaBundle &bundle, const EventCustodyAction action);

			const dtn::data::MetaBundle m_bundle;
			const EventCustodyAction m_action;
		};
	}
}

#endif /* CUSTODYEVENT_H_ */
