/*
 * EventConnection.h
 *
 *  Created on: 13.10.2011
 *      Author: morgenro
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
			EventConnection(ClientHandler &client, ibrcommon::tcpstream &stream);
			virtual ~EventConnection();

			void run();
			void finally();
			void setup();
			void __cancellation();

			void raiseEvent(const dtn::core::Event *evt);

		private:
			ibrcommon::Mutex _mutex;
			bool _running;
		};
	} /* namespace api */
} /* namespace dtn */
#endif /* EVENTCONNECTION_H_ */
