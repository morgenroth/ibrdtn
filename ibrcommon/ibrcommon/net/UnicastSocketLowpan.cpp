/*
 * UnicastSocket.cpp
 *
 *  Created on: 28.06.2010
 *      Author: morgenro
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
