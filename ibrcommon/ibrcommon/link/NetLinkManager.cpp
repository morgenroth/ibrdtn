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

#include "ibrcommon/link/NetLinkManager.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/Logger.h"

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
#include <typeinfo>

namespace ibrcommon
{
	static void nl_cache_callback(struct nl_cache*, struct nl_object *obj, int action)
	{
		if (obj == NULL) return;

		// TODO: parse nl_objects
		struct nl_dump_params dp;
		memset(&dp, 0, sizeof(struct nl_dump_params));
		dp.dp_type = NL_DUMP_BRIEF;
		dp.dp_fd = stdout;

		if (action == NL_ACT_NEW)
			printf("NEW ");
		else if (action == NL_ACT_DEL)
			printf("DEL ");
		else if (action == NL_ACT_CHANGE)
			printf("CHANGE ");

		nl_object_dump(obj, &dp);
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

	NetLinkManager::netlinkcache::netlinkcache(int protocol)
	 : _protocol(protocol), _nl_handle(NULL), _mngr(NULL), _link_cache(NULL), _addr_cache(NULL)
	{
	}

	NetLinkManager::netlinkcache::~netlinkcache()
	{
	}

	void NetLinkManager::netlinkcache::up() throw (socket_exception)
	{
		int ret = 0;

		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		_nl_handle = nl_handle_alloc();

//		// Disables checking of sequence numbers on the netlink socket
//		nl_socket_disable_seq_check(_nl_handle);

//		// connect the socket to ROUTE
//		ret = nl_connect(_nl_handle, _protocol);

		// allocate a cache manager for ROUTE
		_mngr = nl_cache_mngr_alloc(_nl_handle, _protocol, NL_AUTO_PROVIDE);

		if (_mngr == NULL)
			throw socket_exception("can not allocate netlink cache manager");

		_link_cache = nl_cache_mngr_add(_mngr, "route/link", &nl_cache_callback);

		if (_link_cache == NULL)
			throw socket_exception("can not allocate netlink cache route/link");

		_addr_cache = nl_cache_mngr_add(_mngr, "route/addr", &nl_cache_callback);

		if (_addr_cache == NULL)
			throw socket_exception("can not allocate netlink cache route/addr");

		// mark this socket as up
		_state = SOCKET_UP;
	}

	void NetLinkManager::netlinkcache::down() throw (socket_exception)
	{
		if ((_state == SOCKET_DOWN) || (_state == SOCKET_DESTROYED))
			throw socket_exception("socket is not up");

		nl_cache_free(_link_cache);
		nl_cache_free(_addr_cache);

		// delete the cache manager
		nl_cache_mngr_free(_mngr);

		nl_close(_nl_handle);
		nl_handle_destroy(_nl_handle);

		// mark this socket as down
		if (_state == SOCKET_UNMANAGED)
			_state = SOCKET_DESTROYED;
		else
			_state = SOCKET_DOWN;
	}

	int NetLinkManager::netlinkcache::fd() const throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("fd not available");
		return nl_cache_mngr_get_fd(_mngr);
	}

	void NetLinkManager::netlinkcache::receive() throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("socket not connected");
		int ret = nl_cache_mngr_data_ready(_mngr);
		if (ret < 0)
			throw socket_exception("can not receive data from netlink manager");
	}

	struct nl_cache* NetLinkManager::netlinkcache::operator*() const throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("socket not available");
		return _addr_cache;
	}

	NetLinkManager::NetLinkManager()
	 : _route_cache(NETLINK_ROUTE), _running(true)
	{
		// initialize the netlink cache
		_route_cache.up();
	}

	NetLinkManager::~NetLinkManager()
	{
		stop();
		join();

		// destroy netlink cache
		_route_cache.down();
	}

	void NetLinkManager::up() throw ()
	{
		this->start();
	}

	void NetLinkManager::down() throw ()
	{
		this->stop();
		this->join();
	}

	void NetLinkManager::__cancellation()
	{
		_running = false;
		_sock.down();
	}

	void NetLinkManager::run()
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

	const std::list<vaddress> NetLinkManager::getAddressList(const vinterface &iface)
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

	const vinterface NetLinkManager::getInterface(const int ifindex) const
	{
		char buf[256];
		rtnl_link_i2name(*_route_cache, ifindex, (char*)&buf, sizeof buf);
		return std::string((char*)&buf);
	}
}
