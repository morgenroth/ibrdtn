/*
 * NodeEvent.h
 *
 *  Created on: 05.03.2009
 *      Author: morgenro
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
			NODE_INFO_UPDATED = 2
		};

		class NodeEvent : public Event
		{
		public:
			virtual ~NodeEvent();

			EventNodeAction getAction() const;
			const Node& getNode() const;
			const std::string getName() const;

			std::string toString() const;

			static void raise(const Node &n, const EventNodeAction action);

			static const std::string className;

		private:
			NodeEvent(const Node &n, const EventNodeAction action);

			const Node m_node;
			const EventNodeAction m_action;
		};
	}
}

#endif /* NODEEVENT_H_ */
