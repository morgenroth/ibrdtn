/*
 * Event.h
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

#ifndef EVENT_H_
#define EVENT_H_

#include "core/EventReceiver.h"
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Conditional.h>

namespace dtn
{
	namespace core
	{
		class Event
		{
		public:
			virtual ~Event() = 0;
			virtual const std::string getName() const = 0;

			virtual std::string toString() const = 0;

			void set_ref_count(size_t c);
			bool decrement_ref_count();

			const int prio;

		protected:
			Event(int prio = 0);
			static void raiseEvent(Event *evt, bool block_until_processed = false);

		private:
			ibrcommon::Conditional _ref_count_mutex;
			size_t _ref_count;

			// mark the event for auto deletion if the last reference is gone
			bool _auto_delete;

			// mark the event as processed
			bool _processed;
		};
	}
}

#endif /* EVENT_H_ */
