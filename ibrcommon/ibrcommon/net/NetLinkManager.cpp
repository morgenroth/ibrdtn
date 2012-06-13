/*
 * NetLinkManager.cpp
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

#include "ibrcommon/net/NetLinkManager.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/Logger.h"

#include <netlink/cache.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>
#include <netlink/route/rtnl.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>

namespace ibrcommon
{
	NetLinkManager::NetLinkManager()
	 : _initialized(false), _sock(NULL), _refresh_cache(false)
	{
		ibrcommon::MutexLock l(_call_mutex);

		_handle = nl_handle_alloc();
		nl_connect(_handle, NETLINK_ROUTE);

		_link_cache = rtnl_link_alloc_cache(_handle);

		if (_link_cache == NULL)
		{
			nl_close(_handle);
			nl_handle_destroy(_handle);
			throw ibrcommon::vsocket_exception("netlink cache allocation failed");
		}

		_addr_cache = rtnl_addr_alloc_cache(_handle);

		if (_addr_cache == NULL)
		{
			nl_close(_handle);
			nl_handle_destroy(_handle);
			nl_cache_free(_link_cache);
			_link_cache = NULL;
			throw ibrcommon::vsocket_exception("netlink cache allocation failed");
		}

		_initialized = true;

		// create a new socket for the netlink interface
		_sock = new ibrcommon::vsocket();
	}

	NetLinkManager::~NetLinkManager()
	{
		stop();
		join();

		// destroy the socket for the netlink interface
		delete _sock;

		nl_close(_handle);
		nl_cache_free(_addr_cache);
		nl_cache_free(_link_cache);
		nl_handle_destroy(_handle);
	}

	void add_addr_to_list(struct nl_object *obj, void *data)
	{
		char buf[INET6_ADDRSTRLEN+5];
		std::list<vaddress> *list = static_cast<std::list<vaddress>*>(data);

		struct nl_addr *naddr = rtnl_addr_get_local((struct rtnl_addr *) obj);
		int ifindex = 0;
		int scope = rtnl_addr_get_scope((struct rtnl_addr *) obj);

		if (scope == rtnl_str2scope("link"))
			ifindex = rtnl_addr_get_ifindex((struct rtnl_addr *) obj);

		if (naddr)
		{
			int family = nl_addr_get_family(naddr);
			nl_addr2str( naddr, buf, sizeof( buf ) );
			vaddress vaddr(vaddress::Family(family), vaddress::strip_netmask(std::string(buf)), ifindex, false);
			list->push_back( vaddr );
		}

		struct nl_addr *baddr = rtnl_addr_get_broadcast((struct rtnl_addr *) obj);

		if (baddr)
		{
			int family = nl_addr_get_family(baddr);
			nl_addr2str( baddr, buf, sizeof( buf ) );
			vaddress vaddr(vaddress::Family(family), vaddress::strip_netmask(std::string(buf)), ifindex, true);
			list->push_back( vaddr );
		}
	}

	void NetLinkManager::__cancellation()
	{
		ibrcommon::MutexLock l(_call_mutex);
		_initialized = false;
		_sock->close();
	}

	void NetLinkManager::run()
	{
		struct nl_handle *handle = nl_handle_alloc();
		nl_connect(handle, NETLINK_ROUTE);

		// init route messages
		nl_socket_add_membership(handle, RTNLGRP_IPV4_IFADDR);

//		IPv6 requires further support in the parsing procedures!
//		nl_socket_add_membership(handle, RTNLGRP_IPV6_IFADDR);
		nl_socket_add_membership(handle, RTNLGRP_LINK);

		// add netlink fd to vsocket
		_sock->add(nl_socket_get_fd(handle));

		try {
			while (_initialized)
			{
				std::list<int> fds;
				_sock->select(fds, NULL);
				int fd = fds.front();

				// create a new event object
				NetLinkManagerEvent lme(fd);

				// ignore if the event is unknown
				if (lme.getType() == LinkManagerEvent::EVENT_UNKOWN) continue;

				// ignore if this is an wireless extension event
				if (lme.isWirelessExtension()) continue;

				// need to refresh the cache
				{
					ibrcommon::MutexLock l(_call_mutex);
					_refresh_cache = true;
				}

				// print out some debugging
				IBRCOMMON_LOGGER_DEBUG(10) << lme.toString() << IBRCOMMON_LOGGER_ENDL;

				// notify all subscribers about this event
				raiseEvent(lme);
			}
		} catch (const vsocket_exception&) {
			// stopped / interrupted
			IBRCOMMON_LOGGER(error) << "NetLink connection stopped" << IBRCOMMON_LOGGER_ENDL;
		}

		nl_close(handle);
		nl_handle_destroy(handle);
	}

	const std::list<vaddress> NetLinkManager::getAddressList(const vinterface &interface, const vaddress::Family f)
	{
		ibrcommon::MutexLock l(_call_mutex);

		if (_refresh_cache)
		{
			nl_cache_free(_addr_cache);
			nl_cache_free(_link_cache);

			_link_cache = rtnl_link_alloc_cache(_handle);

			if (_link_cache == NULL)
			{
				throw ibrcommon::vsocket_exception("netlink cache allocation failed");
			}

			_addr_cache = rtnl_addr_alloc_cache(_handle);

			if (_addr_cache == NULL)
			{
				nl_cache_free(_link_cache);
				throw ibrcommon::vsocket_exception("netlink cache allocation failed");
			}

			// mark the cache as refreshed
			_refresh_cache = false;
		}

		std::list<vaddress> addresses;

		struct rtnl_addr *filter = rtnl_addr_alloc();
		const std::string i = interface.toString();
		rtnl_addr_set_ifindex(filter, rtnl_link_name2i(_link_cache, i.c_str()));

		if (f == vaddress::VADDRESS_UNSPEC)
		{
			rtnl_addr_set_family(filter, AF_INET6);
			nl_cache_foreach_filter(_addr_cache, (struct nl_object *) filter,
									add_addr_to_list, &addresses);

			rtnl_addr_set_family(filter, AF_INET);
			nl_cache_foreach_filter(_addr_cache, (struct nl_object *) filter,
									add_addr_to_list, &addresses);
		}
		else
		{
			rtnl_addr_set_family(filter, f);
			nl_cache_foreach_filter(_addr_cache, (struct nl_object *) filter,
									add_addr_to_list, &addresses);
		}

		rtnl_addr_put(filter);

		return addresses;
	}

	const std::string NetLinkManager::getInterface(const int ifindex) const
	{
		char buf[256];
		rtnl_link_i2name(_link_cache, ifindex, (char*)&buf, sizeof buf);
		return std::string((char*)&buf);
	}

	/** read a netlink message from the socket and create a new netlink event object **/
	NetLinkManagerEvent::NetLinkManagerEvent(int fd)
	 : _type(EVENT_UNKOWN), _state(0), _wireless(false)
	{
		char buf[4096];
		int len = 0;
		struct nlmsghdr *nlh;

		// cast netlink message
		nlh = (struct nlmsghdr *) buf;

		// get local reference to the attributes list
		std::map<int, std::string> &attrlist = _attributes;

		while ((len = recv(fd, nlh, 4096, 0)) > 0)
		{
			while ((NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE))
			{
				switch (nlh->nlmsg_type)
				{
					case RTM_BASE:
					{
						int attrlen, nlmsg_len, rta_len;
						struct rtattr * attr;

						struct ifinfomsg *ifi = (struct ifinfomsg *) NLMSG_DATA(nlh);

						nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));
						attrlen = nlh->nlmsg_len - nlmsg_len;

						if (attrlen < 0) break;

						attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);

						rta_len = RTA_ALIGN(sizeof(struct rtattr));
						while (RTA_OK(attr, attrlen))
						{
							size_t rta_length = RTA_PAYLOAD(attr);

							switch (attr->rta_type)
							{
							case IFLA_IFNAME:
								_interface = ibrcommon::vinterface( std::string((char*)RTA_DATA(attr), rta_length) );
								_type = EVENT_LINK_STATE;
								break;

							case IFLA_OPERSTATE:
							{
								char s;
								::memcpy(&s, (char*)RTA_DATA(attr), 1);
								_state = s;
								break;
							}

							case IFLA_WIRELESS:
								_wireless = true;
								break;

							default:
								attrlist[attr->rta_type] = std::string((char*)RTA_DATA(attr), rta_length);
								break;
							}

							attr = RTA_NEXT(attr, attrlen);
						}
						break;
					}

					case RTM_NEWADDR:
					{
						struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
						struct rtattr *rth = IFA_RTA(ifa);
						int rtl = IFA_PAYLOAD(nlh);

						// parse all attributes
						while (rtl && RTA_OK(rth, rtl))
						{
							switch (rth->rta_type)
							{
								// local address
								case IFA_LOCAL:
								{
									char address[256];

									uint32_t ipaddr = htonl(*((uint32_t *)RTA_DATA(rth)));
									sprintf(address, "%d.%d.%d.%d", (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);

									_address = ibrcommon::vaddress(ibrcommon::vaddress::VADDRESS_INET, std::string(address));
									_type = EVENT_ADDRESS_ADDED;
									break;
								}

								// interface name
								case IFA_LABEL:
								{
									//char name[IFNAMSIZ];
									char *name = (char *)RTA_DATA(rth);
									_interface = ibrcommon::vinterface(name);
									break;
								}
							}
							rth = RTA_NEXT(rth, rtl);
						}
						break;
					}

					case RTM_DELADDR:
					{
						struct ifaddrmsg *ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
						struct rtattr *rth = IFA_RTA(ifa);
						int rtl = IFA_PAYLOAD(nlh);

						// parse all attributes
						while (rtl && RTA_OK(rth, rtl))
						{
							switch (rth->rta_type)
							{
								// local address
								case IFA_LOCAL:
								{
									char address[256];

									uint32_t ipaddr = htonl(*((uint32_t *)RTA_DATA(rth)));
									sprintf(address, "%d.%d.%d.%d", (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);

									_address = ibrcommon::vaddress(ibrcommon::vaddress::VADDRESS_INET, std::string(address));
									_type = EVENT_ADDRESS_REMOVED;
									break;
								}

								// interface name
								case IFA_LABEL:
								{
									//char name[IFNAMSIZ];
									char *name = (char *)RTA_DATA(rth);
									_interface = ibrcommon::vinterface(name);
									break;
								}
							}
							rth = RTA_NEXT(rth, rtl);
						}
						break;
					}

					default:
						IBRCOMMON_LOGGER_DEBUG(10) << "unknown netlink type received: " << nlh->nlmsg_type << IBRCOMMON_LOGGER_ENDL;
						break;
				}

				nlh = NLMSG_NEXT(nlh, len);
			}

			return;
		}
	}

	NetLinkManagerEvent::~NetLinkManagerEvent()
	{
	}

	const ibrcommon::vinterface& NetLinkManagerEvent::getInterface() const
	{
		return _interface;
	}

	const ibrcommon::vaddress& NetLinkManagerEvent::getAddress() const
	{
		return _address;
	}

	unsigned int NetLinkManagerEvent::getState() const
	{
		return _state;
	}

	bool NetLinkManagerEvent::isWirelessExtension() const
	{
		return _wireless;
	}

	LinkManagerEvent::EventType NetLinkManagerEvent::getType() const
	{
		return _type;
	}

	const std::string NetLinkManagerEvent::toString() const
	{
		std::stringstream ss;
		ss << "NetLinkManagerEvent on " << getInterface().toString() << "; Type: " << getType();

		switch (getType())
		{
		case EVENT_LINK_STATE:
			ss << "; State: " << getState();
			break;

		case EVENT_ADDRESS_ADDED:
			ss << "; Address added: " << getAddress().toString();
			break;

		case EVENT_ADDRESS_REMOVED:
			ss << "; Address removed: " << getAddress().toString();
			break;

		default:
			break;
		};

		return ss.str();
	}

	void NetLinkManagerEvent::debug() const
	{
//		for (std::map<int, std::string>::const_iterator iter = attr.begin(); iter != attr.end(); iter++)
//		{
//			std::stringstream ss;
//			const std::string &value = (*iter).second;
//
//			for (std::string::const_iterator si = value.begin(); si != value.end(); si++)
//			{
//				const char &c = (*si);
//				ss << std::hex << "0x" << (int)c << " ";
//			}
//
//			IBRCOMMON_LOGGER_DEBUG(10) << (*iter).first << ": " << ss.str() << IBRCOMMON_LOGGER_ENDL;
//		}
	}
}
