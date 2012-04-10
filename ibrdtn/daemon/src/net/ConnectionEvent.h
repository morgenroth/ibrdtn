/*
 * ConnectionEvent.h
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
 */

#ifndef CONNECTIONEVENT_H_
#define CONNECTIONEVENT_H_

#include "core/Event.h"
#include "core/Node.h"
#include <ibrdtn/data/EID.h>

namespace dtn
{
	namespace net
	{
		class ConnectionEvent : public dtn::core::Event
		{
		public:
			enum State
			{
				CONNECTION_SETUP = 0,
				CONNECTION_UP = 1,
				CONNECTION_DOWN = 2,
				CONNECTION_TIMEOUT = 3
			};

			virtual ~ConnectionEvent();

			const string getName() const;

			std::string toString() const;

			static const string className;

			static void raise(State, const dtn::core::Node&);

			const State state;
			const dtn::data::EID peer;
			const dtn::core::Node node;

		private:
			ConnectionEvent(State, const dtn::core::Node&);
		};
	}
}


#endif /* CONNECTIONEVENT_H_ */
