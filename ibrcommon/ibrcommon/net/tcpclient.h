/*
 * tcpclient.h
 *
 *  Created on: 29.07.2009
 *      Author: morgenro
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
