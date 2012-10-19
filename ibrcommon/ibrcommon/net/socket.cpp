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
#include "ibrcommon/TimeMeasurement.h"

#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/tcp.h>

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

	void set_fd_non_blocking(int fd, bool nonblock)
	{
		int opts;
		opts = fcntl(fd,F_GETFL);
		if (opts < 0) {
			throw socket_exception("cannot set non-blocking");
		}

		if (nonblock)
			opts |= O_NONBLOCK;
		else
			opts &= ~(O_NONBLOCK);

		if (fcntl(fd,F_SETFL,opts) < 0) {
			throw socket_exception("cannot set non-blocking");
		}
	}

	basesocket::~basesocket()
	{ }

	clientsocket::clientsocket()
	 : _fd(-1)
	{
	}

	clientsocket::clientsocket(int fd)
	 : _fd(fd)
	{
	}

	clientsocket::~clientsocket()
	{
	}

	void clientsocket::up() throw (socket_exception)
	{
	}

	void clientsocket::down() throw (socket_exception)
	{
		::close(fd());
		_fd = 0;
	}

	int clientsocket::fd() const throw (socket_exception)
	{
		if (_fd < 0)
			throw socket_exception("fd not available");

		return _fd;
	}

	void clientsocket::enableKeepalive() const throw (socket_exception)
	{
		/* Set the option active */
		int optval = 1;
		if(setsockopt(this->fd(), SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
			throw ibrcommon::socket_exception("can not activate keepalives");
		}
	}

	void clientsocket::enableLinger(int l) const throw (socket_exception)
	{
		// set linger option to the socket
		struct linger linger;

		linger.l_onoff = 1;
		linger.l_linger = l;
		::setsockopt(this->fd(), SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
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
//		set(vsocket::VSOCKET_NONBLOCKING);
//		listen(_listen);
	}

	void fileserversocket::down() throw (socket_exception)
	{
		::close(fd());
	}

	int fileserversocket::fd() const throw (socket_exception)
	{
		throw socket_exception("fd not available");
	}

	filesocket* fileserversocket::accept() throw (socket_exception)
	{
		int fd = this->fd();

		struct sockaddr_storage cliaddr;
		socklen_t len = sizeof(cliaddr);

		int new_fd = ::accept(fd, (struct sockaddr *) &cliaddr, &len);

		if (new_fd <= 0) {
			throw socket_exception("accept failed");
		}

		return new filesocket(new_fd);
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
//		set(vsocket::VSOCKET_REUSEADDR);
//		set(vsocket::VSOCKET_LINGER);
//		set(vsocket::VSOCKET_NONBLOCKING);
//		bind(_address, _port);
//		listen(_listen);
	}

	void tcpserversocket::down() throw (socket_exception)
	{
		::close(fd());
	}

	int tcpserversocket::fd() const throw (socket_exception)
	{
		throw socket_exception("fd not available");
	}

	tcpsocket* tcpserversocket::accept() throw (socket_exception)
	{
		int fd = this->fd();

		struct sockaddr_storage cliaddr;
		socklen_t len = sizeof(cliaddr);

		int new_fd = ::accept(fd, (struct sockaddr *) &cliaddr, &len);

		if (new_fd <= 0) {
			throw socket_exception("accept failed");
		}

		return new tcpsocket(new_fd);
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
//		struct addrinfo hints;
//		struct addrinfo *walk;
//		memset(&hints, 0, sizeof(struct addrinfo));
//		hints.ai_family = PF_UNSPEC;
//		hints.ai_socktype = SOCK_STREAM;
//
//		struct addrinfo *res;
//		int ret;
//
//		std::stringstream ss; ss << _port; std::string port_string = ss.str();
//
//		if ((ret = getaddrinfo(address.c_str(), port_string.c_str(), &hints, &res)) != 0)
//		{
//			throw SocketException("getaddrinfo(): " + std::string(gai_strerror(ret)));
//		}
//
//		if (res == NULL)
//		{
//			throw SocketException("Could not connect to the server.");
//		}
//
//		try {
//			for (walk = res; walk != NULL; walk = walk->ai_next) {
//				_socket = socket(walk->ai_family, walk->ai_socktype, walk->ai_protocol);
//				if (_socket < 0){
//					/* Hier kann eine Fehlermeldung hin, z.B. mit warn() */
//
//					if (walk->ai_next ==  NULL)
//					{
//						throw SocketException("Could not create a socket.");
//					}
//					continue;
//				}
//
//				if (timeout == 0)
//				{
//					if (connect(_socket, walk->ai_addr, walk->ai_addrlen) != 0) {
//						::close(_socket);
//						_socket = -1;
//						/* Hier kann eine Fehlermeldung hin, z.B. mit warn() */
//						if (walk->ai_next ==  NULL)
//						{
//							throw SocketException("Could not connect to the server.");
//						}
//						continue;
//					}
//				}
//				else
//				{
//					// timeout is requested, set socket to non-blocking
//					ibrcommon::set_fd_non_blocking(_socket);
//
//					// now connect to the host (this returns immediately
//					::connect(_socket, walk->ai_addr, walk->ai_addrlen);
//
//					try {
//						bool read = false;
//						bool write = true;
//						bool error = false;
//
//						// now wait until we could write on this socket
//						tcpstream::select(_interrupt_pipe_read[1], read, write, error, timeout);
//
//						// set the socket to blocking again
//						ibrcommon::set_fd_non_blocking(_socket, false);
//
//						// check if the attempt was successful
//						int err = 0;
//						socklen_t err_len = sizeof(err_len);
//						::getsockopt(_socket, SOL_SOCKET, SO_ERROR, &err, &err_len);
//
//						if (err != 0)
//						{
//							throw SocketException("Could not connect to the server.");
//						}
//					} catch (const select_exception &ex) {
//						throw SocketException("Could not connect to the server.");
//					}
//				}
//				break;
//			}
//
//			freeaddrinfo(res);
//		} catch (ibrcommon::Exception&) {
//			freeaddrinfo(res);
//			throw;
//		}
	}

	void tcpsocket::down() throw (socket_exception)
	{
		::close(fd());
		this->fd(0);
	}


	void tcpsocket::enableNoDelay() const throw (socket_exception)
	{
		int set = 1;
		::setsockopt(this->fd(), IPPROTO_TCP, TCP_NODELAY, (char *)&set, sizeof(set));
	}

	filesocket::filesocket(int fd)
	 : clientsocket(fd)
	{
	}

	filesocket::filesocket(const ibrcommon::File &file)
	 : _filename(file)
	{
	}

	filesocket::~filesocket()
	{
	}

	void filesocket::up() throw (socket_exception)
	{
//		int len = 0;
//		struct sockaddr_un saun;
//
//		/*
//		 * Get a socket to work with.  This socket will
//		 * be in the UNIX domain, and will be a
//		 * stream socket.
//		 */
//		if ((_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
//			throw SocketException("Could not create a socket.");
//		}
//
//		/*
//		 * Create the address we will be connecting to.
//		 */
//		saun.sun_family = AF_UNIX;
//		strcpy(saun.sun_path, s.getPath().c_str());
//
//		/*
//		 * Try to connect to the address.  For this to
//		 * succeed, the server must already have bound
//		 * this address, and must have issued a listen()
//		 * request.
//		 *
//		 * The third argument indicates the "length" of
//		 * the structure, not just the length of the
//		 * socket name.
//		 */
//		len = sizeof(saun.sun_family) + strlen(saun.sun_path);
//
//		if (connect(_socket, (struct sockaddr *)&saun, len) < 0) {
//			throw SocketException("Could not connect to the named socket.");
//		}
	}

	void filesocket::down() throw (socket_exception)
	{
		::close(fd());
		this->fd(0);
	}
}
