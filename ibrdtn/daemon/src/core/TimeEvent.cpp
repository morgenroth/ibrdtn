/*
 * TimeEvent.cpp
 *
 *  Created on: 16.03.2009
 *      Author: morgenro
 */

#include "core/TimeEvent.h"

namespace dtn
{
	namespace core
	{
		TimeEvent::TimeEvent(const size_t timestamp, const size_t unixtimestamp, const TimeEventAction action)
		: m_timestamp(timestamp), m_unixtimestamp(unixtimestamp), m_action(action)
		{

		}

		TimeEvent::~TimeEvent()
		{

		}

		TimeEventAction TimeEvent::getAction() const
		{
			return m_action;
		}

		size_t TimeEvent::getTimestamp() const
		{
			return m_timestamp;
		}

		size_t TimeEvent::getUnixTimestamp() const
		{
			return m_unixtimestamp;
		}

		const std::string TimeEvent::getName() const
		{
			return TimeEvent::className;
		}

		void TimeEvent::raise(const size_t timestamp, const size_t unixtimestamp, const TimeEventAction action)
		{
			// raise the new event
			raiseEvent( new TimeEvent(timestamp, unixtimestamp, action) );
		}

		std::string TimeEvent::toString() const
		{
			return TimeEvent::className;
		}

		const std::string TimeEvent::className = "TimeEvent";
	}
}
