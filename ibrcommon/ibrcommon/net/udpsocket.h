/*
 * udpsocket.h
 *
 *  Created on: 02.03.2010
 *      Author: morgenro
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
