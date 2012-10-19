/*
 * DefaultLinkManager.cpp
 *
 *  Created on: 11.10.2012
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/link/PosixLinkManager.h"

#include <string>
#include <typeinfo>

#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

namespace ibrcommon
{
	PosixLinkManager::PosixLinkManager()
	{
	}

	PosixLinkManager::~PosixLinkManager()
	{
	}

	const vinterface PosixLinkManager::getInterface(int index) const
	{
		struct ifaddrs *ifap = NULL;
		int status = getifaddrs(&ifap);
		int i = 0;
		std::string ret;

		if ((status != 0) || (ifap == NULL))
		{
				// error, return with default address
				throw ibrcommon::Exception("can not iterate through interfaces");
		}

		for (struct ifaddrs *iter = ifap; iter != NULL; iter = iter->ifa_next, i++)
		{
			if (index == i)
			{
				ret = iter->ifa_name;
				break;
			}
		}

		freeifaddrs(ifap);

		return ret;
	}

	const std::list<vaddress> PosixLinkManager::getAddressList(const vinterface &iface)
	{
		std::list<vaddress> ret;

		struct ifaddrs *ifap = NULL;
		int status = getifaddrs(&ifap);
		int i = 0;

		if ((status != 0) || (ifap == NULL))
		{
			// error, return with default address
			throw ibrcommon::Exception("can not iterate through interfaces");
		}

		for (struct ifaddrs *iter = ifap; iter != NULL; iter = iter->ifa_next, i++)
		{
			if (iter->ifa_addr == NULL) continue;
			if ((iter->ifa_flags & IFF_UP) == 0) continue;

			// get the interface name
			vinterface interface(iter->ifa_name);

			// skip all non-matching interfaces
			if (interface != iface) continue;

			// cast to a sockaddr
			sockaddr *ifaceaddr = iter->ifa_addr;

			char address[256];
			if (::getnameinfo((struct sockaddr *) &ifaceaddr, sizeof ifaceaddr, address, sizeof address, 0, 0, NI_NUMERICHOST) == 0) {
				ret.push_back( vaddress(std::string(address)) );
			}
		}
		freeifaddrs(ifap);

		return ret;
	}

} /* namespace ibrcommon */
