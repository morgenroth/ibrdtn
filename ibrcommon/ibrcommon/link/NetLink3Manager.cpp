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

#include "ibrcommon/link/NetLink3Manager.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/Logger.h"

#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/route/rtnl.h>
#include <netlink/socket.h>
#include <netlink/msg.h>

#include <string.h>
#include <typeinfo>

namespace ibrcommon
{
	netlink_callback::~netlink_callback() { };

	static void nl_cache_callback(struct nl_cache*, struct nl_object *obj, int action, void *data)
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

		list->push_back( ibrcommon::vaddress(addrname, "", scopename) );
	}

	NetLink3Manager::netlinkcache::netlinkcache(int protocol)
	 : _protocol(protocol), _nl_socket(NULL), _mngr(NULL), _cache(NULL)
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

//		// Disables checking of sequence numbers on the netlink socket
//		nl_socket_disable_seq_check(_nl_socket);

//		// connect the socket to ROUTE
//		ret = nl_connect(_nl_socket, _protocol);

		// allocate a cache manager for ROUTE
		ret = nl_cache_mngr_alloc(_nl_socket, _protocol, NL_AUTO_PROVIDE, &_mngr);

		if (ret != 0)
			throw socket_exception("can not allocate netlink cache manager");

		// add route/link filter to the cache manager
		ret = nl_cache_mngr_add(_mngr, "route/link", &nl_cache_callback, this, &_cache);
		ret = nl_cache_mngr_add(_mngr, "route/addr", &nl_cache_callback, this, &_cache);

		// mark this socket as up
		_state = SOCKET_UP;
	}

	void NetLink3Manager::netlinkcache::down() throw (socket_exception)
	{
		if ((_state == SOCKET_DOWN) || (_state == SOCKET_DESTROYED))
			throw socket_exception("socket is not up");

		// delete the cache manager
		nl_cache_mngr_free(_mngr);

		// delete the socket
		nl_socket_free(_nl_socket);

		// mark this socket as down
		if (_state == SOCKET_UNMANAGED)
			_state = SOCKET_DESTROYED;
		else
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
		memset(&dp, 0, sizeof(struct nl_dump_params));

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
	}

	NetLink3Manager::~NetLink3Manager()
	{
		stop();
		join();

		// destroy netlink cache
		_route_cache.down();
	}

	void NetLink3Manager::up() throw ()
	{
		this->start();
	}

	void NetLink3Manager::down() throw ()
	{
		this->stop();
		this->join();
	}

	const vinterface NetLink3Manager::getInterface(int index) const
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

	void NetLink3Manager::callback(const LinkEvent &lme)
	{
		// ignore if the event is unknown
		if (lme.getType() == LinkEvent::EVENT_UNKOWN) return;

//		// ignore if this is an wireless extension event
//		if (lme.isWirelessExtension()) return;

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
} /* namespace ibrcommon */
