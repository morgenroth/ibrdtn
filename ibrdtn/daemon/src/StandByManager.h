/*
 * StandByManager.h
 *
 *  Created on: 29.03.2012
 *      Author: jm
 */

#ifndef STANDBYMANAGER_H_
#define STANDBYMANAGER_H_

#include "Component.h"
#include "core/EventReceiver.h"
#include <ibrcommon/thread/Mutex.h>
#include <list>

namespace dtn {
	namespace daemon {
		class StandByManager : public dtn::core::EventReceiver, public IntegratedComponent {
		public:
			StandByManager();
			virtual ~StandByManager();

			void adopt(Component *c);

			virtual void raiseEvent(const dtn::core::Event *evt);

			virtual void componentUp();
			virtual void componentDown();

			virtual const std::string getName() const;

		private:
			void wakeup();
			void suspend();

			std::list<Component*> _components;
			ibrcommon::Mutex _lock;
			bool _suspended;
			bool _enabled;
		};
	} /* namespace daemon */
} /* namespace dtn */
#endif /* STANDBYMANAGER_H_ */
