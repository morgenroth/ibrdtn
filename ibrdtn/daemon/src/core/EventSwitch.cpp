/*
 * EventSwitch.cpp
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

#include "core/EventSwitch.h"
#include "core/EventReceiver.h"
#include "core/EventDispatcher.h"

#include <ibrcommon/thread/MutexLock.h>
#include "core/GlobalEvent.h"
#include <ibrcommon/Logger.h>
#include <stdexcept>
#include <iostream>
#include <typeinfo>

namespace dtn
{
	namespace core
	{
		EventSwitch::EventSwitch()
		 : _running(true), _shutdown(false)
		{
		}

		EventSwitch::~EventSwitch()
		{
			componentDown();
		}

		void EventSwitch::componentUp() throw ()
		{
			// routine checked for throw() on 15.02.2013

			// clear all queues
			_queue.clear();
			_prio_queue.clear();
			_low_queue.clear();

			// reset component state
			_running = true;
			_shutdown = false;

			// reset aborted conditional
			_queue_cond.reset();
		}

		void EventSwitch::componentDown() throw ()
		{
			try {
				ibrcommon::MutexLock l(_queue_cond);

				// stop receiving events
				_shutdown = true;

				// wait until all queues are empty
				while (!this->empty())
				{
					_queue_cond.wait();
				}

				_queue_cond.abort();
			} catch (const ibrcommon::Conditional::ConditionalAbortException&) {};
		}

		bool EventSwitch::empty()
		{
			return (_low_queue.empty() && _queue.empty() && _prio_queue.empty());
		}

		void EventSwitch::process()
		{
			EventSwitch::Task *t = NULL;

			// just look for an event to process
			{
				ibrcommon::MutexLock l(_queue_cond);
				if (!_running) return;

				while (this->empty() && !_shutdown)
				{
					_queue_cond.wait();
				}

				if (!_prio_queue.empty())
				{
//					IBRCOMMON_LOGGER_DEBUG(1) << "process element of priority queue" << IBRCOMMON_LOGGER_ENDL;
					t = _prio_queue.front();
					_prio_queue.pop_front();
				}
				else if (!_queue.empty())
				{
					t = _queue.front();
					_queue.pop_front();
				}
				else if (!_low_queue.empty())
				{
					t = _low_queue.front();
					_low_queue.pop_front();
				}
				else if (_shutdown)
				{
					// if all queues are empty and shutdown is requested
					// set running mode to false
					_running = false;

					// abort the conditional to release all blocking threads
					_queue_cond.abort();

					return;
				}

				_queue_cond.signal(true);
			}

			if (t != NULL)
			{
				// execute the event
				t->processor.process(t->event);

				// delete the Task
				delete t;
			}
		}

		void EventSwitch::loop(size_t threads)
		{
			// create worker threads
			std::list<Worker*> wlist;

			for (size_t i = 0; i < threads; ++i)
			{
				Worker *w = new Worker(*this);
				w->start();
				wlist.push_back(w);
			}

			try {
				while (_running)
				{
					process();
				}
			} catch (const ibrcommon::Conditional::ConditionalAbortException&) { };

			for (std::list<Worker*>::iterator iter = wlist.begin(); iter != wlist.end(); ++iter)
			{
				Worker *w = (*iter);
				w->stop();
				w->join();
				delete w;
			}
		}

		void EventSwitch::queue(EventProcessor &proc, Event *evt)
		{
			EventSwitch &s = EventSwitch::getInstance();

			ibrcommon::MutexLock l(s._queue_cond);

			// do not process any event if the system is going down
			if (s._shutdown)
			{
				delete evt;
				return;
			}

			EventSwitch::Task *t = new EventSwitch::Task(proc, evt);

			if (evt->prio > 0)
			{
				s._prio_queue.push_back(t);
			}
			else if (evt->prio < 0)
			{
				s._low_queue.push_back(t);
			}
			else
			{
				s._queue.push_back(t);
			}
			s._queue_cond.signal();
		}

		void EventSwitch::shutdown()
		{
			try {
				ibrcommon::MutexLock l(_queue_cond);

				// stop receiving events
				_shutdown = true;

				// signal all blocking thread to check _shutdown variable
				_queue_cond.signal(true);
			} catch (const ibrcommon::Conditional::ConditionalAbortException&) {};
		}

		EventSwitch& EventSwitch::getInstance()
		{
			static EventSwitch instance;
			return instance;
		}

		const std::string EventSwitch::getName() const
		{
			return "EventSwitch";
		}

		EventSwitch::Task::Task(EventProcessor &proc, dtn::core::Event *evt)
		 : processor(proc), event(evt)
		{
		}

		EventSwitch::Task::~Task()
		{
			if (event != NULL)
			{
				delete event;
			}
		}

		EventSwitch::Worker::Worker(EventSwitch &sw)
		 : _switch(sw), _running(true)
		{}

		EventSwitch::Worker::~Worker()
		{}

		void EventSwitch::Worker::run() throw ()
		{
			try {
				while (_running)
					_switch.process();
			} catch (const ibrcommon::Conditional::ConditionalAbortException&) { };
		}

		void EventSwitch::Worker::__cancellation() throw ()
		{
			_running = false;
		}
	}
}

