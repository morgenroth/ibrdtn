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
			dtn::core::EventDispatcher<TimeAdjustmentEvent>::queue( new TimeAdjustmentEvent(offset, rating) );
		}

		const std::string TimeAdjustmentEvent::getName() const
		{
			return "TimeAdjustmentEvent";
		}

		std::string TimeAdjustmentEvent::getMessage() const
		{
			std::stringstream ss;
			ss << "time adjusted by " << dtn::utils::Clock::toDouble(offset) << "s, based on clock with rating " << rating;
			return ss.str();
		}
	} /* namespace core */
} /* namespace dtn */
