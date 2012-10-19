/*
 * Clock.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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

		void WallClock::componentUp() throw ()
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

		void WallClock::componentDown() throw ()
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
