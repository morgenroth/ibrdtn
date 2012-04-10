/*
 * UnicastSocket.cpp
 *
 *  Created on: 28.06.2010
 *      Author: morgenro
 */

#include "ibrcommon/net/UnicastSocket.h"
#include "ibrcommon/net/vsocket.h"
#include <netdb.h>

namespace ibrcommon
{
	UnicastSocket::UnicastSocket()
	{
	}

	UnicastSocket::~UnicastSocket()
	{
	}

	void UnicastSocket::bind(int port, const vinterface &iface)
	{
		std::list<vaddress> list = iface.getAddresses();
		for (std::list<vaddress>::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			if (!iter->isBroadcast())
			{
				bind(port, *iter);
			}
		}
	}

	void UnicastSocket::bind(int port, const vaddress &address)
	{
		_socket.bind(address, port, SOCK_DGRAM);
	}

	void UnicastSocket::bind()
	{
		_socket.bind(0, SOCK_DGRAM);
	}
}
