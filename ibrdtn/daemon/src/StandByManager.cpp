/*
 * StandByManager.cpp
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

#include "StandByManager.h"
#include "core/GlobalEvent.h"
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>
#include <typeinfo>

namespace dtn {
	namespace daemon {
		StandByManager::StandByManager()
		 : _suspended(false), _enabled(false)
		{
		}

		StandByManager::~StandByManager()
		{
		}

		void StandByManager::adopt(Component *c)
		{
			ibrcommon::MutexLock l(_lock);
			_components.push_back(c);

			IBRCOMMON_LOGGER(info) << "StandByManager: " << c->getName() << " adopted" << IBRCOMMON_LOGGER_ENDL;
		}

		void StandByManager::raiseEvent(const dtn::core::Event *evt) throw ()
		{
			try {
				const dtn::core::GlobalEvent &global = dynamic_cast<const dtn::core::GlobalEvent&>(*evt);

				switch (global.getAction())
				{
				case dtn::core::GlobalEvent::GLOBAL_SUSPEND:
					suspend();
					break;

				case dtn::core::GlobalEvent::GLOBAL_WAKEUP:
					wakeup();
					break;

				default:
					break;
				}
			} catch (const bad_cast&) { };
		}

		void StandByManager::wakeup()
		{
			ibrcommon::MutexLock l(_lock);
			if ((!_suspended) || (!_enabled)) return;
			IBRCOMMON_LOGGER(info) << "StandByManager: wake-up components" << IBRCOMMON_LOGGER_ENDL;

			for (std::list<Component*>::const_iterator iter = _components.begin(); iter != _components.end(); iter++)
			{
				Component &c = (**iter);
				c.initialize();
			}

			for (std::list<Component*>::const_iterator iter = _components.begin(); iter != _components.end(); iter++)
			{
				Component &c = (**iter);
				c.startup();
			}

			_suspended = false;
		}

		void StandByManager::suspend()
		{
			ibrcommon::MutexLock l(_lock);
			if (_suspended || (!_enabled)) return;
			IBRCOMMON_LOGGER(info) << "StandByManager: suspend components" << IBRCOMMON_LOGGER_ENDL;

			for (std::list<Component*>::const_iterator iter = _components.begin(); iter != _components.end(); iter++)
			{
				Component &c = (**iter);
				c.terminate();
			}

			_suspended = true;
		}

		void StandByManager::componentUp() throw ()
		{
			bindEvent(dtn::core::GlobalEvent::className);
			_enabled = true;
		}

		void StandByManager::componentDown() throw ()
		{
			unbindEvent(dtn::core::GlobalEvent::className);
			_enabled = false;
		}

		const std::string StandByManager::getName() const
		{
			return "StandByManager";
		}
	} /* namespace daemon */
} /* namespace dtn */
