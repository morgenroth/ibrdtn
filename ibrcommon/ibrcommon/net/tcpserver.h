/*
 * tcpserver.h
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

#ifndef IBRCOMMON_TCPSERVER_H_
#define IBRCOMMON_TCPSERVER_H_

#include "ibrcommon/net/tcpstream.h"
#include "ibrcommon/net/vinterface.h"
#include <netinet/in.h>
#include "ibrcommon/Exceptions.h"
#include "ibrcommon/data/File.h"
#include "ibrcommon/net/vsocket.h"

namespace ibrcommon
{
	class tcpserver
	{
	public:
		/**
		 * creates a tcpserver with no bound socket
		 * @return
		 */
		tcpserver();

		/**
		 * creates a tcpserver bound to a filesocket
		 * @param socket
		 * @return
		 */
		tcpserver(const ibrcommon::File &socket);

		/**
		 * @param address the address to listen to
		 * @param port the port to listen to
		 */
		void bind(const vinterface &net, int port, bool reuseaddr = true);

		/**
		 * @param port the port to listen to
		 */
		void bind(int port, bool reuseaddr = true);

		/**
		 * listen on all sockets bound to
		 * @param connections
		 */
		void listen(int connections);

		/**
		 * Destructor
		 */
		virtual ~tcpserver();

		/**
		 * Accept a new connection.
		 * @return A socket for the connection.
		 */
		tcpstream* accept();

		void close();
		void shutdown();

	private:
		vsocket _socket;
	};
}

#endif /* TCPSERVER_H_ */
