/*
 * CustodyEvent.cpp
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

#include "core/CustodyEvent.h"
#include "core/EventDispatcher.h"
#include "ibrdtn/data/Exceptions.h"

namespace dtn
{
	namespace core
	{
		CustodyEvent::CustodyEvent(const dtn::data::MetaBundle &bundle, const EventCustodyAction action)
		: m_bundle(bundle), m_action(action)
		{
		}

		CustodyEvent::~CustodyEvent()
		{
		}

		const dtn::data::MetaBundle& CustodyEvent::getBundle() const
		{
			return m_bundle;
		}

		EventCustodyAction CustodyEvent::getAction() const
		{
			return m_action;
		}

		const std::string CustodyEvent::getName() const
		{
			return "CustodyEvent";
		}

		std::string CustodyEvent::getMessage() const
		{
			switch (getAction())
			{
				case CUSTODY_ACCEPT:
					if (getBundle().procflags & dtn::data::Bundle::CUSTODY_REQUESTED)
					{
						return "custody acceptance";
					}
					break;

				case CUSTODY_REJECT:
					if (getBundle().procflags & dtn::data::Bundle::CUSTODY_REQUESTED)
					{
						return "custody reject";
					}
					break;
			};

			return "unknown";
		}

		void CustodyEvent::raise(const dtn::data::MetaBundle &bundle, const EventCustodyAction action)
		{
			dtn::core::EventDispatcher<CustodyEvent>::queue( new CustodyEvent(bundle, action) );
		}
	}
}
