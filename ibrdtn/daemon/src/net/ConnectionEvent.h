/*
 * ConnectionEvent.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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
