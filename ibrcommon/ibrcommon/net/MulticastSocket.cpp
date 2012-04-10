/*
 * MulticastSocket.cpp
 *
 *  Created on: 28.06.2010
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/net/MulticastSocket.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/Logger.h"
#include <netdb.h>
#include <string.h>

#ifndef HAVE_BZERO
#define bzero(s,n) (memset((s), '\0', (n)), (void) 0)
#endif

namespace ibrcommon
{
	MulticastSocket::MulticastSocket(const int fd)
	 : _fd(fd)
	{
	}

	MulticastSocket::~MulticastSocket()
	{
	}

	void MulticastSocket::setInterface(const vinterface &iface)
	{
		struct ip_mreq imr;
		::memset(&imr, 0, sizeof(ip_mreq));

		std::list<vaddress> addrs = iface.getAddresses(vaddress::VADDRESS_INET);
		if (addrs.empty()) return;

		const vaddress &vaddr = addrs.front();

		// set interface address
		struct addrinfo hints, *addr;
		memset(&hints, 0, sizeof hints);

		hints.ai_family = vaddr.getFamily();
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(vaddr.get().c_str(), NULL, &hints, &addr);

		struct sockaddr_in *saddr = (struct sockaddr_in *) addr->ai_addr;

		if ( ::setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&saddr->sin_addr, sizeof(saddr->sin_addr)) < 0 )
		{
			throw vsocket_exception("MulticastSocket: cannot set default interface");
		}

		freeaddrinfo(addr);
	}

	void MulticastSocket::joinGroup(const vaddress &group)
	{
		struct ip_mreq imr;
		::memset(&imr, 0, sizeof(ip_mreq));
		imr.imr_interface.s_addr = INADDR_ANY;

		if (inet_pton(group.getFamily(), group.get().c_str(), &imr.imr_multiaddr) <= 0)
		{
			throw vsocket_exception("MulticastSocket: can not parse address " + group.get());
		}

		if ( ::setsockopt(_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&imr, sizeof(struct ip_mreq)) < 0)
		{
			throw vsocket_exception("MulticastSocket: setsockopt(IP_ADD_MEMBERSHIP)");
		}

		IBRCOMMON_LOGGER_DEBUG(5) << "multicast join group successful to "<< group.toString() << IBRCOMMON_LOGGER_ENDL;
	}

	void MulticastSocket::joinGroup(const vaddress &group, const vinterface &iface)
	{
		std::list<vaddress> addrlist = iface.getAddresses(vaddress::VADDRESS_INET);

		for (std::list<vaddress>::const_iterator iter = addrlist.begin(); iter != addrlist.end(); iter++)
		{
			const vaddress &vaddr = *iter;

			// currently we only support IPv4 addresses
			if (vaddr.getFamily() != vaddress::VADDRESS_INET) continue;

			try {
				struct ip_mreq imr;
				::memset(&imr, 0, sizeof(ip_mreq));

				if (inet_pton(group.getFamily(), group.get().c_str(), &imr.imr_multiaddr) <= 0)
				{
					throw vsocket_exception("MulticastSocket: can not parse address " + group.get());
				}

				// set join address
				struct addrinfo hints, *addr;
				memset(&hints, 0, sizeof hints);

				hints.ai_family = group.getFamily();
				hints.ai_socktype = SOCK_DGRAM;
				hints.ai_flags = AI_PASSIVE;

				getaddrinfo(vaddr.get().c_str(), NULL, &hints, &addr);

				struct sockaddr_in *saddr = (struct sockaddr_in *) addr->ai_addr;
				::memcpy(&imr.imr_interface, &saddr->sin_addr, sizeof(saddr->sin_addr));

				if ( ::setsockopt(_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&imr, sizeof(struct ip_mreq)) < 0)
				{
					freeaddrinfo(addr);
					throw vsocket_exception("MulticastSocket: setsockopt(IP_ADD_MEMBERSHIP)");
				}

				freeaddrinfo(addr);

				// successful!
				IBRCOMMON_LOGGER_DEBUG(5) << "multicast join group successful to " << group.toString() << " on " << vaddr.toString() << IBRCOMMON_LOGGER_ENDL;
				return;
			} catch (const vsocket_exception&) {
				// try the next address
			}
		}
	}

	void MulticastSocket::leaveGroup(const vaddress &group)
	{
		struct ip_mreq imr;
		::memset(&imr, 0, sizeof(ip_mreq));

		if (inet_pton(group.getFamily(), group.get().c_str(), &imr.imr_multiaddr) <= 0)
		{
			throw vsocket_exception("MulticastSocket: can not parse address " + group.get());
		}

		if ( ::setsockopt(_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *)&imr, sizeof(struct ip_mreq)) < 0)
		{
			throw vsocket_exception("MulticastSocket: setsockopt(IP_DROP_MEMBERSHIP)");
		}
	}

	void MulticastSocket::leaveGroup(const vaddress &group, const vinterface &iface)
	{
		struct ip_mreq imr;
		::memset(&imr, 0, sizeof(ip_mreq));

		if (inet_pton(group.getFamily(), group.get().c_str(), &imr.imr_multiaddr) <= 0)
		{
			throw vsocket_exception("MulticastSocket: can not parse address " + group.get());
		}

		// set leave address
		struct addrinfo hints, *addr;
		memset(&hints, 0, sizeof hints);

		hints.ai_family = group.getFamily();
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(group.get().c_str(), NULL, &hints, &addr);

		struct sockaddr_in *saddr = (struct sockaddr_in *) addr->ai_addr;
		::memcpy(&imr.imr_interface, &saddr->sin_addr, sizeof(saddr->sin_addr));

		if ( ::setsockopt(_fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *)&imr, sizeof(struct ip_mreq)) < 0)
		{
			throw vsocket_exception("MulticastSocket: setsockopt(IP_DROP_MEMBERSHIP)");
		}

		freeaddrinfo(addr);
	}
}
