/*
 * Notifier.cpp
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

#include "Notifier.h"
#include "core/NodeEvent.h"
#include <stdlib.h>

#include <sstream>

namespace dtn
{
	namespace daemon
	{
		using namespace dtn::core;

		Notifier::Notifier(std::string cmd) : _cmd(cmd)
		{
		}

		Notifier::~Notifier()
		{
		}

		void Notifier::componentUp() throw ()
		{
			bindEvent(NodeEvent::className);
		}

		void Notifier::componentDown() throw ()
		{
			unbindEvent(NodeEvent::className);
		}

		void Notifier::raiseEvent(const dtn::core::Event *evt)
		{
			const NodeEvent *node = dynamic_cast<const NodeEvent*>(evt);

			if (node != NULL)
			{
				std::stringstream msg;

				switch (node->getAction())
				{
				case NODE_AVAILABLE:
					msg << "Node is available: " << node->getNode().toString();
					notify("IBR-DTN", msg.str());
					break;

				case NODE_UNAVAILABLE:
					msg << "Node is unavailable: " << node->getNode().toString();
					notify("IBR-DTN", msg.str());
					break;
				default:
					break;
				}
			}
		}

		void Notifier::notify(std::string title, std::string msg)
		{
			std::stringstream notifycmd;
			notifycmd << _cmd;
			notifycmd << " \"" << title << "\" \"" << msg << "\"";

			system(notifycmd.str().c_str());
		}

		const std::string Notifier::getName() const
		{
			return "Notifier";
		}
	}
}
