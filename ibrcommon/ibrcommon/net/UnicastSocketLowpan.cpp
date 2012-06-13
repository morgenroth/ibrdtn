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

#include "ibrcommon/net/UnicastSocketLowpan.h"

namespace ibrcommon
{
	UnicastSocketLowpan::UnicastSocketLowpan()
	 : lowpansocket(IPPROTO_UDP) //FIXME
	{
	}

	UnicastSocketLowpan::~UnicastSocketLowpan()
	{
	}

	void UnicastSocketLowpan::bind(int panid, const vinterface &iface)
	{

		_sockaddr.addr.addr_type = IEEE802154_ADDR_SHORT;
		_sockaddr.addr.pan_id = panid;

		// Get address via netlink
		getAddress(&_sockaddr.addr, iface);

		if ( ::bind(_vsocket.fd(), (struct sockaddr *) &_sockaddr, sizeof(_sockaddr)) < 0 )
		{
			throw SocketException("UnicastSocketLowpan: cannot bind socket");
		}
	}
}
