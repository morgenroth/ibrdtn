/*
 * EventDebugger.cpp
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

#include "core/EventDebugger.h"
#include "core/NodeEvent.h"
#include "core/CustodyEvent.h"
#include "core/BundleEvent.h"
#include "core/TimeEvent.h"
#include <ibrcommon/Logger.h>
#include <iostream>

using namespace std;
using namespace ibrcommon;

namespace dtn
{
	namespace core
	{
		EventDebugger::EventDebugger()
		{
		}

		EventDebugger::~EventDebugger()
		{
		}

		void EventDebugger::raiseEvent(const Event *evt)
		{
			const NodeEvent *node = dynamic_cast<const NodeEvent*>(evt);
			const BundleEvent *bundle = dynamic_cast<const BundleEvent*>(evt);
			const CustodyEvent *custody = dynamic_cast<const CustodyEvent*>(evt);
			const TimeEvent *time = dynamic_cast<const TimeEvent*>(evt);

			if (custody != NULL)
			{
				switch (custody->getAction())
				{
					case CUSTODY_ACCEPT:
						if (custody->getBundle().procflags & dtn::data::Bundle::CUSTODY_REQUESTED)
						{
							IBRCOMMON_LOGGER(notice) << evt->getName() << ": custody acceptance" << IBRCOMMON_LOGGER_ENDL;
						}
						break;

					case CUSTODY_REJECT:
						if (custody->getBundle().procflags & dtn::data::Bundle::CUSTODY_REQUESTED)
						{
							IBRCOMMON_LOGGER(notice) << evt->getName() << ": custody reject" << IBRCOMMON_LOGGER_ENDL;
						}
						break;
				};
			}
			else if (bundle != NULL)
			{
				switch (bundle->getAction())
				{
				case BUNDLE_DELETED:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": bundle " << bundle->getBundle().toString() << " deleted" << IBRCOMMON_LOGGER_ENDL;
					break;
				case BUNDLE_CUSTODY_ACCEPTED:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": custody accepted for " << bundle->getBundle().toString() << IBRCOMMON_LOGGER_ENDL;
					break;
				case BUNDLE_FORWARDED:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": bundle " << bundle->getBundle().toString() << " forwarded" << IBRCOMMON_LOGGER_ENDL;
					break;
				case BUNDLE_DELIVERED:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": bundle " << bundle->getBundle().toString() << " delivered" << IBRCOMMON_LOGGER_ENDL;
					break;
				case BUNDLE_RECEIVED:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": bundle " << bundle->getBundle().toString() << " received" << IBRCOMMON_LOGGER_ENDL;
					break;
				case BUNDLE_STORED:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": bundle " << bundle->getBundle().toString() << " stored" << IBRCOMMON_LOGGER_ENDL;
					break;
				default:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": unknown" << IBRCOMMON_LOGGER_ENDL;
					break;
				}
			}
			else if (node != NULL)
			{
				switch (node->getAction())
				{
				case NODE_INFO_UPDATED:
					IBRCOMMON_LOGGER_DEBUG(10) << evt->getName() << ": Info updated for " << node->getNode().toString() << IBRCOMMON_LOGGER_ENDL;
					break;
				case NODE_AVAILABLE:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": Node " << node->getNode().toString() << " available " << IBRCOMMON_LOGGER_ENDL;
					break;
				case NODE_UNAVAILABLE:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": Node " << node->getNode().toString() << " unavailable" << IBRCOMMON_LOGGER_ENDL;
					break;
				default:
					IBRCOMMON_LOGGER(notice) << evt->getName() << ": unknown" << IBRCOMMON_LOGGER_ENDL;
					break;
				}
			}
			else if (time != NULL)
			{
				// do not debug print time events
			}
			else
			{
				// unknown event
				IBRCOMMON_LOGGER(notice) << evt->toString() << IBRCOMMON_LOGGER_ENDL;
			}
		}
	}
}
