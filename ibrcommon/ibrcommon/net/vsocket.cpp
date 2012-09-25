/*
 * vsocket.cpp
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

#include "ibrcommon/config.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/TimeMeasurement.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/Logger.h"

#include <algorithm>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <errno.h>
#include <sstream>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace ibrcommon
{
	int __nonlinux_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
	{
		if (timeout == NULL)
		{
			return ::select(nfds, readfds, writefds, exceptfds, NULL);
		}

		TimeMeasurement tm;

		struct timeval to_copy;
		::memcpy(&to_copy, timeout, sizeof to_copy);

		tm.start();
		int ret = ::select(nfds, readfds, writefds, exceptfds, &to_copy);
		tm.stop();

		uint64_t us = tm.getMicroseconds();

		while ((us > 1000000) && (timeout->tv_sec > 0))
		{
			us -= 1000000;
			timeout->tv_sec--;
		}

		if (us >= (uint64_t)timeout->tv_usec)
		{
			timeout->tv_usec = 0;
		}
		else
		{
			timeout->tv_usec -= us;
		}

		return ret;
	}

	void vsocket::set_non_blocking(int fd, bool nonblock)
	{
		int opts;
		opts = fcntl(fd,F_GETFL);
		if (opts < 0) {
			throw vsocket_exception("cannot set non-blocking");
		}

		if (nonblock)
			opts |= O_NONBLOCK;
		else
			opts &= ~(O_NONBLOCK);

		if (fcntl(fd,F_SETFL,opts) < 0) {
			throw vsocket_exception("cannot set non-blocking");
		}
	}

	vsocket::vsocket(bool sendonly)
	 : _options(0), _interrupt(false), _listen_connections(0), _cb(NULL), _send_only(sendonly)
	{
		// create a pipe for interruption
		if (pipe(_interrupt_pipe) < 0)
		{
			IBRCOMMON_LOGGER(error) << "Error " << errno << " creating pipe" << IBRCOMMON_LOGGER_ENDL;
			throw vsocket_exception("failed to create pipe");
		}

		// set the pipe to non-blocking
		vsocket::set_non_blocking(_interrupt_pipe[0]);
		vsocket::set_non_blocking(_interrupt_pipe[1]);
	}

	vsocket::~vsocket()
	{
		ibrcommon::LinkManager::getInstance().unregisterAllEvents(this);

		// close all used pipes
		::close(_interrupt_pipe[0]);
		::close(_interrupt_pipe[1]);
	}

	int vsocket::bind(const vsocket::vbind &b)
	{
		_binds.push_back(b);
		vsocket::vbind &vb = _binds.back();

		try {
			if (_options & VSOCKET_REUSEADDR) vb.set(VSOCKET_REUSEADDR);
			if (_options & VSOCKET_BROADCAST) vb.set(VSOCKET_BROADCAST);
			if (_options & VSOCKET_MULTICAST) vb.set(VSOCKET_MULTICAST);

			vb.bind();

			if (_options & VSOCKET_LINGER) vb.set(VSOCKET_LINGER);
			if (_options & VSOCKET_NODELAY) vb.set(VSOCKET_NODELAY);
			if (_options & VSOCKET_NONBLOCKING) vb.set(VSOCKET_NONBLOCKING);

			// join all already assigned groups
			for (mcast_groups::iterator iter = _groups.begin();
					iter != _groups.end(); iter++) {
				std::set<ibrcommon::vinterface> ifaces = (*iter).second;

				for (std::set<ibrcommon::vinterface>::iterator it_iface = ifaces.begin();
						it_iface != ifaces.end(); it_iface++) {
					vb.join(iter->first, *it_iface);
				}
			}
		} catch (const vsocket_exception&) {
			_binds.pop_back();
			throw;
		}

		return vb._fd;
	}

	std::set<int> vsocket::bind(const vinterface &iface, const int port, unsigned int socktype)
	{
		std::set<int> ret;
		if (iface.empty()) { bind(port); return ret; }

		// remember the port for dynamic bind/unbind
		_portmap[iface] = port;
		_typemap[iface] = socktype;

		// watch at events on this interface
		ibrcommon::LinkManager::getInstance().registerInterfaceEvent(iface, this);

		// bind on all interfaces of "iface"!
		const std::list<vaddress> addrlist = iface.getAddresses();

		for (std::list<vaddress>::const_iterator iter = addrlist.begin(); iter != addrlist.end(); iter++)
		{
			if ((*iter).getFamily() == ibrcommon::vaddress::VADDRESS_INET6)
				if ((_options & VSOCKET_LINKLOCAL) && ((*iter).getScope() != ibrcommon::vaddress::SCOPE_LINKLOCAL)) continue;

			if (port == 0)
			{
				vsocket::vbind vb(iface, (*iter), socktype);
				ret.insert( bind( vb ) );
			}
			else
			{
				vsocket::vbind vb(iface, (*iter), port, socktype);
				ret.insert( bind( vb ) );
			}
		}

		return ret;
	}

	void vsocket::unbind(const vinterface &iface, const int port)
	{
		// delete the watch at events on this interface
		ibrcommon::LinkManager::getInstance().unregisterInterfaceEvent(iface, this);

		// unbind all interfaces on interface "iface"!
		const std::list<vaddress> addrlist = iface.getAddresses();

		for (std::list<vaddress>::const_iterator iter = addrlist.begin(); iter != addrlist.end(); iter++)
		{
			if ((*iter).getFamily() == ibrcommon::vaddress::VADDRESS_INET6)
				if ((_options & VSOCKET_LINKLOCAL) && ((*iter).getScope() != ibrcommon::vaddress::SCOPE_LINKLOCAL)) continue;
			unbind( *iter, port );
		}
	}

	std::set<int> vsocket::bind(const int port, unsigned int socktype)
	{
		std::set<int> ret;

		vaddress addr6(vaddress::VADDRESS_INET6);
		std::set<int> addr6_ret = bind( addr6, port, socktype );
		ret.insert( addr6_ret.begin(), addr6_ret.end() );

		vaddress addr(vaddress::VADDRESS_INET);
		std::set<int> addr_ret = bind( addr, port, socktype );
		ret.insert( addr_ret.begin(), addr_ret.end() );

		return ret;
	}

	void vsocket::unbind(const int port)
	{
		vaddress addr6(vaddress::VADDRESS_INET6);
		unbind( addr6, port );

		vaddress addr(vaddress::VADDRESS_INET);
		unbind( addr, port );
	}

	std::set<int> vsocket::bind(const vaddress &address, const int port, unsigned int socktype)
	{
		std::set<int> ret;
		vsocket::vbind vb(address, port, socktype);
		ret.insert( bind( vb ) );
		return ret;
	}

	std::set<int> vsocket::bind(const ibrcommon::File &file, unsigned int socktype)
	{
		std::set<int> ret;
		vsocket::vbind vb(file, socktype);
		ret.insert( bind( vb ) );
		return ret;
	}

	void vsocket::unbind(const vaddress &address, const int port)
	{
		for (std::list<vsocket::vbind>::iterator iter = _binds.begin(); iter != _binds.end(); iter++)
		{
			vsocket::vbind &b = (*iter);
			if ((b._vaddress == address) && (b._port == port))
			{
				_unbind_queue.push(b);
			}
		}
	}

	void vsocket::unbind(const ibrcommon::File &file)
	{
		for (std::list<vsocket::vbind>::iterator iter = _binds.begin(); iter != _binds.end(); iter++)
		{
			vsocket::vbind &b = (*iter);
			if (b._file == file)
			{
				_unbind_queue.push(b);
			}
		}
	}

	void vsocket::add(const int fd)
	{
		vsocket::vbind vb(fd);
		bind(vb);
	}

	void vsocket::listen(int connections)
	{
		ibrcommon::MutexLock l(_bind_lock);
		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			bind.listen(connections);
		}

		_listen_connections = connections;
	}

	void vsocket::relisten()
	{
		if (_listen_connections > 0)
			listen(_listen_connections);
	}

	void vsocket::set(const Option &o)
	{
		// set options
		_options |= o;

		ibrcommon::MutexLock l(_bind_lock);
		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			bind.set(o);
		}
	}

	void vsocket::unset(const Option &o)
	{
		// unset options
		_options &= ~(o);

		ibrcommon::MutexLock l(_bind_lock);
		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			bind.unset(o);
		}
	}

	void vsocket::join(const ibrcommon::vaddress &group, const ibrcommon::vinterface &iface)
	{
		_groups[group].insert(iface);

		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			bind.join(group, iface);
		}
	}

	void vsocket::leave(const ibrcommon::vaddress &group)
	{
		const std::set<ibrcommon::vinterface> &ifaces = _groups[group];

		for (std::set<ibrcommon::vinterface>::const_iterator it_ifaces = ifaces.begin();
				it_ifaces != ifaces.end(); it_ifaces++) {
			const ibrcommon::vinterface &iface = (*it_ifaces);

			for (std::list<ibrcommon::vsocket::vbind>::iterator it_bind = _binds.begin();
					it_bind != _binds.end(); it_bind++)
			{
				ibrcommon::vsocket::vbind &bind = (*it_bind);
				bind.leave(group, iface);
			}
		}

		_groups.erase(group);
	}

	void vsocket::close()
	{
		ibrcommon::MutexLock l(_bind_lock);
		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			bind.close();
			_binds.erase(iter++);
		}

		interrupt();
	}

	void vsocket::shutdown()
	{
		ibrcommon::MutexLock l(_bind_lock);
		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			bind.shutdown();
		}

		interrupt();
	}

	int vsocket::fd()
	{
		if (_binds.empty()) return -1;
		return _binds.front()._fd;
	}

	void vsocket::eventNotify(const LinkManagerEvent &evt)
	{
		const ibrcommon::vinterface &iface = evt.getInterface();
		IBRCOMMON_LOGGER_DEBUG(5) << "update socket cause of event on interface " << iface.toString() << IBRCOMMON_LOGGER_ENDL;

		// check if the portmap for this interface is available
		if (_portmap.find(evt.getInterface()) == _portmap.end()) return;

		try {
			switch (evt.getType())
			{
			case LinkManagerEvent::EVENT_ADDRESS_ADDED:
			{
				IBRCOMMON_LOGGER_DEBUG(10) << "dynamic address bind on: " << evt.getAddress().toString() << ":" << _portmap[iface] << IBRCOMMON_LOGGER_ENDL;
				if (_portmap[iface] == 0)
				{
					vsocket::vbind vb(iface, evt.getAddress(), _typemap[iface]);
					bind( vb );
				}
				else
				{
					vsocket::vbind vb(iface, evt.getAddress(), _portmap[iface], _typemap[iface]);
					bind( vb );
				}
				break;
			}

			case LinkManagerEvent::EVENT_ADDRESS_REMOVED:
				IBRCOMMON_LOGGER_DEBUG(10) << "dynamic address unbind on: " << evt.getAddress().toString() << ":" << _portmap[iface] << IBRCOMMON_LOGGER_ENDL;
				unbind(evt.getAddress(), _portmap[iface]);
				break;

			default:
				break;
			}
		} catch (const ibrcommon::Exception &ex) {
			IBRCOMMON_LOGGER(warning) << "dynamic bind process failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
		}

		// forward the event to the listen callback class
		if (_cb != NULL) _cb->eventNotify(evt);

		if (_send_only) {
			// if this is send only socket, then process unbinds directly
			process_unbind_queue();
		} else {
			// refresh the select call
			refresh();
		}
	}

	void vsocket::setEventCallback(ibrcommon::LinkManager::EventCallback *cb)
	{
		_cb = cb;
	}

	const vsocket::fd_address_list vsocket::get(const ibrcommon::vinterface &iface)
	{
		fd_address_list ret;

		ibrcommon::MutexLock l(_bind_lock);
		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			if (bind._interface == iface)
			{
				ret.push_back( make_pair(bind._vaddress, bind._fd) );
			}
		}

		return ret;
	}

	const vsocket::fd_address_list vsocket::get()
	{
		fd_address_list ret;

		ibrcommon::MutexLock l(_bind_lock);
		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
				iter != _binds.end(); iter++)
		{
			ibrcommon::vsocket::vbind &bind = (*iter);
			ret.push_back( make_pair(bind._vaddress, bind._fd) );
		}

		return ret;
	}

	int vsocket::sendto(const void *buf, size_t n, const ibrcommon::vaddress &address, const unsigned int port)
	{
		try {
			size_t ret = 0;
			int flags = 0;

			struct addrinfo hints, *ainfo;
			memset(&hints, 0, sizeof hints);

			hints.ai_socktype = SOCK_DGRAM;
			ainfo = address.addrinfo(&hints, port);

			ibrcommon::MutexLock l(_bind_lock);
			for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
					iter != _binds.end(); iter++)
			{
				ibrcommon::vsocket::vbind &bind = (*iter);
				if (bind._vaddress.getFamily() == address.getFamily())
				{
					std::cout << "send to interface " << bind._interface.toString() << "; " << bind._vaddress.toString() << std::endl;
					ret = ::sendto(bind._fd, buf, n, flags, ainfo->ai_addr, ainfo->ai_addrlen);
				}
			}

			freeaddrinfo(ainfo);

			return ret;
		} catch (const vsocket_exception&) {
			IBRCOMMON_LOGGER_DEBUG(5) << "can not send message to " << address.toString() << IBRCOMMON_LOGGER_ENDL;
		}

		return -1;
	}

	int recvfrom(int fd, char* data, size_t maxbuffer, std::string &address)
	{
		struct sockaddr_storage clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);

		// data waiting
		int ret = ::recvfrom(fd, data, maxbuffer, MSG_WAITALL, (struct sockaddr *) &clientAddress, &clientAddressLength);

		char str[256];
		::getnameinfo((struct sockaddr *) &clientAddress, clientAddressLength, str, 256, 0, 0, NI_NUMERICHOST);

		address = std::string(str);

		return ret;
	}

	void vsocket::refresh()
	{
		::write(_interrupt_pipe[1], "i", 1);
	}

	void vsocket::interrupt()
	{
		_interrupt = true;
		::write(_interrupt_pipe[1], "i", 1);
	}

	void vsocket::select(std::list<int> &fds, struct timeval *tv)
	{
		fd_set fds_read;

		int high_fd = 0;
		int fd_count = 0;

		// clear the fds list
		fds.clear();

		while (true)
		{
			FD_ZERO(&fds_read);

			// add the self-pipe-trick interrupt fd
			FD_SET(_interrupt_pipe[0], &fds_read);
			high_fd = _interrupt_pipe[0];

			{
				ibrcommon::MutexLock l(_bind_lock);
				std::list<ibrcommon::vsocket::vbind> &socks = _binds;
				for (std::list<ibrcommon::vsocket::vbind>::iterator iter = socks.begin();
						iter != socks.end(); iter++)
				{
					ibrcommon::vsocket::vbind &bind = (*iter);

					FD_SET(bind._fd, &fds_read);
					if (high_fd < bind._fd) high_fd = bind._fd;

					fd_count++;
				}
			}

			if (fd_count == 0)
				throw vsocket_exception("select error");

#ifdef HAVE_FEATURES_H
			int res = ::select(high_fd + 1, &fds_read, NULL, NULL, tv);
#else
			int res = __nonlinux_select(high_fd + 1, &fds_read, NULL, NULL, tv);
#endif

			if (res < 0)
				throw vsocket_exception("select error");

			if (res == 0)
				throw vsocket_timeout("select timeout");

			if (FD_ISSET(_interrupt_pipe[0], &fds_read))
			{
				IBRCOMMON_LOGGER_DEBUG(25) << "unblocked by self-pipe-trick" << IBRCOMMON_LOGGER_ENDL;

				// this was an interrupt with the self-pipe-trick
				char buf[2];
				::read(_interrupt_pipe[0], buf, 2);

				// process the unbind queue
				process_unbind_queue();

				// listen on all new binds
				relisten();

				// interrupt the method if requested
				if (_interrupt)
				{
					_interrupt = false;
					throw vsocket_interrupt("select interrupted");
				}

				// start over with the select call
				continue;
			}

			ibrcommon::MutexLock l(_bind_lock);
			std::list<ibrcommon::vsocket::vbind> &socks = _binds;
			for (std::list<ibrcommon::vsocket::vbind>::iterator iter = socks.begin();
					iter != socks.end(); iter++)
			{
				ibrcommon::vsocket::vbind &bind = (*iter);

				if (FD_ISSET(bind._fd, &fds_read))
				{
					fds.push_back(bind._fd);
				}
			}

			if (fds.size() > 0) return;
		}
	}

	void vsocket::process_unbind_queue() {
		if (!_unbind_queue.empty())
		{
			// unbind all removed sockets now
			ibrcommon::Queue<vsocket::vbind>::Locked lq = _unbind_queue.exclusive();

			vsocket::vbind &vb = lq.front();

			for (std::list<vsocket::vbind>::iterator iter = _binds.begin(); iter != _binds.end(); iter++)
			{
				vsocket::vbind &i = (*iter);

				if (i == vb)
				{
					IBRCOMMON_LOGGER_DEBUG(25) << "socket closed" << IBRCOMMON_LOGGER_ENDL;
					i.close();
					_binds.erase(iter);
					break;
				}
			}

			lq.pop();
		}
	}

	vsocket::vbind::vbind(const int fd)
	 : _type(BIND_CUSTOM), _vaddress(), _port(), _fd(fd)
	{
		// check for errors
		if (_fd < 0) try {
			check_socket_error( _fd );
		} catch (const std::exception&) {
			close();
			throw;
		}
	}

	vsocket::vbind::vbind(const vaddress &address, unsigned int socktype)
	 : _type(BIND_ADDRESS_NOPORT), _vaddress(address), _port(0), _fd(0)
	{
		_fd = socket(address.getFamily(), socktype, 0);

		// check for errors
		if (_fd < 0) try {
			check_socket_error( _fd );
		} catch (const std::exception&) {
			close();
			throw;
		}
	}

	vsocket::vbind::vbind(const vaddress &address, const int port, unsigned int socktype)
	 : _type(BIND_ADDRESS), _vaddress(address), _port(port), _fd(0)
	{
		_fd = socket(address.getFamily(), socktype, 0);

		// check for errors
		if (_fd < 0) try {
			check_socket_error( _fd );
		} catch (const std::exception&) {
			close();
			throw;
		}
	}

	vsocket::vbind::vbind(const ibrcommon::vinterface &iface, const vaddress &address, unsigned int socktype)
	 : _type(BIND_ADDRESS_NOPORT), _vaddress(address), _port(0), _interface(iface), _fd(0)
	{
		_fd = socket(address.getFamily(), socktype, 0);

		// check for errors
		if (_fd < 0) try {
			check_socket_error( _fd );
		} catch (const std::exception&) {
			close();
			throw;
		}
	}

	vsocket::vbind::vbind(const ibrcommon::vinterface &iface, const vaddress &address, const int port, unsigned int socktype)
	 : _type(BIND_ADDRESS), _vaddress(address), _port(port), _interface(iface), _fd(0)
	{
		_fd = socket(address.getFamily(), socktype, 0);

		// check for errors
		if (_fd < 0) try {
			check_socket_error( _fd );
		} catch (const std::exception&) {
			close();
			throw;
		}
	}

	vsocket::vbind::vbind(const ibrcommon::File &file, unsigned int socktype)
	 : _type(BIND_FILE), _port(0), _file(file), _fd(0)
	{
		_fd = socket(AF_UNIX, socktype, 0);

		// check for errors
		if (_fd < 0) try {
			check_socket_error( _fd );
		} catch (const std::exception&) {
			close();
			throw;
		}
	}

	vsocket::vbind::~vbind()
	{
	}

	void vsocket::vbind::bind()
	{
		int bind_ret = 0;

		switch (_type)
		{
			case BIND_CUSTOM:
			{
				// custom fd, do nothing
				break;
			}

			case BIND_ADDRESS_NOPORT:
			{
				struct addrinfo hints, *res;
				memset(&hints, 0, sizeof hints);

				hints.ai_family = _vaddress.getFamily();
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_flags = AI_PASSIVE;

				res = _vaddress.addrinfo(&hints);
				bind_ret = ::bind(_fd, res->ai_addr, res->ai_addrlen);
				freeaddrinfo(res);
				break;
			}

			case BIND_ADDRESS:
			{
				struct addrinfo hints, *res;
				memset(&hints, 0, sizeof hints);

				hints.ai_family = _vaddress.getFamily();
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_flags = AI_PASSIVE;

				res = _vaddress.addrinfo(&hints, _port);
				bind_ret = ::bind(_fd, res->ai_addr, res->ai_addrlen);
				freeaddrinfo(res);
				break;
			}

			case BIND_FILE:
			{
				// remove old sockets
				unlink(_file.getPath().c_str());

				struct sockaddr_un address;
				size_t address_length;

				address.sun_family = AF_UNIX;
				strcpy(address.sun_path, _file.getPath().c_str());
				address_length = sizeof(address.sun_family) + strlen(address.sun_path);

				// bind to the socket
				bind_ret = ::bind(_fd, (struct sockaddr *) &address, address_length);

				break;
			}
		}

		if ( bind_ret < 0) check_bind_error( errno );
	}

	void vsocket::vbind::listen(int connections)
	{
		if(::listen(_fd, connections) != 0)
		{
			throw ibrcommon::vsocket_exception("cannot listen to socket");
		}
	}

	void vsocket::vbind::set(const vsocket::Option &o)
	{
		switch (o)
		{
			default:
				break;

			case VSOCKET_REUSEADDR:
			{
				int on = 1;
				if (::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
				{
					throw vsocket_exception("setsockopt(SO_REUSEADDR) failed");
				}
				break;
			}

			case VSOCKET_LINGER:
			{
				int set = 1;
				::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&set, sizeof(set));
				break;
			}

			case VSOCKET_NODELAY:
			{
				// set linger option to the socket
				struct linger linger;

				linger.l_onoff = 1;
				linger.l_linger = 1;
				::setsockopt(_fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
				break;
			}

			case VSOCKET_BROADCAST:
			{
				int b = 1;
				if ( ::setsockopt(_fd, SOL_SOCKET, SO_BROADCAST, (char*)&b, sizeof(b)) == -1 )
				{
					throw vsocket_exception("cannot enable broadcasts");
				}
				break;
			}

			case VSOCKET_NONBLOCKING:
			{
				vsocket::set_non_blocking(_fd);
				break;
			}

			case VSOCKET_MULTICAST:
			{
				if (_vaddress.getFamily() == ibrcommon::vaddress::VADDRESS_INET) {
#ifdef HAVE_FEATURES_H
					int val = 1;
					if ( ::setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
					{
						throw vsocket_exception("setsockopt(IP_MULTICAST_LOOP)");
					}

					u_char ttl = 255; // Multicast TTL
					if ( ::setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0 )
					{
						throw vsocket_exception("setsockopt(IP_MULTICAST_TTL)");
					}
#endif

//					u_char ittl = 255; // IP TTL
//					if ( ::setsockopt(_fd, IPPROTO_IP, IP_TTL, &ittl, sizeof(ittl)) < 0 )
//					{
//						throw vsocket_exception("setsockopt(IP_TTL)");
//					}
				} else if (_vaddress.getFamily() == ibrcommon::vaddress::VADDRESS_INET6) {
#ifdef HAVE_FEATURES_H
					int val = 1;
					if ( ::setsockopt(_fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
					{
						throw vsocket_exception("setsockopt(IPV6_MULTICAST_LOOP)");
					}

//					u_char ttl = 255; // Multicast TTL
//					if ( ::setsockopt(_fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0 )
//					{
//						throw vsocket_exception("setsockopt(IPV6_MULTICAST_HOPS)");
//					}
#endif

//					u_char ittl = 255; // IP TTL
//					if ( ::setsockopt(_fd, IPPROTO_IPV6, IPV6_HOPLIMIT, &ittl, sizeof(ittl)) < 0 )
//					{
//						throw vsocket_exception("setsockopt(IPV6_HOPLIMIT)");
//					}
				}
				break;
			}
		}
	}

	void vsocket::vbind::unset(const vsocket::Option &o)
	{
		switch (o)
		{
			default:
				break;

			case VSOCKET_REUSEADDR:
			{
				int on = 0;
				if (::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
				{
					throw vsocket_exception("setsockopt(SO_REUSEADDR) failed");
				}
				break;
			}

			case VSOCKET_LINGER:
			{
				int set = 0;
				::setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&set, sizeof(set));
				break;
			}

			case VSOCKET_NODELAY:
			{
				// set linger option to the socket
				struct linger linger;

				linger.l_onoff = 0;
				linger.l_linger = 0;
				::setsockopt(_fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
				break;
			}

			case VSOCKET_BROADCAST:
			{
				int b = 0;
				if ( ::setsockopt(_fd, SOL_SOCKET, SO_BROADCAST, (char*)&b, sizeof(b)) == -1 )
				{
					throw vsocket_exception("cannot disable broadcasts");
				}
				break;
			}

			case VSOCKET_NONBLOCKING:
			{
				vsocket::set_non_blocking(_fd);
				break;
			}

			case VSOCKET_MULTICAST:
			{
				if (_vaddress.getFamily() == ibrcommon::vaddress::VADDRESS_INET) {
#ifdef HAVE_FEATURES_H
					int val = 0;
					if ( ::setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
					{
						throw vsocket_exception("setsockopt(IP_MULTICAST_LOOP)");
					}
#endif
				} else if (_vaddress.getFamily() == ibrcommon::vaddress::VADDRESS_INET6) {
#ifdef HAVE_FEATURES_H
					int val = 0;
					if ( ::setsockopt(_fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
					{
						throw vsocket_exception("setsockopt(IPV6_MULTICAST_LOOP)");
					}
#endif
				}
				break;
			}
		}
	}

	void vsocket::vbind::join(const ibrcommon::vaddress &group, const ibrcommon::vinterface &iface)
	{
		if (group.getFamily() != _vaddress.getFamily()) return;

		// only join to this interface if it is up (has addresses available)
		if (!iface.up(group.getFamily())) return;

		switch (group.getFamily()) {
			case ibrcommon::vaddress::VADDRESS_INET:
			{
				struct sockaddr_in mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin_addr)) < 0 )
				{
					throw vsocket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IP, MCAST_JOIN_GROUP, &req, sizeof(req)) == -1 )
				{
					throw vsocket_exception("setsockopt(MCAST_JOIN_GROUP)");
				}
				break;
			}

			case ibrcommon::vaddress::VADDRESS_INET6:
			{
				struct sockaddr_in6 mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin6_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin6_addr)) < 0 )
				{
					throw vsocket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IPV6, MCAST_JOIN_GROUP, &req, sizeof(req)) == -1 )
				{
					throw vsocket_exception("setsockopt(MCAST_JOIN_GROUP)");
				}
				break;
			}

			default:
				return;
		}

		// successful!
		IBRCOMMON_LOGGER_DEBUG(5) << "multicast join group successful to " << group.toString() << " on " << iface.toString() << IBRCOMMON_LOGGER_ENDL;
	}

	void vsocket::vbind::leave(const ibrcommon::vaddress &group, const ibrcommon::vinterface &iface)
	{
		if (group.getFamily() != _vaddress.getFamily()) return;

		// only leave on this interface if it is up (has addresses available)
                if (!iface.up(group.getFamily())) return;

		switch (group.getFamily()) {
			case ibrcommon::vaddress::VADDRESS_INET:
			{
				struct sockaddr_in mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin_addr)) < 0 )
				{
					throw vsocket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IP, MCAST_LEAVE_GROUP, &req, sizeof(req)) == -1 )
				{
					throw vsocket_exception("setsockopt(MCAST_LEAVE_GROUP)");
				}
				break;
			}

			case ibrcommon::vaddress::VADDRESS_INET6:
			{
				struct sockaddr_in6 mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin6_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin6_addr)) < 0 )
				{
					throw vsocket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IPV6, MCAST_LEAVE_GROUP, &req, sizeof(req)) == -1 )
				{
					throw vsocket_exception("setsockopt(MCAST_LEAVE_GROUP)");
				}
				break;
			}

			default:
				return;
		}
	}

	void vsocket::vbind::close()
	{
		if (_fd == -1) return;
		::close(_fd);
		_fd = -1;
	}

	void vsocket::vbind::shutdown()
	{
		::shutdown(_fd, SHUT_RDWR);
	}

	bool vsocket::vbind::operator==(const int &fd) const
	{
		return (fd == _fd);
	}

	bool vsocket::vbind::operator==(const vbind &obj) const
	{
		if (obj._type != _type) return false;
		if (obj._port != _port) return false;
		if (obj._vaddress != _vaddress) return false;
		if (obj._file.getPath() != _file.getPath()) return false;

		return true;
	}

	void vsocket::vbind::check_socket_error(const int err) const
	{
		switch (err)
		{
		case EACCES:
			throw vsocket_exception("Permission  to create a socket of the specified type and/or protocol is denied.");

		case EAFNOSUPPORT:
			throw vsocket_exception("The implementation does not support the specified address family.");

		case EINVAL:
			throw vsocket_exception("Unknown protocol, or protocol family not available.");

		case EMFILE:
			throw vsocket_exception("Process file table overflow.");

		case ENFILE:
			throw vsocket_exception("The system limit on the total number of open files has been reached.");

		case ENOBUFS:
		case ENOMEM:
			throw vsocket_exception("Insufficient memory is available. The socket cannot be created until sufficient resources are freed.");

		case EPROTONOSUPPORT:
			throw vsocket_exception("The protocol type or the specified protocol is not supported within this domain.");

		default:
			throw vsocket_exception("cannot create socket");
		}
	}

	void vsocket::vbind::check_bind_error(const int err) const
	{
		switch ( err )
		{
		case EBADF:
			throw vsocket_exception("sockfd ist kein gueltiger Deskriptor.");

		// Die  folgenden  Fehlermeldungen  sind  spezifisch fr UNIX-Domnensockets (AF_UNIX)

		case EINVAL:
			throw vsocket_exception("Die addr_len war  falsch  oder  der  Socket  gehrte  nicht  zur AF_UNIX Familie.");

		case EROFS:
			throw vsocket_exception("Die Socket \"Inode\" sollte auf einem schreibgeschtzten Dateisystem residieren.");

		case EFAULT:
			throw vsocket_exception("my_addr  weist  auf  eine  Adresse  auerhalb  des  erreichbaren Adressraumes zu.");

		case ENAMETOOLONG:
			throw vsocket_exception("my_addr ist zu lang.");

		case ENOENT:
			throw vsocket_exception("Die Datei existiert nicht.");

		case ENOMEM:
			throw vsocket_exception("Nicht genug Kernelspeicher vorhanden.");

		case ENOTDIR:
			throw vsocket_exception("Eine Komponente des Pfad-Prfixes ist kein Verzeichnis.");

		case EACCES:
			throw vsocket_exception("Keine  berechtigung  um  eine  Komponente  des Pfad-prefixes zu durchsuchen.");

		case ELOOP:
			throw vsocket_exception("my_addr enthlt eine Kreis-Referenz (zum  Beispiel  durch  einen symbolischen Link)");

		default:
			throw vsocket_exception("cannot bind socket");
		}
	}
}
