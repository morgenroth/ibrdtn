/*
 * Clock.cpp
 *
 *  Created on: 17.07.2009
 *      Author: morgenro
 */

#include "core/WallClock.h"
#include "core/TimeEvent.h"
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/thread/MutexLock.h>

namespace dtn
{
	namespace core
	{
		WallClock::WallClock(size_t frequency) : _frequency(frequency), _next(0), _timer(*this, frequency)
		{
		}

		WallClock::~WallClock()
		{
		}

		void WallClock::sync()
		{
			try {
				ibrcommon::MutexLock l(*this);
				wait();
			} catch (const ibrcommon::Conditional::ConditionalAbortException &ex) {

			}
		}

		void WallClock::componentUp()
		{
			if(_timer.isRunning())
			{
				_timer.reset();
			}
			else
			{
				_timer.start();
			}
		}

		void WallClock::componentDown()
		{
			_timer.pause();
		}

		size_t WallClock::timeout(ibrcommon::Timer*)
		{
			size_t dtntime = dtn::utils::Clock::getTime();

			if (dtntime == 0)
			{
				TimeEvent::raise(dtntime, dtn::utils::Clock::getUnixTimestamp(), TIME_SECOND_TICK);

				ibrcommon::MutexLock l(*this);
				signal(true);
			}
			else if (_next <= dtntime)
			{
				TimeEvent::raise(dtntime, dtn::utils::Clock::getUnixTimestamp(), TIME_SECOND_TICK);
				_next = dtntime + _frequency;

				ibrcommon::MutexLock l(*this);
				signal(true);
			}

			return _frequency;
		}

		const std::string WallClock::getName() const
		{
			return "WallClock";
		}
	}
}
