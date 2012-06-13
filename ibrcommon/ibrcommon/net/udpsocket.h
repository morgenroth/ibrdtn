/*
 * udpsocket.h
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

#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/Exceptions.h"
#include <list>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace ibrcommon
{
	class udpsocket
	{
	public:
		class SocketException : public ibrcommon::Exception
		{
		public:
			SocketException(string error) : ibrcommon::Exception(error)
			{};
		};

		virtual ~udpsocket();

		virtual void shutdown();

		int receive(char* data, size_t maxbuffer);
		int receive(char* data, size_t maxbuffer, std::string &address, unsigned int &port);

		int send(const ibrcommon::vaddress &address, const unsigned int port, const char *data, const size_t length);

	protected:
		udpsocket() throw (SocketException);
		vsocket _socket;
		struct sockaddr_in _sockaddr;
	};
}

#endif /* UDPSOCKET_H_ */
