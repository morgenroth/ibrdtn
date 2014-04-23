/*
 * TimeEvent.h
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

#ifndef TIMEEVENT_H_
#define TIMEEVENT_H_

#include "core/Event.h"
#include <ibrdtn/data/Number.h>

namespace dtn
{
	namespace core
	{
		enum TimeEventAction
		{
			TIME_SECOND_TICK = 0
		};

		class TimeEvent : public Event
		{
		public:
			virtual ~TimeEvent();

			TimeEventAction getAction() const;
			const dtn::data::Timestamp& getTimestamp() const;
			const std::string getName() const;

			static void raise(const dtn::data::Timestamp &timestamp, const TimeEventAction action);

			std::string getMessage() const;

		private:
			TimeEvent(const dtn::data::Timestamp &timestamp, const TimeEventAction action);

			const dtn::data::Timestamp m_timestamp;
			const TimeEventAction m_action;
		};
	}
}

#endif /* TIMEEVENT_H_ */
