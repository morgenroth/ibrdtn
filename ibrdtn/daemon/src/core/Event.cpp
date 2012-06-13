/*
 * Event.cpp
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
