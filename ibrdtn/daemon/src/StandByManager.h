/*
 * StandByManager.h
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
