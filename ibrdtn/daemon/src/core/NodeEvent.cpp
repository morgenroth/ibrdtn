/*
 * NodeEvent.cpp
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

#include "core/NodeEvent.h"
#include "core/EventDispatcher.h"

using namespace dtn::core;
using namespace std;

namespace dtn
{
	namespace core
	{
		NodeEvent::NodeEvent(const Node &n, const EventNodeAction action)
		: m_node(n), m_action(action)
		{}

		void NodeEvent::raise(const Node &n, const EventNodeAction action)
		{
			dtn::core::EventDispatcher<NodeEvent>::raise( new NodeEvent(n, action) );
		}

		NodeEvent::~NodeEvent()
		{}

		const Node& NodeEvent::getNode() const
		{
			return m_node;
		}

		EventNodeAction NodeEvent::getAction() const
		{
			return m_action;
		}

		const string NodeEvent::getName() const
		{
			return NodeEvent::className;
		}

		string NodeEvent::toString() const
		{
			return className;
		}

		const string NodeEvent::className = "NodeEvent";
	}
}
