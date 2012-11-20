/*
 * EventConnection.h
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

#ifndef EVENTCONNECTION_H_
#define EVENTCONNECTION_H_

#include "api/ClientHandler.h"
#include "core/EventReceiver.h"

namespace dtn
{
	namespace api
	{
		class EventConnection : public ProtocolHandler, public dtn::core::EventReceiver
		{
		public:
			EventConnection(ClientHandler &client, ibrcommon::socketstream &stream);
			virtual ~EventConnection();

			void run();
			void finally();
			void setup();
			void __cancellation();

			void raiseEvent(const dtn::core::Event *evt) throw ();

		private:
			ibrcommon::Mutex _mutex;
			bool _running;
		};
	} /* namespace api */
} /* namespace dtn */
#endif /* EVENTCONNECTION_H_ */
