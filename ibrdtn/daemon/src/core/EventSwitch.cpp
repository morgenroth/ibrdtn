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
		 : _running(true), _active_worker(0), _pause(false)
		{
		}

		EventSwitch::~EventSwitch()
		{
			componentDown();
		}

		void EventSwitch::componentUp()
		{
		}

		void EventSwitch::componentDown()
		{
			try {
				ibrcommon::MutexLock l(_queue_cond);
				_running = false;

				// wait until the queue is empty
				while (!_queue.empty())
				{
					_queue_cond.wait();
				}

				_queue_cond.abort();
			} catch (const ibrcommon::Conditional::ConditionalAbortException&) {};
		}

		void EventSwitch::process()
		{
			EventSwitch::Task *t = NULL;

			// just look for an event to process
			{
				ibrcommon::MutexLock l(_queue_cond);
				while (_queue.empty() && _prio_queue.empty() && _low_queue.empty())
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
				else
				{
					t = _low_queue.front();
					_low_queue.pop_front();
				}

				_queue_cond.signal(true);

				ibrcommon::MutexLock la(_active_cond);
				_active_worker++;
				_active_cond.signal(true);
			}

			try {
				// execute the event
				t->receiver->raiseEvent(t->event);
			} catch (const std::exception&) {};

			// delete the Task
			delete t;

			ibrcommon::MutexLock l(_active_cond);
			_active_worker--;
			_active_cond.signal(true);

			// wait while pause is enabled
			while (_pause) _active_cond.wait();
		}

		void EventSwitch::loop(size_t threads)
		{
			// create worker threads
			std::list<Worker*> wlist;

			for (size_t i = 0; i < threads; i++)
			{
				Worker *w = new Worker(*this);
				w->start();
				wlist.push_back(w);
			}

			try {
				while (_running || (!_queue.empty()))
				{
					process();
				}
			} catch (const ibrcommon::Conditional::ConditionalAbortException&) { };

			for (std::list<Worker*>::iterator iter = wlist.begin(); iter != wlist.end(); iter++)
			{
				Worker *w = (*iter);
				w->join();
				delete w;
			}
		}
		
		void EventSwitch::pause()
		{
			// wait until all workers are done
			ibrcommon::MutexLock la(_active_cond);
			_pause = true;
			while (_active_worker != 0) { _active_cond.wait(); };
		}

		void EventSwitch::unpause()
		{
			ibrcommon::MutexLock la(_active_cond);
			_pause = false;
			_active_cond.signal();
		}

		const list<EventReceiver*>& EventSwitch::getReceivers(string eventName) const
		{
			map<string,list<EventReceiver*> >::const_iterator iter = _list.find(eventName);

			if (iter == _list.end())
			{
				throw NoReceiverFoundException();
			}

			return iter->second;
		}

		void EventSwitch::registerEventReceiver(string eventName, EventReceiver *receiver)
		{
			// get the list for this event
			EventSwitch &s = EventSwitch::getInstance();
			ibrcommon::MutexLock l(s._receiverlock);
			s._list[eventName].push_back(receiver);
		}

		void EventSwitch::unregisterEventReceiver(string eventName, EventReceiver *receiver)
		{
			// unregister the receiver
			EventSwitch::getInstance().unregister(eventName, receiver);
		}

		void EventSwitch::unregister(std::string eventName, EventReceiver *receiver)
		{
			{
				// remove the receiver from the list
				ibrcommon::MutexLock lr(_receiverlock);
				std::list<EventReceiver*> &rlist = _list[eventName];
				for (std::list<EventReceiver*>::iterator iter = rlist.begin(); iter != rlist.end(); iter++)
				{
					if ((*iter) == receiver)
					{
						rlist.erase(iter);
						break;
					}
				}
			}

			// set the event switch into pause mode and wait
			// until all threads are on hold
			pause();

			{
				// freeze the queue
				ibrcommon::MutexLock lq(_queue_cond);

				// remove all elements with this receiver from the queue
				for (std::list<Task*>::iterator iter = _queue.begin(); iter != _queue.end();)
				{
					EventSwitch::Task &t = (**iter);
					std::list<Task*>::iterator current = iter++;
					if (t.receiver == receiver)
					{
						_queue.erase(current);
					}
				}
			}

			// resume all threads
			unpause();
		}

		void EventSwitch::raiseEvent(Event *evt)
		{
			EventSwitch &s = EventSwitch::getInstance();

			// do not process any event if the system is going down
			{
				ibrcommon::MutexLock l(s._queue_cond);
				if (!s._running)
				{
					if (evt->decrement_ref_count())
					{
						delete evt;
					}

					return;
				}
			}

			// forward to debugger
			s._debugger.raiseEvent(evt);

			try {
				dtn::core::GlobalEvent &global = dynamic_cast<dtn::core::GlobalEvent&>(*evt);

				if (global.getAction() == dtn::core::GlobalEvent::GLOBAL_SHUTDOWN)
				{
					// stop receiving events
					try {
						ibrcommon::MutexLock l(s._queue_cond);
						s._running = false;
						s._queue_cond.abort();
					} catch (const ibrcommon::Conditional::ConditionalAbortException&) {};
				}
			} catch (const std::bad_cast&) { }

			try {
				ibrcommon::MutexLock reglock(s._receiverlock);

				// get the list for this event
				const std::list<EventReceiver*> receivers = s.getReceivers(evt->getName());
				evt->set_ref_count(receivers.size());

				for (list<EventReceiver*>::const_iterator iter = receivers.begin(); iter != receivers.end(); iter++)
				{
					Task *t = new Task(*iter, evt);
					ibrcommon::MutexLock l(s._queue_cond);

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
			} catch (const NoReceiverFoundException&) {
				// No receiver available!
			}
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

		void EventSwitch::clear()
		{
			ibrcommon::MutexLock l(_receiverlock);
			_list.clear();
		}

		EventSwitch::Task::Task()
		 : receiver(NULL), event(NULL)
		{
		}

		EventSwitch::Task::Task(EventReceiver *er, dtn::core::Event *evt)
		 : receiver(er), event(evt)
		{
		}

		EventSwitch::Task::~Task()
		{
			if (event != NULL)
			{
				if (event->decrement_ref_count())
				{
					delete event;
				}
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

