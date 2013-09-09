/*
 * TimeAdjustmentEvent.cpp
 *
 *  Created on: 11.09.2012
 *      Author: morgenro
 */

#include "core/TimeAdjustmentEvent.h"
#include "core/EventDispatcher.h"
#include <ibrdtn/utils/Clock.h>
#include <sstream>
#include <sys/time.h>

namespace dtn
{
	namespace core
	{
		TimeAdjustmentEvent::TimeAdjustmentEvent(const timeval &o, const double &r)
		 : offset(o), rating(r)
		{
		}

		TimeAdjustmentEvent::~TimeAdjustmentEvent()
		{
		}

		void TimeAdjustmentEvent::raise(const timeval &offset, const double &rating)
		{
			dtn::core::EventDispatcher<TimeAdjustmentEvent>::raise( new TimeAdjustmentEvent(offset, rating) );
		}

		const std::string TimeAdjustmentEvent::getName() const
		{
			return className;
		}

		std::string TimeAdjustmentEvent::getMessage() const
		{
			std::stringstream ss;
			ss << "time adjusted by " << dtn::utils::Clock::toDouble(offset) << "s, new rating is " << rating;
			return ss.str();
		}

		const std::string TimeAdjustmentEvent::className = "TimeAdjustmentEvent";
	} /* namespace core */
} /* namespace dtn */
