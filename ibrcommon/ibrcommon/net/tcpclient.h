/*
 * tcpclient.h
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

#ifndef IBRCOMMON_TCPCLIENT_H_
#define IBRCOMMON_TCPCLIENT_H_

#include "ibrcommon/net/tcpstream.h"
#include "ibrcommon/Exceptions.h"
#include "ibrcommon/data/File.h"

namespace ibrcommon
{
	class tcpclient : public tcpstream
	{
	public:
		class SocketException : public Exception
		{
		public:
			SocketException(string error) : Exception(error)
			{};
		};

		/**
		 * create a not connected tcpclient
		 * @return
		 */
		tcpclient();

		/**
		 * Connect the tcpclient to a named socket
		 * @param socket to use
		 * @return
		 */
		tcpclient(const ibrcommon::File &socket);
		void open(const ibrcommon::File &socket);

		/**
		 * Connect the tcpclient to a address and port (IP)
		 * @param address
		 * @param port
		 * @return
		 */
		tcpclient(const std::string &address, const int port, size_t timeout = 0);
		void open(const std::string &address, const int port, size_t timeout = 0);

		/**
		 * destructor of the tcpclient
		 * @return
		 */
		virtual ~tcpclient();
	};
}

#endif /* TCPCLIENT_H_ */
