/*
 * EventSwitch.h
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

#ifndef EVENTSWITCH_H_
#define EVENTSWITCH_H_

#include "Component.h"
#include "core/Event.h"
#include <ibrcommon/Exceptions.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Conditional.h>

#include <list>
#include <map>

namespace dtn
{
	namespace core
	{
		class EventReceiver;

		class EventSwitch : public dtn::daemon::IntegratedComponent
		{
		private:
			EventSwitch();
			virtual ~EventSwitch();

			bool _running;
			bool _shutdown;

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			class Task
			{
			public:
				Task(EventProcessor &proc, dtn::core::Event *evt);
				~Task();

				EventProcessor &processor;
				dtn::core::Event *event;
			};

			class Worker : public ibrcommon::JoinableThread
			{
			public:
				Worker(EventSwitch &sw);
				~Worker();

			protected:
				void run() throw ();
				virtual void __cancellation() throw ();

			private:
				EventSwitch &_switch;
				bool _running;
			};

			/**
			 * checks if all the queues are empty
			 */
			bool empty();

			ibrcommon::Conditional _queue_cond;
			std::list<Task*> _queue;
			std::list<Task*> _prio_queue;
			std::list<Task*> _low_queue;

			void process();

		protected:
			virtual void componentUp() throw ();
			virtual void componentDown() throw ();

		public:
			static EventSwitch& getInstance();
			void loop(size_t threads = 0);

			/**
			 * Bring down the EventSwitch.
			 * Do not accept any further events, but deliver all events in the queue.
			 */
			void shutdown();

			//static void raiseEvent(Event *evt);
			static void queue(EventProcessor &proc, Event *evt);

			friend class Worker;
		};
	}
}

#endif /* EVENTSWITCH_H_ */
