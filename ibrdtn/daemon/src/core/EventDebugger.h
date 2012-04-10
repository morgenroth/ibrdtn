/*
 * EventDebugger.h
 *
 *  Created on: 06.03.2009
 *      Author: morgenro
 */

#ifndef EVENTDEBUGGER_H_
#define EVENTDEBUGGER_H_

#include "core/EventReceiver.h"

namespace dtn
{
	namespace core
	{
		class EventDebugger : public EventReceiver
		{
		public:
			EventDebugger();
			virtual ~EventDebugger();

			void raiseEvent(const Event *evt);
		};
	}
}

#endif /* EVENTDEBUGGER_H_ */
