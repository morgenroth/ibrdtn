/*
 * ClientHandler.h
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

#ifndef CLIENTHANDLER_H_
#define CLIENTHANDLER_H_

#include "api/Registration.h"
#include "core/EventReceiver.h"
#include "core/Node.h"
#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/net/tcpstream.h>
#include <string>

namespace dtn
{
	namespace api
	{
		class ApiServerInterface;
		class ClientHandler;

		class ProtocolHandler
		{
		public:
			virtual ~ProtocolHandler() = 0;

			virtual void run() = 0;
			virtual void finally() = 0;
			virtual void setup() {};
			virtual void __cancellation() = 0;

		protected:
			ProtocolHandler(ClientHandler &client, ibrcommon::tcpstream &stream);
			ClientHandler &_client;
			ibrcommon::tcpstream &_stream;
		};

		class ClientHandler : public ibrcommon::DetachedThread
		{
		public:
			enum STATUS_CODES
			{
				API_STATUS_CONTINUE = 100,
				API_STATUS_OK = 200,
				API_STATUS_CREATED = 201,
				API_STATUS_ACCEPTED = 202,
				API_STATUS_FOUND = 302,
				API_STATUS_BAD_REQUEST = 400,
				API_STATUS_UNAUTHORIZED = 401,
				API_STATUS_FORBIDDEN = 403,
				API_STATUS_NOT_FOUND = 404,
				API_STATUS_NOT_ALLOWED = 405,
				API_STATUS_NOT_ACCEPTABLE = 406,
				API_STATUS_CONFLICT = 409,
				API_STATUS_INTERNAL_ERROR = 500,
				API_STATUS_NOT_IMPLEMENTED = 501,
				API_STATUS_SERVICE_UNAVAILABLE = 503,
				API_STATUS_VERSION_NOT_SUPPORTED = 505
			};

			ClientHandler(ApiServerInterface &srv, Registration &registration, ibrcommon::tcpstream *conn);
			virtual ~ClientHandler();

			Registration& getRegistration();
			ApiServerInterface& getAPIServer();

			/**
			 * swaps the active registration of this client with the given one
			 * @param reg the new registration
			 */
			void switchRegistration(Registration &reg);

			void eventNodeAvailable(const dtn::core::Node &node);
			void eventNodeUnavailable(const dtn::core::Node &node);

		protected:
			void run();
			void finally();
			void setup();
			void __cancellation();

		private:
			void error(STATUS_CODES code, const std::string &msg);
			void processCommand(const std::vector<std::string> &cmd);

			ApiServerInterface &_srv;
			Registration *_registration;
			ibrcommon::Mutex _write_lock;
			ibrcommon::tcpstream *_stream;
			dtn::data::EID _endpoint;

			ProtocolHandler *_handler;
		};

		class ApiServerInterface
		{
		public:
			virtual ~ApiServerInterface() {};
			virtual void processIncomingBundle(const dtn::data::EID &source, dtn::data::Bundle &bundle) = 0;
			virtual void connectionUp(ClientHandler *conn) = 0;
			virtual void connectionDown(ClientHandler *conn) = 0;
			virtual void freeRegistration(Registration &reg) = 0;
			virtual Registration& getRegistration(const std::string &handle) = 0;
		};
	}
}
#endif /* CLIENTHANDLER_H_ */
