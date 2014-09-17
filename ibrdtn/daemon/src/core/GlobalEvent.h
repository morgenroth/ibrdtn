/*
 * GlobalEvent.h
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

#ifndef GLOBALEVENT_H_
#define GLOBALEVENT_H_

#include "core/Event.h"

namespace dtn
{
	namespace core
	{
		class GlobalEvent : public Event
		{
		public:
			enum Action
			{
				GLOBAL_SHUTDOWN = 0,
				GLOBAL_RELOAD = 1,
				GLOBAL_IDLE = 2,
				GLOBAL_BUSY = 3,
				GLOBAL_LOW_ENERGY = 4,
				GLOBAL_NORMAL = 5,
				GLOBAL_START_DISCOVERY = 6,
				GLOBAL_STOP_DISCOVERY = 7,
				GLOBAL_INTERNET_AVAILABLE = 8,
				GLOBAL_INTERNET_UNAVAILABLE = 9
			};

			virtual ~GlobalEvent();

			const std::string getName() const;
			std::string getMessage() const;

			Action getAction() const;

			static void raise(const Action a);

		private:
			GlobalEvent(const Action a);
			Action _action;
		};
	}
}

#endif /* GLOBALEVENT_H_ */
