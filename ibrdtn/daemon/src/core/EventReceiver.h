/*
 * EventReceiver.h
 *
 *  Created on: 05.03.2009
 *      Author: morgenro
 */

#ifndef EVENTRECEIVER_H_
#define EVENTRECEIVER_H_

#include <string>

namespace dtn
{
	namespace core
	{
		class Event;

		class EventReceiver
		{
		public:
			virtual ~EventReceiver() = 0;
			virtual void raiseEvent(const Event *evt) = 0;

		protected:
			void bindEvent(std::string eventName);
			void unbindEvent(std::string eventName);
		};
	}
}

#endif /* EVENTRECEIVER_H_ */
