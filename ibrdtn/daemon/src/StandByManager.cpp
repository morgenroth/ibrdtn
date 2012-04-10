/*
 * StandByManager.cpp
 *
 *  Created on: 29.03.2012
 *      Author: jm
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

		void StandByManager::raiseEvent(const dtn::core::Event *evt)
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

		void StandByManager::componentUp()
		{
			bindEvent(dtn::core::GlobalEvent::className);
			_enabled = true;
		}

		void StandByManager::componentDown()
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
