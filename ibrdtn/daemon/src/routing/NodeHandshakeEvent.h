/*
 * NodeHandshakeEvent.h
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
				HANDSHAKE_COMPLETED = 2
			};

			virtual ~NodeHandshakeEvent();

			const std::string getName() const;

			std::string getMessage() const;

			static void raiseEvent(HANDSHAKE_STATE state, const dtn::data::EID &peer);

			HANDSHAKE_STATE state;
			dtn::data::EID peer;

		private:
			NodeHandshakeEvent(HANDSHAKE_STATE state, const dtn::data::EID &peer);
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* NODEHANDSHAKEEVENT_H_ */
