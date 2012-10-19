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
#include <typeinfo>

namespace ibrcommon
{
	netlink_callback::~netlink_callback() { };

	static void nl3_cache_callback(struct nl_cache*, struct nl_object *obj, int action, void *data)
	{
		netlink_callback *c = static_cast<netlink_callback *>(data);
		c->parse(obj, action);
	}

	void add_addr_to_list(struct nl_object *obj, void *data)
	{
		std::list<vaddress> *list = static_cast<std::list<vaddress>*>(data);

		struct nl_addr *naddr = rtnl_addr_get_local((struct rtnl_addr *) obj);

		if (!naddr) return;

		int scope = rtnl_addr_get_scope((struct rtnl_addr *) obj);

		char scope_buf[256];
		rtnl_scope2str(scope, scope_buf, sizeof( scope_buf ));
		std::string scopename(scope_buf);

		char addr_buf[256];
		nl_addr2str( naddr, addr_buf, sizeof( addr_buf ));
		std::string addrname(addr_buf);

		list->push_back( ibrcommon::vaddress(addrname, scopename) );
	}

	NetLink3Manager::netlinkcache::netlinkcache(int protocol, const std::string &name)
	 : _protocol(protocol), _name(name), _nl_socket(NULL), _mngr(NULL), _cache(NULL)
	{
	}

	NetLink3Manager::netlinkcache::~netlinkcache()
	{
	}

	void NetLink3Manager::netlinkcache::up() throw (socket_exception)
	{
		int ret = 0;

		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		// create netlink socket
		_nl_socket = nl_socket_alloc();

		// Disables checking of sequence numbers on the netlink socket
		nl_socket_disable_seq_check(_nl_socket);

		// connect the socket to ROUTE
		ret = nl_connect(_nl_socket, _protocol);

		// allocate a cache manager for ROUTE
		ret = nl_cache_mngr_alloc(_nl_socket, _protocol, NL_AUTO_PROVIDE, &_mngr);

		if (ret != 0)
			throw socket_exception("can not allocate netlink cache manager");

		// add route/link filter to the cache manager
		ret = nl_cache_mngr_add(_mngr, "route/link", &nl3_cache_callback, this, &_cache);
		ret = nl_cache_mngr_add(_mngr, "route/addr", &nl3_cache_callback, this, &_cache);

		// mark this socket as up
		_state = SOCKET_UP;
	}

	void NetLink3Manager::netlinkcache::down() throw (socket_exception)
	{
		if (_state != SOCKET_UP)
			throw socket_exception("socket is not up");

		// delete the cache manager
		nl_cache_mngr_free(_mngr);

		// delete the socket
		nl_socket_free(_nl_socket);

		// mark this socket as down
		_state = SOCKET_DOWN;
	}

