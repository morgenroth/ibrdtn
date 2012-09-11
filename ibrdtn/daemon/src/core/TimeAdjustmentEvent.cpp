/*
 * TimeAdjustmentEvent.cpp
 *
 *  Created on: 11.09.2012
 *      Author: morgenro
 */

#include "TimeAdjustmentEvent.h"

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
			raiseEvent( new TimeAdjustmentEvent(offset, rating) );
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
