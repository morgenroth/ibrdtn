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
		TimeEvent::TimeEvent(const size_t timestamp, const size_t unixtimestamp, const TimeEventAction action)
		: m_timestamp(timestamp), m_unixtimestamp(unixtimestamp), m_action(action)
		{

		}

		TimeEvent::~TimeEvent()
		{

		}

		TimeEventAction TimeEvent::getAction() const
		{
			return m_action;
		}

		size_t TimeEvent::getTimestamp() const
		{
			return m_timestamp;
		}

		size_t TimeEvent::getUnixTimestamp() const
		{
			return m_unixtimestamp;
		}

		const std::string TimeEvent::getName() const
		{
			return TimeEvent::className;
		}

		void TimeEvent::raise(const size_t timestamp, const size_t unixtimestamp, const TimeEventAction action)
		{
			// raise the new event
			dtn::core::EventDispatcher<TimeEvent>::raise( new TimeEvent(timestamp, unixtimestamp, action) );
		}

		std::string TimeEvent::getMessage() const
		{
			if (getAction() == TIME_SECOND_TICK) {
				std::stringstream ss; ss << getTimestamp();
				return "new timestamp is " + ss.str();
			}
			return "unknown";
		}

		const std::string TimeEvent::className = "TimeEvent";
	}
}