	int NetLink3Manager::netlinkcache::fd() const throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("fd not available");
		return nl_cache_mngr_get_fd(_mngr);
	}

	void NetLink3Manager::netlinkcache::receive() throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("socket not connected");
		int ret = nl_cache_mngr_data_ready(_mngr);
		if (ret < 0)
			throw socket_exception("can not receive data from netlink manager");
	}

	void NetLink3Manager::netlinkcache::parse(struct nl_object *obj, int action)
	{
		// TODO: parse nl_objects
		struct nl_dump_params dp;
		dp.dp_type = NL_DUMP_LINE;
		dp.dp_fd = stdout;

		if (action == NL_ACT_NEW)
			printf("NEW ");
		else if (action == NL_ACT_DEL)
			printf("DEL ");
		else if (action == NL_ACT_CHANGE)
			printf("CHANGE ");

		nl_object_dump(obj, &dp);
	}

	struct nl_cache* NetLink3Manager::netlinkcache::operator*() const throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("socket not available");
		return _cache;
	}

	NetLink3Manager::NetLink3Manager()
	 : _route_cache(NETLINK_ROUTE, "route/link"), _running(true)
	{
		// initialize the netlink cache
		_route_cache.up();

//		// initialize the sockets
//		_nl_notify_sock.up();
//		_nl_query_sock.up();
//
//		// disable seq check for notify socket
//		nl_socket_disable_seq_check(*_nl_notify_sock);
//
//		// define callback method
//		nl_socket_modify_cb(*_nl_notify_sock, NL_CB_VALID, NL_CB_CUSTOM, nl3_callback, this);
//
//		// connect to routing netlink protocol
//		nl_connect(*_nl_notify_sock, NETLINK_ROUTE);
//		nl_connect(*_nl_query_sock, NETLINK_ROUTE);
//
//		// init route messages
//		nl_socket_add_memberships(*_nl_notify_sock, RTNLGRP_IPV4_IFADDR);
//		nl_socket_add_memberships(*_nl_notify_sock, RTNLGRP_IPV6_IFADDR);
//		nl_socket_add_memberships(*_nl_notify_sock, RTNLGRP_LINK);
//
//		// create a cache for the links
//		if (rtnl_link_alloc_cache(*_nl_query_sock, AF_UNSPEC, &_link_cache) < 0)
//		{
//			_nl_notify_sock.down();
//			_nl_query_sock.down();
//			// error
//			throw ibrcommon::socket_exception("netlink cache allocation failed");
//		}
//
//		// create a cache for addresses
//		if (rtnl_addr_alloc_cache(*_nl_query_sock, &_addr_cache) < 0)
//		{
//			_nl_notify_sock.down();
//			_nl_query_sock.down();
//			// error
//			nl_cache_free(_link_cache);
//			_link_cache = NULL;
//			throw ibrcommon::socket_exception("netlink cache allocation failed");
//		}
//
//		// create a new socket for the netlink interface
//		_sock = new ibrcommon::vsocket();
	}

	NetLink3Manager::~NetLink3Manager()
	{
		stop();
		join();

		// destroy netlink cache
		_route_cache.down();
	}

	const std::string NetLink3Manager::getInterface(int index) const
	{
		char buf[256];
		rtnl_link_i2name(*_route_cache, index, (char*)&buf, sizeof buf);
		return std::string((char*)&buf);
	}

	const std::list<vaddress> NetLink3Manager::getAddressList(const vinterface &iface)
	{
		ibrcommon::MutexLock l(_cache_mutex);

		std::list<vaddress> addresses;

		struct rtnl_addr *filter = rtnl_addr_alloc();
		const std::string i = iface.toString();
		rtnl_addr_set_ifindex(filter, rtnl_link_name2i(*_route_cache, i.c_str()));

//		rtnl_addr_set_family(filter, AF_INET6);
		nl_cache_foreach_filter(*_route_cache, (struct nl_object *) filter, add_addr_to_list, &addresses);

//		rtnl_addr_set_family(filter, AF_INET);
//		nl_cache_foreach_filter(_addr_cache, (struct nl_object *) filter,
//								add_addr_to_list, &addresses);

		rtnl_addr_put(filter);

		return addresses;
	}

	void NetLink3Manager::callback(const NetLink3ManagerEvent &lme)
	{
		// ignore if the event is unknown
		if (lme.getType() == LinkManagerEvent::EVENT_UNKOWN) return;

		// ignore if this is an wireless extension event
		if (lme.isWirelessExtension()) return;

		// print out some debugging
		IBRCOMMON_LOGGER_DEBUG(10) << lme.toString() << IBRCOMMON_LOGGER_ENDL;

		// notify all subscribers about this event
		raiseEvent(lme);
	}

	void NetLink3Manager::run()
	{
		// add netlink fd to vsocket
		_sock.add(&_route_cache);

		try {
			while (_running)
			{
				socketset socks;
				_sock.select(&socks, NULL, NULL, NULL);

				for (socketset::iterator iter = socks.begin(); iter != socks.end(); iter++) {
					try {
						netlinkcache &cache = dynamic_cast<netlinkcache&>(**iter);
						cache.receive();
					} catch (const bad_cast&) { };
				}
			}
		} catch (const socket_exception&) {
			// stopped / interrupted
			IBRCOMMON_LOGGER(error) << "NetLink connection stopped" << IBRCOMMON_LOGGER_ENDL;
		}
	}

	void NetLink3Manager::__cancellation()
	{
		_running = false;
		_sock.down();
	}

	/** read a netlink message from the socket and create a new netlink event object **/
	NetLink3ManagerEvent::NetLink3ManagerEvent(EventType type, unsigned int state,
			ibrcommon::vinterface iface, ibrcommon::vaddress addr, bool wireless)
	 : _type(type), _state(state), _wireless(wireless), _interface(iface), _address(addr)
	{
//		int attrlen, nlmsg_len, rta_len, rtl;
//		struct rtattr *attr;
//		struct rtattr *rth;
//		struct ifaddrmsg *ifa;
//		struct ifinfomsg *ifi;
//		struct nlmsghdr *nlh;
//
//		// cast netlink message
//		nlh = nlmsg_hdr(msg);
//
//		switch (nlh->nlmsg_type)
//		{
//			case RTM_BASE:
//			{
//				ifi = (struct ifinfomsg *) NLMSG_DATA(nlh);
//
//				nlmsg_len = NLMSG_ALIGN(sizeof(struct ifinfomsg));
//				attrlen = nlh->nlmsg_len - nlmsg_len;
//
//				if (attrlen < 0) break;
//
//				attr = (struct rtattr *) (((char *) ifi) + nlmsg_len);
//
//				rta_len = RTA_ALIGN(sizeof(struct rtattr));
//				while (RTA_OK(attr, attrlen))
//				{
//					size_t rta_length = RTA_PAYLOAD(attr);
//
//					switch (attr->rta_type)
//					{
//					case IFLA_IFNAME:
//						_interface = ibrcommon::vinterface( std::string((char*)RTA_DATA(attr), rta_length) );
//						_type = EVENT_LINK_STATE;
//						break;
//
//					case IFLA_OPERSTATE:
//					{
//						char s;
//						::memcpy(&s, (char*)RTA_DATA(attr), 1);
//						_state = s;
//						break;
//					}
//
//					case IFLA_WIRELESS:
//						_wireless = true;
//						break;
//
//					default:
//						_attributes[attr->rta_type] = std::string((char*)RTA_DATA(attr), rta_length);
//						break;
//					}
//
//					attr = RTA_NEXT(attr, attrlen);
//				}
//				break;
//			}
//
//			case RTM_NEWADDR:
//			{
//				ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
//				rth = IFA_RTA(ifa);
//				rtl = IFA_PAYLOAD(nlh);
//
//				// parse all attributes
//				while (rtl && RTA_OK(rth, rtl))
//				{
//					switch (rth->rta_type)
//					{
//						// local address
//						case IFA_LOCAL:
//						{
//							char address[256];
//
//							uint32_t ipaddr = htonl(*((uint32_t *)RTA_DATA(rth)));
//							sprintf(address, "%d.%d.%d.%d", (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);
//
//							_address = ibrcommon::vaddress(std::string(address));
//							_type = EVENT_ADDRESS_ADDED;
//							break;
//						}
//
//						// interface name
//						case IFA_LABEL:
//						{
//							//char name[IFNAMSIZ];
//							char *name = (char *)RTA_DATA(rth);
//							_interface = ibrcommon::vinterface(name);
//							break;
//						}
//					}
//					rth = RTA_NEXT(rth, rtl);
//				}
//				break;
//			}
//
//			case RTM_DELADDR:
//			{
//				ifa = (struct ifaddrmsg *) NLMSG_DATA(nlh);
//				rth = IFA_RTA(ifa);
//				rtl = IFA_PAYLOAD(nlh);
//
//				// parse all attributes
//				while (rtl && RTA_OK(rth, rtl))
//				{
//					switch (rth->rta_type)
//					{
//						// local address
//						case IFA_LOCAL:
//						{
//							char address[256];
//
//							uint32_t ipaddr = htonl(*((uint32_t *)RTA_DATA(rth)));
//							sprintf(address, "%d.%d.%d.%d", (ipaddr >> 24) & 0xff, (ipaddr >> 16) & 0xff, (ipaddr >> 8) & 0xff, ipaddr & 0xff);
//
//							_address = ibrcommon::vaddress(std::string(address));
//							_type = EVENT_ADDRESS_REMOVED;
//							break;
//						}
//
//						// interface name
//						case IFA_LABEL:
//						{
//							//char name[IFNAMSIZ];
//							char *name = (char *)RTA_DATA(rth);
//							_interface = ibrcommon::vinterface(name);
//							break;
//						}
//					}
//					rth = RTA_NEXT(rth, rtl);
//				}
//				break;
//			}
//
//			default:
//				IBRCOMMON_LOGGER_DEBUG(10) << "unknown netlink type received: " << nlh->nlmsg_type << IBRCOMMON_LOGGER_ENDL;
//				break;
//		}
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
} /* namespace ibrcommon */
