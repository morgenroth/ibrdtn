/*
 * socket.h
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

#ifndef IBRCOMMON_SOCKET_H_
#define IBRCOMMON_SOCKET_H_

#include <ibrcommon/Exceptions.h>
#include <ibrcommon/data/File.h>
#include <ibrcommon/net/vaddress.h>
#include <ibrcommon/net/vinterface.h>

namespace ibrcommon {
	class socket_exception : public Exception
	{
	public:
		socket_exception(string error) : Exception(error)
		{};
	};

	/**
	 * The basesocket is an interface for all kinds of sockets. The
	 * methods allows to bring the socket up or down.
	 */
	class basesocket {
	public:
		basesocket();
		virtual ~basesocket() = 0;

		/**
		 * Create the file descriptor for this socket
		 * and bind to the interface if necessary.
		 * @throw socket_exception if the action has failed
		 */
		virtual void up() throw (socket_exception) = 0;

		/**
		 * Close and destroy the file descriptor of this
		 * socket assignment.
		 * @throw socket_exception if the action has failed
		 */
		virtual void down() throw (socket_exception) = 0;

		/**
		 * Return the file descriptor for this socket.
		 * @throw socket_exception if no file descriptor is available
		 */
		virtual int fd() const throw (socket_exception) = 0;

		/**
		 * Standard socket calls
		 */
		void close() throw (socket_exception);
		void shutdown(int how) throw (socket_exception);
		void listen(int connections) throw (socket_exception);
//		void bind() throw (socket_exception);
		void recvfrom(char *buf, size_t buflen, int flags, ibrcommon::vaddress &addr) throw (socket_exception);
		void sendto(const char *buf, size_t buflen, int flags, const ibrcommon::vaddress &addr) throw (socket_exception);

		void set_blocking_mode(bool val) const throw (socket_exception);
		void set_keepalive(bool val) const throw (socket_exception);
		void set_linger(bool val, int l = 1) const throw (socket_exception);
		void set_reuseaddr(bool val) const throw (socket_exception);
		void set_nodelay(bool val) const throw (socket_exception);

	protected:
		enum socketstate {
			SOCKET_DOWN,
			SOCKET_UP,
			SOCKET_UNMANAGED
		};

		/**
		 * Error check methods
		 */
		void check_socket_error(const int err) const throw (socket_exception);
		void check_bind_error(const int err) const throw (socket_exception);

		socketstate _state;
	};

	/**
	 * A clientsocket is used if a incoming connection has been accepted
	 * by the serversocket.
	 */
	class clientsocket : public basesocket {
	public:
		clientsocket();
		clientsocket(int fd);
		~clientsocket();
		virtual void up() throw (socket_exception);
		virtual void down() throw (socket_exception);
		int fd() const throw (socket_exception);

	protected:
		void fd(int fd);

	private:
		int _fd;
	};

	/**
	 * A file socket opens a named socket for communication.
	 */
	class filesocket : public clientsocket {
	public:
		filesocket(int fd);
		filesocket(const File &file);
		~filesocket();
		void up() throw (socket_exception);
		void down() throw (socket_exception);

	private:
		const File _filename;
	};

	/**
	 * A fileserversocket is bound to a specific socket
	 * file waiting for incoming connections.
	 */
	class fileserversocket : public basesocket {
	public:
		fileserversocket(const File &file, int listen = 0);
		~fileserversocket();
		void up() throw (socket_exception);
		void down() throw (socket_exception);
		int fd() const throw (socket_exception);

		filesocket* accept() throw (socket_exception);

	private:
		const File _filename;
		const int _listen;
	};

	/**
	 * A tcpsocket is used to connect to a TCP server.
	 */
	class tcpsocket : public clientsocket {
	public:
		tcpsocket(int fd);
		tcpsocket(const vaddress &destination, const int port, int timeout = 0);
		~tcpsocket();
		void up() throw (socket_exception);
		void down() throw (socket_exception);

		void enableNoDelay() const throw (socket_exception);

	private:
		const vaddress _address;
		const int _port;
		const int _timeout;
	};

	/**
	 * A tcpserversocket is bound to a specific port and
	 * listen for incoming connections. It binds on the ANY
	 * or a specific address.
	 */
	class tcpserversocket : public basesocket {
	public:
		tcpserversocket(const int port, int listen = 0);
		tcpserversocket(const vaddress &address, const int port, int listen = 0);
		~tcpserversocket();
		void up() throw (socket_exception);
		void down() throw (socket_exception);
		int fd() const throw (socket_exception);

		tcpsocket* accept() throw (socket_exception);

	private:
		const vaddress _address;
		const int _port;
		const int _listen;
		int _fd;
	};

	/**
	 * A udpsocket allows to send and receive UDP datagrams
	 * with a bound or non-bound file descriptor.
	 */
	class udpsocket : public basesocket {
	public:
		udpsocket(const int port = 0);
		udpsocket(const vaddress &address, const int port = 0);
		~udpsocket();
		void up() throw (socket_exception);
		void down() throw (socket_exception);
		int fd() const throw (socket_exception);

		void join(const vaddress &group, const vinterface &iface);
		void leave(const vaddress &group);

	private:
		const vaddress _address;
		const int _port;
		const int _listen;
		int _fd;
	};

	/**
	 * This select emulated the linux behavior of a select.
	 * It measures the time being in the select call and decrement the given timeout value.
	 * @param nfds
	 * @param readfds
	 * @param writefds
	 * @param exceptfds
	 * @param timeout
	 * @return
	 */
	int __linux_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

	void set_fd_non_blocking(int socket, bool nonblock = true);
}

#endif /* IBRCOMMON_SOCKET_H_ */
