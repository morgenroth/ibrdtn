/*
 * EventReceiver.h
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

#ifndef EVENTRECEIVER_H_
#define EVENTRECEIVER_H_

#include <string>

namespace dtn
{
	namespace core
	{
		class Event;

		class EventReceiver
		{
		public:
			virtual ~EventReceiver() = 0;
			virtual void raiseEvent(const Event *evt) throw () = 0;

		protected:
			void bindEvent(std::string eventName);
			void unbindEvent(std::string eventName);
		};
	}
}

#endif /* EVENTRECEIVER_H_ */
