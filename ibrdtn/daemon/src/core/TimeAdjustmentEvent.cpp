/*
 * TimeAdjustmentEvent.cpp
 *
 *  Created on: 11.09.2012
 *      Author: morgenro
 */

#include "core/TimeAdjustmentEvent.h"
#include "core/EventDispatcher.h"

#include <sys/time.h>

namespace dtn
{
	namespace core
	{
		TimeAdjustmentEvent::TimeAdjustmentEvent(const timeval &o, const float &r)
		 : offset(o), rating(r)
		{
		}

		TimeAdjustmentEvent::~TimeAdjustmentEvent()
		{
		}

		void TimeAdjustmentEvent::raise(const timeval &offset, const float &rating)
		{
			dtn::core::EventDispatcher<TimeAdjustmentEvent>::raise( new TimeAdjustmentEvent(offset, rating) );
		}

		const std::string TimeAdjustmentEvent::getName() const
		{
			return className;
		}

		std::string TimeAdjustmentEvent::toString() const
		{
			return className;
		}

		const std::string TimeAdjustmentEvent::className = "TimeAdjustmentEvent";
	} /* namespace core */
} /* namespace dtn */
