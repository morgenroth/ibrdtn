/*
 * Event.cpp
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
 */

#include "core/Event.h"
#include "core/EventSwitch.h"
#include <ibrcommon/thread/MutexLock.h>

namespace dtn
{
	namespace core
	{
		Event::Event(int p) : prio(p), _ref_count(1), _auto_delete(true), _processed(false) { }
		Event::~Event() {};

		void Event::raiseEvent(Event *evt, bool block_until_processed)
		{
			if (block_until_processed)
			{
				evt->_auto_delete = false;
			}

			// raise the new event
			dtn::core::EventSwitch::raiseEvent( evt );

			// if this is a blocking event...
			if (block_until_processed)
			{
				{
					ibrcommon::MutexLock l(evt->_ref_count_mutex);
					while (!evt->_processed) evt->_ref_count_mutex.wait();
				}

				// ... then delete the event here
				delete evt;
			}
		}

		void Event::set_ref_count(size_t c)
		{
			ibrcommon::MutexLock l(_ref_count_mutex);
			_ref_count = c;
		}

		bool Event::decrement_ref_count()
		{
			ibrcommon::MutexLock l(_ref_count_mutex);
			if (_ref_count == 0)
			{
				throw ibrcommon::Exception("This reference count is already zero!");
			}

			_ref_count--;
			_processed = (_ref_count == 0);

			if (_auto_delete)
			{
				return _processed;
			}
			else
			{
				_ref_count_mutex.signal(true);
				return false;
			}
		}
	}
}
