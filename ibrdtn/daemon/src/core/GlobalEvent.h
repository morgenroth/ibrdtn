/*
 * GlobalEvent.h
 *
 *  Created on: 30.10.2009
 *      Author: morgenro
 */

#ifndef GLOBALEVENT_H_
#define GLOBALEVENT_H_

#include "core/Event.h"

namespace dtn
{
	namespace core
	{
		class GlobalEvent : public Event
		{
		public:
			enum Action
			{
				GLOBAL_SHUTDOWN = 0,
				GLOBAL_RELOAD = 1,
				GLOBAL_IDLE = 2,
				GLOBAL_BUSY = 3,
				GLOBAL_SUSPEND = 4,
				GLOBAL_POWERSAVE = 5,
				GLOBAL_WAKEUP = 6
			};

			virtual ~GlobalEvent();

			const std::string getName() const;

			Action getAction() const;

			static void raise(const Action a);

			std::string toString() const;

			static const std::string className;

		private:
			GlobalEvent(const Action a);
			Action _action;
		};
	}
}

#endif /* GLOBALEVENT_H_ */
