/*
 * NodeEvent.h
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

#ifndef NODEEVENT_H_
#define NODEEVENT_H_

#include "core/Node.h"
#include "core/Event.h"

using namespace dtn::core;

namespace dtn
{
	namespace core
	{
		enum EventNodeAction
		{
			NODE_UNAVAILABLE = 0,
			NODE_AVAILABLE = 1,
			NODE_DATA_ADDED = 2,
			NODE_DATA_REMOVED = 3
		};

		class NodeEvent : public Event
		{
		public:
			virtual ~NodeEvent();

			EventNodeAction getAction() const;
			const Node& getNode() const;
			const std::string getName() const;

			std::string getMessage() const;

			static void raise(const Node &n, const EventNodeAction action);

		private:
			NodeEvent(const Node &n, const EventNodeAction action);

			const Node m_node;
			const EventNodeAction m_action;
		};
	}
}

#endif /* NODEEVENT_H_ */
