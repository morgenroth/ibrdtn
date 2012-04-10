/*
 * Notifier.cpp
 *
 *  Created on: 11.12.2009
 *      Author: morgenro
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

		void Notifier::componentUp()
		{
			bindEvent(NodeEvent::className);
		}

		void Notifier::componentDown()
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
