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
#include <ibrcommon/thread/RWMutex.h>
#include <ibrcommon/thread/RWLock.h>
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
			EventDispatcher() : _processor(*this), _stat_count(0)
			{ };

			class EventProcessorImpl : public EventProcessor {
			public:
				EventProcessorImpl(EventDispatcher<E> &dispatcher)
				: _dispatcher(dispatcher) { };

				virtual ~EventProcessorImpl() { };

				void process(const Event *evt)
				{
					ibrcommon::MutexLock l(_dispatcher._dispatch_lock);
					for (typename std::list<EventReceiver<E>*>::iterator iter = _dispatcher._receivers.begin();
							iter != _dispatcher._receivers.end(); ++iter)
					{
						EventReceiver<E> &receiver = (**iter);
						receiver.raiseEvent(static_cast<const E&>(*evt));
					}

					_dispatcher._stat_count++;
				}

				EventDispatcher<E> &_dispatcher;
			};

			void _reset() {
				ibrcommon::RWLock l(_dispatch_lock);
				_stat_count = 0;
			}

			/**
			 * Directly deliver this event to all subscribers
			 */
			void _raise(E *evt)
			{
				_processor.process(evt);
				delete evt;
			}

			/**
			 * Queue the event for later delivery
			 */
			void _queue(E *evt)
			{
				// raise the new event
				dtn::core::EventSwitch::queue( _processor, evt );
			}

			void _add(EventReceiver<E> *receiver) {
				ibrcommon::RWLock l(_dispatch_lock);
				_receivers.push_back(receiver);
			}

			void _remove(const EventReceiver<E> *receiver) {
				ibrcommon::RWLock l(_dispatch_lock);
				for (typename std::list<EventReceiver<E>*>::iterator iter = _receivers.begin(); iter != _receivers.end(); ++iter)
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
			 * Directly deliver this event to all subscribers
			 */
			static void raise(E *evt) {
				instance()._raise(evt);
			}

			/**
			 * Queue the event for later delivery
			 */
			static void queue(E *evt) {
				instance()._queue(evt);
			}

			static void add(EventReceiver<E> *receiver) {
				instance()._add(receiver);
			}

			static void remove(const EventReceiver<E> *receiver) {
				instance()._remove(receiver);
			}

			static void resetCounter() {
				instance()._reset();
			}

			static size_t getCounter() {
				return instance()._stat_count;
			}

		private:
			ibrcommon::RWMutex _dispatch_lock;
			std::list<EventReceiver<E>*> _receivers;
			EventProcessorImpl _processor;
			size_t _stat_count;
		};
	}
}

#endif /* EVENTDISPATCHER_H_ */
