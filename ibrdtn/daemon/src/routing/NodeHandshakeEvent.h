/*
 * NodeHandshakeEvent.h
 *
 *  Created on: 08.12.2011
 *      Author: morgenro
 */

#ifndef NODEHANDSHAKEEVENT_H_
#define NODEHANDSHAKEEVENT_H_

#include "core/Event.h"
#include <ibrdtn/data/EID.h>

namespace dtn
{
	namespace routing
	{
		class NodeHandshakeEvent : public dtn::core::Event
		{
		public:
			enum HANDSHAKE_STATE
			{
				HANDSHAKE_REPLIED = 1,
				HANDSHAKE_COMPLETED = 2,
				HANDSHAKE_UPDATED = 3
			};

			virtual ~NodeHandshakeEvent();

			const std::string getName() const;

			std::string toString() const;

			static void raiseEvent(HANDSHAKE_STATE state, const dtn::data::EID &peer);

			HANDSHAKE_STATE state;
			dtn::data::EID peer;

			static const string className;

		private:
			NodeHandshakeEvent(HANDSHAKE_STATE state, const dtn::data::EID &peer);
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* NODEHANDSHAKEEVENT_H_ */
