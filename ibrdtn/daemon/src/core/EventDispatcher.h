/*
 * EventDispatcher.h
 *
 *  Created on: 06.12.2012
 *      Author: morgenro
 */

#ifndef EVENTDISPATCHER_H_
#define EVENTDISPATCHER_H_

#include "core/Event.h"
#include "core/EventSwitch.h"
#include "core/EventReceiver.h"
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/MutexLock.h>
#include <list>

namespace dtn
{
	namespace core
	{
		template<class E>
		class EventDispatcher {
		private:
			/**
			 * never create a dispatcher
			 */
			EventDispatcher() : _processor(*this)
			{ };

			class EventProcessorImpl : public EventProcessor {
			public:
				EventProcessorImpl(EventDispatcher<E> &dispatcher)
				: _dispatcher(dispatcher) { };

				virtual ~EventProcessorImpl() { };

				void process(const Event *evt)
				{
					ibrcommon::MutexLock l(_dispatcher._dispatch_lock);
					for (std::list<EventReceiver*>::iterator iter = _dispatcher._receivers.begin();
							iter != _dispatcher._receivers.end(); iter++)
					{
						EventReceiver &receiver = (**iter);
						receiver.raiseEvent(evt);
					}
				}

				EventDispatcher<E> &_dispatcher;
			};

			/**
			 * deliver this event to all subscribers
			 */
			void _raise(Event *evt, bool detach)
			{
				if (!detach)
				{
					_processor.process(evt);
					delete evt;
				}
				else
				{
					// raise the new event
					dtn::core::EventSwitch::queue( _processor, evt );
				}
			}

			void _add(EventReceiver *receiver) {
				ibrcommon::MutexLock l(_dispatch_lock);
				_receivers.push_back(receiver);
			}

			void _remove(const EventReceiver *receiver) {
				ibrcommon::MutexLock l(_dispatch_lock);
				for (std::list<EventReceiver*>::iterator iter = _receivers.begin(); iter != _receivers.end(); iter++)
				{
					if ((*iter) == receiver)
					{
						_receivers.erase(iter);
						break;
					}
				}
			}

			static EventDispatcher<E>& instance() {
				static EventDispatcher<E> i;
				return i;
			}

		public:
			virtual ~EventDispatcher() { };

			/**
			 * deliver this event to all subscribers
			 */
			static void raise(Event *evt, bool detach = true) {
				instance()._raise(evt, detach);
			}

			static void add(EventReceiver *receiver) {
				instance()._add(receiver);
			}

			static void remove(const EventReceiver *receiver) {
				instance()._remove(receiver);
			}

		private:
			ibrcommon::Mutex _dispatch_lock;
			std::list<EventReceiver*> _receivers;
			EventProcessorImpl _processor;
		};
	}
}

#endif /* EVENTDISPATCHER_H_ */
