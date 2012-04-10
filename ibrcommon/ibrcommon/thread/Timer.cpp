/*
 * Timer.cpp
 *
 *  Created on: 29.09.2009
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/thread/Timer.h"
#include "ibrcommon/thread/MutexLock.h"

#include <iostream>
#include <queue>
#include <utility>
#include <sstream>

namespace ibrcommon
{
	Timer::time_t Timer::get_current_time()
	{
		::time_t rawtime = time(NULL);
		tm * ptm;

		ptm = gmtime ( &rawtime );

		return mktime(ptm);
	}

	Timer::Timer(TimerCallback &callback, size_t timeout)
	 : _state(TIMER_UNSET), _callback(callback), _timeout(timeout * 1000)
	{
	}

	Timer::~Timer()
	{
		stop();
		join();
	}

	void Timer::__cancellation()
	{
		MutexLock l(_state);
		_state.setState(TIMER_CANCELLED);
		_state.abort();
	}

	void Timer::set(size_t timeout)
	{
		MutexLock l(_state);
		_timeout = timeout * 1000;
		_state.setState(TIMER_RESET);
	}

	void Timer::reset()
	{
		MutexLock l(_state);
		_state.setState(TIMER_RESET);
	}

	void Timer::pause()
	{
		MutexLock l(_state);
		_state.setState(TIMER_STOPPED);
	}

	size_t Timer::getTimeout() const
	{
		return _timeout / 1000;
	}

	void Timer::run()
	{
		MutexLock l(_state);
		_state.setState(TIMER_RUNNING);

		while (_state.ifState(TIMER_RUNNING) || _state.ifState(TIMER_STOPPED))
		{
			try {
				if(_state.ifState(TIMER_RUNNING))
				{
					if(_timeout > 0)
					{
						_state.wait(_timeout);
					}
					else
					{
						throw ibrcommon::Conditional::ConditionalAbortException(ibrcommon::Conditional::ConditionalAbortException::COND_ABORT);
					}
				}
				else
				{
					_state.wait();
				}

				// got signal, check for reset request
				if (_state.ifState(TIMER_RESET))
				{
					_state.setState(TIMER_RUNNING);
				}
				else if(!_state.ifState(TIMER_STOPPED))
				{
					// stop the timer
					_state.setState(TIMER_CANCELLED);
				}
			}
			catch (const ibrcommon::Conditional::ConditionalAbortException &ex)
			{
				switch (ex.reason)
				{
					case ibrcommon::Conditional::ConditionalAbortException::COND_TIMEOUT:
					{
						try
						{
							// timeout exceeded, call callback method
							_timeout = _callback.timeout(this) * 1000;
							_state.setState(TIMER_RUNNING);
						}
						catch(const StopTimerException &ex)
						{
							// stop the timer
							_state.setState(TIMER_STOPPED);
						}
						break;
					}

					case ibrcommon::Conditional::ConditionalAbortException::COND_ABORT:
					case ibrcommon::Conditional::ConditionalAbortException::COND_ERROR:
					{
						_state.setState(TIMER_CANCELLED);
						return;
					}
				}
			}

			yield();
		}
	}
}
