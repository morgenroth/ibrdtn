/*
 * Event.h
 *
 *  Created on: 05.03.2009
 *      Author: morgenro
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
