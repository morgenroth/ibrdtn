/*
 * TimeEvent.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef TIMEEVENT_H_
#define TIMEEVENT_H_

#include "core/Event.h"

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
			size_t getTimestamp() const;
			size_t getUnixTimestamp() const;
			const std::string getName() const;

			static void raise(const size_t timestamp, const size_t unixtimestamp, const TimeEventAction action);

			std::string toString() const;

			static const std::string className;

		private:
			TimeEvent(const size_t timestamp, const size_t unixtimestamp, const TimeEventAction action);

			const size_t m_timestamp;
			const size_t m_unixtimestamp;
			const TimeEventAction m_action;
		};
	}
}

#endif /* TIMEEVENT_H_ */
