/*
 * socket.cpp
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
#include "ibrcommon/net/socket.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/TimeMeasurement.h"
#include "ibrcommon/Logger.h"

#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/socket.h>

#include <sstream>

#ifndef HAVE_BZERO
#define bzero(s,n) (memset((s), '\0', (n)), (void) 0)
#endif

namespace ibrcommon
{
	int __linux_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
	{
#ifdef HAVE_FEATURES_H
		// on linux platform we can use the native select method
		return ::select(nfds, readfds, writefds, exceptfds, timeout);
#endif

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

	basesocket::basesocket(int fd)
	 : _state(SOCKET_DOWN), _fd(fd)
	{
		if (_fd <= 0) {
			_state = SOCKET_UNMANAGED;
		}
	}

	basesocket::~basesocket()
	{ }

	int basesocket::fd() const throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("fd not available");
		return _fd;
	}

	void basesocket::close() throw (socket_exception)
	{
		int ret = ::close(this->fd());
		if (ret == -1)
			throw socket_exception("close error");
		_state = SOCKET_DOWN;
	}

	void basesocket::shutdown(int how) throw (socket_exception)
	{
		int ret = ::shutdown(this->fd(), how);
		if (ret == -1)
			throw socket_exception("shutdown error");
		_state = SOCKET_DOWN;
	}

//	void basesocket::bind(const ibrcommon::vaddress &addr) throw (socket_exception)
//	{
//		int ret = ::bind(this->fd(), sock, len);
//		check_socket_error(ret);
//	}

	void basesocket::set_blocking_mode(bool val, int fd) const throw (socket_exception)
	{
		int opts;
		opts = fcntl((fd == -1) ? this->fd() : fd, F_GETFL);
		if (opts < 0) {
			throw socket_exception("cannot set non-blocking");
		}

		if (val)
			opts &= ~(O_NONBLOCK);
		else
			opts |= O_NONBLOCK;

		if (fcntl((fd == -1) ? this->fd() : fd, F_SETFL, opts) < 0) {
			throw socket_exception("cannot set non-blocking");
		}
	}

	void basesocket::set_keepalive(bool val, int fd) const throw (socket_exception)
	{
		/* Set the option active */
		int optval = (val ? 1 : 0);
		if (::setsockopt((fd == -1) ? this->fd() : fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
			throw ibrcommon::socket_exception("can not activate keepalives");
		}
	}

	void basesocket::set_linger(bool val, int l, int fd) const throw (socket_exception)
	{
		// set linger option to the socket
		struct linger linger;

		linger.l_onoff = (val ? 1 : 0);
		linger.l_linger = l;
		if (::setsockopt((fd == -1) ? this->fd() : fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) < 0) {
			throw ibrcommon::socket_exception("can not set linger option");
		}
	}

	void basesocket::set_reuseaddr(bool val, int fd) const throw (socket_exception)
	{
		int on = (val ? 1: 0);
		if (::setsockopt((fd == -1) ? this->fd() : fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		{
			throw socket_exception("setsockopt(SO_REUSEADDR) failed");
		}
	}

	void basesocket::set_nodelay(bool val, int fd) const throw (socket_exception)
	{
		int set = (val ? 1 : 0);
		if (::setsockopt((fd == -1) ? this->fd() : fd, IPPROTO_TCP, TCP_NODELAY, (char *)&set, sizeof(set)) < 0) {
			throw socket_exception("set no delay option failed");
		}
	}

	sa_family_t basesocket::get_family() const throw (socket_exception)
	{
		struct sockaddr_storage bound_addr;
		socklen_t bound_len = sizeof(bound_addr);

		// get the socket family
		int ret = ::getsockname(this->fd(), (struct sockaddr*)&bound_addr, &bound_len);
		if (ret == -1) {
			throw socket_exception("socket is not bound");
		}

		return bound_addr.ss_family;
	}

	clientsocket::clientsocket(int fd)
	 : basesocket(fd)
	{
	}

	clientsocket::~clientsocket()
	{
	}

	void clientsocket::up() throw (socket_exception)
	{
		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		_state = SOCKET_UP;
	}

	void clientsocket::down() throw (socket_exception)
	{
		if (_state != SOCKET_UP)
			throw socket_exception("socket is not up");

		this->close();
		_state = SOCKET_DOWN;
	}

	void clientsocket::send(const char *data, size_t len, int flags) throw (socket_exception)
	{
		int ret = ::send(this->fd(), data, len, flags);
		if (ret == -1)
			throw socket_exception("send error");
	}

	void clientsocket::recv(char *data, size_t len, int flags) throw (socket_exception)
	{
		int ret = ::recv(this->fd(), data, len, flags);
		if (ret == -1)
			throw socket_exception("recv error");
	}

	serversocket::serversocket(int fd)
	 : basesocket(fd)
	{
	}

	serversocket::~serversocket()
	{
	}

	void serversocket::listen(int connections) throw (socket_exception)
	{
		int ret = ::listen(this->fd(), connections);
		if (ret == -1)
			throw socket_exception("listen failed");
	}

	int serversocket::_accept_fd(ibrcommon::vaddress &addr) throw (socket_exception)
	{
		int fd = this->fd();

		struct sockaddr_storage cliaddr;
		socklen_t len = sizeof(cliaddr);

		int new_fd = ::accept(fd, (struct sockaddr *) &cliaddr, &len);

		if (new_fd <= 0) {
			throw socket_exception("accept failed");
		}

		// TODO: set source to addr

		return new_fd;
	}

	datagramsocket::datagramsocket(int fd)
	 : basesocket(fd)
	{
	}

	datagramsocket::~datagramsocket()
	{
	}

	void datagramsocket::recvfrom(char *buf, size_t buflen, int flags, ibrcommon::vaddress &addr) throw (socket_exception)
	{
		//int ret = ::recvfrom(this->fd(),  __buf, size_t __n, int __flags, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);
		//if (ret == -1)
		//	throw socket_exception("recvfrom error");
	}

	void datagramsocket::sendto(const char *buf, size_t buflen, int flags, const ibrcommon::vaddress &addr) throw (socket_exception)
	{
		//int ret = ::sendto(this->fd(), __const void *__buf, size_t __n, int __flags, __CONST_SOCKADDR_ARG __addr, socklen_t __addr_len);
		//if (ret == -1)
		//	throw socket_exception("sendto error");
	}

	filesocket::filesocket(int fd)
	 : clientsocket(fd)
	{
		_state = SOCKET_UNMANAGED;
	}

	filesocket::filesocket(const ibrcommon::File &file)
	 : _filename(file)
	{
		_state = SOCKET_DOWN;
	}

	filesocket::~filesocket()
	{
	}

	void filesocket::up() throw (socket_exception)
	{
		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		int len = 0;
		struct sockaddr_un saun;

		/*
		 * Get a socket to work with.  This socket will
		 * be in the UNIX domain, and will be a
		 * stream socket.
		 */
		if ((_fd = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			::close(_fd);
			throw socket_exception("Could not create a socket.");
		}

		/*
		 * Create the address we will be connecting to.
		 */
		saun.sun_family = AF_UNIX;
		::strcpy(saun.sun_path, _filename.getPath().c_str());

		/*
		 * Try to connect to the address.  For this to
		 * succeed, the server must already have bound
		 * this address, and must have issued a listen()
		 * request.
		 *
		 * The third argument indicates the "length" of
		 * the structure, not just the length of the
		 * socket name.
		 */
		len = sizeof(saun.sun_family) + strlen(saun.sun_path);

		if (::connect(this->fd(), (struct sockaddr *)&saun, len) < 0) {
			this->close();
			throw socket_exception("Could not connect to the named socket.");
		}

		_state = SOCKET_UP;
	}

	void filesocket::down() throw (socket_exception)
	{
		if (_state != SOCKET_UP)
			throw socket_exception("socket is not up");

		this->close();
	}

	fileserversocket::fileserversocket(const ibrcommon::File &file, int listen)
	 : _filename(file), _listen(listen)
	{
	}

	fileserversocket::~fileserversocket()
	{

	}

	void fileserversocket::up() throw (socket_exception)
	{
		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		this->set_blocking_mode(false);
		this->listen(_listen);
		_state = SOCKET_UP;
	}

	void fileserversocket::down() throw (socket_exception)
	{
		if (_state != SOCKET_UP)
			throw socket_exception("socket is not up");

		this->close();
		_state = SOCKET_DOWN;
	}

	clientsocket* fileserversocket::accept(ibrcommon::vaddress &addr) throw (socket_exception)
	{
		return new filesocket(_accept_fd(addr));
	}

	tcpserversocket::tcpserversocket(const int port, int listen)
	 : _address(), _port(port), _listen(listen)
	{

	}

	tcpserversocket::tcpserversocket(const ibrcommon::vaddress &address, const int port, int listen)
	 : _address(address), _port(port), _listen(listen)
	{
	}

	tcpserversocket::~tcpserversocket()
	{
	}

	void tcpserversocket::up() throw (socket_exception)
	{
		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		_fd = ::socket(_address.getFamily(), SOCK_STREAM, 0);

		this->set_reuseaddr(true);
		this->set_blocking_mode(false);
		this->set_linger(true);

//		this->bind(_address, _port);
		this->listen(_listen);

		_state = SOCKET_UP;
	}

	void tcpserversocket::down() throw (socket_exception)
	{
		if (_state != SOCKET_UP)
			throw socket_exception("socket is not up");
		this->close();

		_state = SOCKET_DOWN;
	}

	clientsocket* tcpserversocket::accept(ibrcommon::vaddress &addr) throw (socket_exception)
	{
		return new tcpsocket(_accept_fd(addr));
	}

	tcpsocket::tcpsocket(int fd)
	 : clientsocket(fd), _port(0), _timeout(0)
	{
	}

	tcpsocket::tcpsocket(const ibrcommon::vaddress &destination, const int port, int timeout)
	 : _address(destination), _port(port), _timeout(timeout)
	{
	}

	tcpsocket::~tcpsocket()
	{
	}

	void tcpsocket::up() throw (socket_exception)
	{
		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		struct addrinfo hints;
		struct addrinfo *walk;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		struct addrinfo *res;
		int ret;

		std::stringstream ss; ss << _port; std::string port_string = ss.str();

		if ((ret = ::getaddrinfo(_address.get().c_str(), port_string.c_str(), &hints, &res)) != 0)
		{
			throw socket_exception("getaddrinfo(): " + std::string(gai_strerror(ret)));
		}

		if (res == NULL)
		{
			throw socket_exception("Could not connect to the server.");
		}

		// create a vsocket for concurrent connection setup
		ibrcommon::vsocket probesocket;

		try {
			// walk through all the returned addresses and try to connect to all of them
			for (walk = res; walk != NULL; walk = walk->ai_next) {
				// mark the socket as invalid first
				int fd = -1;

				// create a matching socket
				fd = ::socket(walk->ai_family, walk->ai_socktype, walk->ai_protocol);

				// if the socket is invalid we proceed with the next address
				if (fd < 0) {
					/* Hier kann eine Fehlermeldung hin, z.B. mit warn() */

					if (walk->ai_next ==  NULL)
					{
						throw socket_exception("Could not create a socket.");
					}
					continue;
				}

				// set the socket to non-blocking
				this->set_blocking_mode(false);

				// connect to the current address using the created socket
				if (::connect(fd, walk->ai_addr, walk->ai_addrlen) != 0) {
					// the connect failed, so we close the socket immediately
					::close(fd);

					/* Hier kann eine Fehlermeldung hin, z.B. mit warn() */
					if (walk->ai_next ==  NULL)
					{
						throw socket_exception("Could not connect to the server.");
					}
					continue;
				}

				// add the current socket to the probe-socket for later select-call
				probesocket.add( new tcpsocket(fd) );
			}

			// TODO:
			//probesocket.select();

			// remember the fasted socket
			//_fd = fd;

			// close all other sockets

			// free the address
			freeaddrinfo(res);

			// set the current state to UP
			_state = SOCKET_UP;
		} catch (ibrcommon::Exception&) {
			freeaddrinfo(res);
			throw;
		}
	}

	void tcpsocket::down() throw (socket_exception)
	{
		if (_state != SOCKET_UP)
			throw socket_exception("socket is not up");

		this->close();
		_state = SOCKET_DOWN;
	}

	multicastsocket::multicastsocket(const int port)
	 : udpsocket(port)
	{
	}

	multicastsocket::multicastsocket(const vaddress &address, const int port)
	 : udpsocket(address, port)
	{
	}

	multicastsocket::~multicastsocket()
	{
	}

	void multicastsocket::up() throw (socket_exception)
	{
		udpsocket::up();

		try {
			switch (get_family()) {
			case AF_INET: {
#ifdef HAVE_FEATURES_H
				int val = 1;
				if ( ::setsockopt(this->fd(), IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
				{
					throw socket_exception("setsockopt(IP_MULTICAST_LOOP)");
				}

				u_char ttl = 255; // Multicast TTL
				if ( ::setsockopt(this->fd(), IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0 )
				{
					throw socket_exception("setsockopt(IP_MULTICAST_TTL)");
				}
#endif
//				u_char ittl = 255; // IP TTL
//				if ( ::setsockopt(this->fd(), IPPROTO_IP, IP_TTL, &ittl, sizeof(ittl)) < 0 )
//				{
//					throw socket_exception("setsockopt(IP_TTL)");
//				}
				break;
			}

			case AF_INET6: {
#ifdef HAVE_FEATURES_H
				int val = 1;
				if ( ::setsockopt(this->fd(), IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
				{
					throw socket_exception("setsockopt(IPV6_MULTICAST_LOOP)");
				}

//				u_char ttl = 255; // Multicast TTL
//				if ( ::setsockopt(this->fd(), IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl)) < 0 )
//				{
//					throw socket_exception("setsockopt(IPV6_MULTICAST_HOPS)");
//				}
#endif

//				u_char ittl = 255; // IP TTL
//				if ( ::setsockopt(this->fd(), IPPROTO_IPV6, IPV6_HOPLIMIT, &ittl, sizeof(ittl)) < 0 )
//				{
//					throw socket_exception("setsockopt(IPV6_HOPLIMIT)");
//				}
				break;
			}
			}
		} catch (const socket_exception&) {
			udpsocket::down();
			throw;
		}
	}

	void multicastsocket::down() throw (socket_exception)
	{
		switch (get_family()) {
		case AF_INET: {
#ifdef HAVE_FEATURES_H
			int val = 0;
			if ( ::setsockopt(this->fd(), IPPROTO_IP, IP_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
			{
				throw socket_exception("setsockopt(IP_MULTICAST_LOOP)");
			}
#endif
			break;
		}

		case AF_INET6: {
#ifdef HAVE_FEATURES_H
			int val = 0;
			if ( ::setsockopt(this->fd(), IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (const char *)&val, sizeof(val)) < 0 )
			{
				throw socket_exception("setsockopt(IPV6_MULTICAST_LOOP)");
			}
#endif
			break;
		}
		}

		udpsocket::down();
	}

	void multicastsocket::join(const vaddress &group, const vinterface &iface) throw (socket_exception)
	{
		if (get_family() != group.getFamily())
			throw socket_exception("family does not match");

		switch (group.getFamily()) {
			case AF_INET:
			{
				struct sockaddr_in mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin_addr)) < 0 )
				{
					throw socket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IP, MCAST_JOIN_GROUP, &req, sizeof(req)) == -1 )
				{
					throw socket_exception("setsockopt(MCAST_JOIN_GROUP)");
				}
				break;
			}

			case AF_INET6:
			{
				struct sockaddr_in6 mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin6_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin6_addr)) < 0 )
				{
					throw socket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IPV6, MCAST_JOIN_GROUP, &req, sizeof(req)) == -1 )
				{
					throw socket_exception("setsockopt(MCAST_JOIN_GROUP)");
				}
				break;
			}

			default:
				return;
		}

		// successful!
		IBRCOMMON_LOGGER_DEBUG(5) << "multicast join group successful to " << group.toString() << " on " << iface.toString() << IBRCOMMON_LOGGER_ENDL;
	}

	void multicastsocket::leave(const vaddress &group, const vinterface &iface) throw (socket_exception)
	{
		if (get_family() != group.getFamily())
			throw socket_exception("family does not match");

		switch (group.getFamily()) {
			case AF_INET:
			{
				struct sockaddr_in mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin_addr)) < 0 )
				{
					throw socket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IP, MCAST_LEAVE_GROUP, &req, sizeof(req)) == -1 )
				{
					throw socket_exception("setsockopt(MCAST_LEAVE_GROUP)");
				}
				break;
			}

			case AF_INET6:
			{
				struct sockaddr_in6 mcast_addr;
				::memset(&mcast_addr, 0, sizeof(mcast_addr));

				// set the family of the multicast address
				mcast_addr.sin6_family = group.getFamily();

				// convert the group address
				if ( ::inet_pton(group.getFamily(), group.get().c_str(), &(mcast_addr.sin6_addr)) < 0 )
				{
					throw socket_exception("can not transform multicast address with inet_pton()");
				}

				struct group_req req;
				::memset(&req, 0, sizeof(req));

				// copy the address to the group request
				::memcpy(&req.gr_group, &mcast_addr, sizeof(mcast_addr));

				// set the right interface here
				req.gr_interface = iface.getIndex();

				if ( ::setsockopt(_fd, IPPROTO_IPV6, MCAST_LEAVE_GROUP, &req, sizeof(req)) == -1 )
				{
					throw socket_exception("setsockopt(MCAST_LEAVE_GROUP)");
				}
				break;
			}

			default:
				return;
		}
	}

	void basesocket::check_socket_error(const int err) const throw (socket_exception)
	{
		switch (err)
		{
		case EACCES:
			throw socket_exception("Permission  to create a socket of the specified type and/or protocol is denied.");

		case EAFNOSUPPORT:
			throw socket_exception("The implementation does not support the specified address family.");

		case EINVAL:
			throw socket_exception("Unknown protocol, or protocol family not available.");

		case EMFILE:
			throw socket_exception("Process file table overflow.");

		case ENFILE:
			throw socket_exception("The system limit on the total number of open files has been reached.");

		case ENOBUFS:
		case ENOMEM:
			throw socket_exception("Insufficient memory is available. The socket cannot be created until sufficient resources are freed.");

		case EPROTONOSUPPORT:
			throw socket_exception("The protocol type or the specified protocol is not supported within this domain.");

		default:
			throw socket_exception("cannot create socket");
		}
	}

	void basesocket::check_bind_error(const int err) const throw (socket_exception)
	{
		switch ( err )
		{
		case EBADF:
			throw socket_exception("sockfd ist kein gueltiger Deskriptor.");

		// Die  folgenden  Fehlermeldungen  sind  spezifisch fr UNIX-Domnensockets (AF_UNIX)

		case EINVAL:
			throw socket_exception("Die addr_len war  falsch  oder  der  Socket  gehrte  nicht  zur AF_UNIX Familie.");

		case EROFS:
			throw socket_exception("Die Socket \"Inode\" sollte auf einem schreibgeschtzten Dateisystem residieren.");

		case EFAULT:
			throw socket_exception("my_addr  weist  auf  eine  Adresse  auerhalb  des  erreichbaren Adressraumes zu.");

		case ENAMETOOLONG:
			throw socket_exception("my_addr ist zu lang.");

		case ENOENT:
			throw socket_exception("Die Datei existiert nicht.");

		case ENOMEM:
			throw socket_exception("Nicht genug Kernelspeicher vorhanden.");

		case ENOTDIR:
			throw socket_exception("Eine Komponente des Pfad-Prfixes ist kein Verzeichnis.");

		case EACCES:
			throw socket_exception("Keine  berechtigung  um  eine  Komponente  des Pfad-prefixes zu durchsuchen.");

		case ELOOP:
			throw socket_exception("my_addr enthlt eine Kreis-Referenz (zum  Beispiel  durch  einen symbolischen Link)");

		default:
			throw socket_exception("cannot bind socket");
		}
	}
}
