/*
 * Timer.h
 *
 *  Created on: 29.09.2009
 *      Author: morgenro
 */

#ifndef IBRCOMMON_TIMER_H_
#define IBRCOMMON_TIMER_H_

#include "ibrcommon/thread/Thread.h"
#include "ibrcommon/thread/Conditional.h"
#include <map>
#include <set>

#include <string>
#include <iostream>

namespace ibrcommon
{
	class Timer;
	class TimerCallback
	{
	public:
		/**
		 * This method will be called if the timer timed out.
		 * @param timer The reference to the timer which timed out.
		 * @exception Timer::StopTimerException if thrown will abort the timer
		 * @return The new timeout value.
		 */
		virtual size_t timeout(Timer *timer) = 0;
	};

	class Timer : public JoinableThread
	{
	public:
		typedef size_t time_t;
		static time_t get_current_time();

		/**
		 * Constructor for this timer
		 * @param timeout Timeout in seconds
		 */
		Timer(TimerCallback &callback, size_t timeout = 0);

		virtual ~Timer();

		void set(size_t timeout);

		/**
		 * This method resets the timer.
		 */
		void reset();

		/**
		 * This method stops the timer.
		 * Use reset() to start it again.
		 */
		void pause();

		/**
		 * This method returns the current timeout
		 */
		size_t getTimeout() const;

	protected:
		void run();
		void __cancellation();

	private:
		enum TIMER_STATE
		{
			TIMER_UNSET = 0,
			TIMER_RUNNING = 1,
			TIMER_RESET = 2,
			TIMER_STOPPED = 3,
			TIMER_CANCELLED = 4
		};

		StatefulConditional<Timer::TIMER_STATE, TIMER_STOPPED> _state;
		TimerCallback &_callback;
		size_t _timeout;
	public:
		class StopTimerException : public ibrcommon::Exception
		{
		public:
			StopTimerException(const std::string& what = "") : Exception(what)
			{
			}
		};
	};
}

#endif /* TIMER_H_ */
