/*
 * TimeEvent.cpp
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

#include "core/TimeEvent.h"
#include "core/EventDispatcher.h"
#include <sstream>

namespace dtn
{
	namespace core
	{
		TimeEvent::TimeEvent(const dtn::data::Timestamp &timestamp, const TimeEventAction action)
		: m_timestamp(timestamp), m_action(action)
		{

		}

		TimeEvent::~TimeEvent()
		{

		}

		TimeEventAction TimeEvent::getAction() const
		{
			return m_action;
		}

		const dtn::data::Timestamp& TimeEvent::getTimestamp() const
		{
			return m_timestamp;
		}

		const std::string TimeEvent::getName() const
		{
			return "TimeEvent";
		}

		void TimeEvent::raise(const dtn::data::Timestamp &timestamp, const TimeEventAction action)
		{
			// raise the new event
			dtn::core::EventDispatcher<TimeEvent>::raise( new TimeEvent(timestamp, action) );
		}

		std::string TimeEvent::getMessage() const
		{
			if (getAction() == TIME_SECOND_TICK) {
				return "new timestamp is " + getTimestamp().toString();
			}
			return "unknown";
		}
	}
}
