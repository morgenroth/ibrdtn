/*
 * TimeAdjustmentEvent.h
 *
 *  Created on: 11.09.2012
 *      Author: morgenro
 */

#ifndef TIMEADJUSTMENTEVENT_H_
#define TIMEADJUSTMENTEVENT_H_

#include "core/Node.h"
#include "core/Event.h"
#include <sys/time.h>

namespace dtn
{
	namespace core
	{
		class TimeAdjustmentEvent : public Event
		{
		public:
			virtual ~TimeAdjustmentEvent();

			const std::string getName() const;

			std::string toString() const;

			static void raise(const timeval &offset, const float &rating);

			static const std::string className;

			timeval offset;
			float rating;

		private:
			TimeAdjustmentEvent(const timeval &offset, const float &rating);
		};
	} /* namespace core */
} /* namespace dtn */
#endif /* TIMEADJUSTMENTEVENT_H_ */
