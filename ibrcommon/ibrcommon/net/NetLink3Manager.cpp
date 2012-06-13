/*
 * NetLink3Manager.cpp
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

#include "ibrcommon/net/NetLink3Manager.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/Logger.h"

#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/route/rtnl.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
//#include <net/if.h>
#include <string.h>

namespace ibrcommon
{
	static int nl3_callback(struct nl_msg *msg, void *arg)
	{
		NetLink3Manager *m = static_cast<NetLink3Manager *>(arg);
		NetLink3ManagerEvent evt(msg);
		m->callback(evt);
		return 0;
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

	NetLink3Manager::NetLink3Manager()
	 : _refresh_cache(false), _running(true), _sock(NULL)
	{
		// initialize the sockets
		_nl_notify_sock = nl_socket_alloc();
		_nl_query_sock = nl_socket_alloc();

		// disable seq check for notify socket
		nl_socket_disable_seq_check(_nl_notify_sock);

		// define callback method
		nl_socket_modify_cb(_nl_notify_sock, NL_CB_VALID, NL_CB_CUSTOM, nl3_callback, this);

		// connect to routing netlink protocol
		nl_connect(_nl_notify_sock, NETLINK_ROUTE);
		nl_connect(_nl_query_sock, NETLINK_ROUTE);

		// init route messages
		nl_socket_add_memberships(_nl_notify_sock, RTNLGRP_IPV4_IFADDR);

//		IPv6 requires further support in the parsing procedures!
//		nl_socket_add_membership(_nl_notify_sock, RTNLGRP_IPV6_IFADDR);
		nl_socket_add_memberships(_nl_notify_sock, RTNLGRP_LINK);

		// create a cache for the links
		if (rtnl_link_alloc_cache(_nl_query_sock, AF_UNSPEC, &_link_cache) < 0)
		{
			nl_socket_free(_nl_notify_sock);
			nl_socket_free(_nl_query_sock);
			// error
			throw ibrcommon::vsocket_exception("netlink cache allocation failed");
		}

		// create a cache for addresses
		if (rtnl_addr_alloc_cache(_nl_query_sock, &_addr_cache) < 0)
		{
			nl_socket_free(_nl_notify_sock);
			nl_socket_free(_nl_query_sock);
			// error
			nl_cache_free(_link_cache);
			_link_cache = NULL;
			throw ibrcommon::vsocket_exception("netlink cache allocation failed");
		}

		// create a new socket for the netlink interface
		_sock = new ibrcommon::vsocket();
	}

	NetLink3Manager::~NetLink3Manager()
	{
		stop();
		join();

		// destroy the socket for the netlink interface
		delete _sock;

		nl_cache_free(_addr_cache);
		nl_cache_free(_link_cache);

		nl_socket_free(_nl_notify_sock);
		nl_socket_free(_nl_query_sock);
	}

	const std::string NetLink3Manager::getInterface(int index) const
	{
		char buf[256];
		rtnl_link_i2name(_link_cache, index, (char*)&buf, sizeof buf);
		return std::string((char*)&buf);
	}

	const std::list<vaddress> NetLink3Manager::getAddressList(const vinterface &iface, const vaddress::Family f)
	{
		ibrcommon::MutexLock l(_call_mutex);

		if (_refresh_cache)
		{
			nl_cache_free(_addr_cache);
			nl_cache_free(_link_cache);

			// create a cache for the links
			if (rtnl_link_alloc_cache(_nl_query_sock, AF_UNSPEC, &_link_cache) < 0)
			{
				// error
				throw ibrcommon::vsocket_exception("netlink cache allocation failed");
			}

			// create a cache for addresses
			if (rtnl_addr_alloc_cache(_nl_query_sock, &_addr_cache) < 0)
			{
				// error
				nl_cache_free(_link_cache);
				_link_cache = NULL;
				throw ibrcommon::vsocket_exception("netlink cache allocation failed");
			}

			// mark the cache as refreshed
			_refresh_cache = false;
		}

		std::list<vaddress> addresses;

		struct rtnl_addr *filter = rtnl_addr_alloc();
		const std::string i = iface.toString();
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

	void NetLink3Manager::callback(const NetLink3ManagerEvent &lme)
	{
		// ignore if the event is unknown
		if (lme.getType() == LinkManagerEvent::EVENT_UNKOWN) return;

		// ignore if this is an wireless extension event
		if (lme.isWirelessExtension()) return;

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

	void NetLink3Manager::run()
	{
		// add netlink fd to vsocket
		_sock->add(nl_socket_get_fd(_nl_notify_sock));

		try {
			while (_running)
			{
				std::list<int> fds;
				_sock->select(fds, NULL);

				nl_recvmsgs_default(_nl_notify_sock);
			}
		} catch (const vsocket_exception&) {
			// stopped / interrupted
			IBRCOMMON_LOGGER(error) << "NetLink connection stopped" << IBRCOMMON_LOGGER_ENDL;
		}
	}

	void NetLink3Manager::__cancellation()
	{
		_running = false;
		_sock->close();
	}

	/** read a netlink message from the socket and create a new netlink event object **/
	NetLink3ManagerEvent::NetLink3ManagerEvent(struct nl_msg *msg)
	 : _type(EVENT_UNKOWN), _state(0), _wireless(false)
	{
		int attrlen, nlmsg_len, rta_len, rtl;
		struct rtattr *attr;
		struct rtattr *rth;
		struct ifaddrmsg *ifa;
		struct ifinfomsg *ifi;
		struct nlmsghdr *nlh;

		// cast netlink message
		nlh = nlmsg_hdr(msg);

		switch (nlh->nlmsg_type)
		{
			case RTM_BASE:
			{
				ifi = (struct ifinfomsg *) NLMSG_DATA(nlh);

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
						_attributes[attr->rta_type] = std::string((char*)RTA_DATA(attr), rta_length);
						break;
					}

					attr = RTA_NEXT(attr, attrlen);
				}
				break;
			}

			case RTM_NEWADDR:
			{
				ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
				rth = IFA_RTA(ifa);
				rtl = IFA_PAYLOAD(nlh);

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
				ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
				rth = IFA_RTA(ifa);
				rtl = IFA_PAYLOAD(nlh);

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
	}

	NetLink3ManagerEvent::~NetLink3ManagerEvent()
	{
	}

	const ibrcommon::vinterface& NetLink3ManagerEvent::getInterface() const
	{
		return _interface;
	}

	const ibrcommon::vaddress& NetLink3ManagerEvent::getAddress() const
	{
		return _address;
	}

	unsigned int NetLink3ManagerEvent::getState() const
	{
		return _state;
	}

	bool NetLink3ManagerEvent::isWirelessExtension() const
	{
		return _wireless;
	}

	LinkManagerEvent::EventType NetLink3ManagerEvent::getType() const
	{
		return _type;
	}

	const std::string NetLink3ManagerEvent::toString() const
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

	void NetLink3ManagerEvent::debug() const
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
} /* namespace ibrcommon */
