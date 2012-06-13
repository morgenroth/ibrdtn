/*
 * UnicastSocket.cpp
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
