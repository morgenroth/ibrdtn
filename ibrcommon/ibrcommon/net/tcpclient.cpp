/*
 * tcpclient.cpp
 *
 *  Created on: 29.07.2009
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/net/tcpclient.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <streambuf>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

namespace ibrcommon
{
	tcpclient::tcpclient()
	{
	}

	tcpclient::tcpclient(const ibrcommon::File &s)
	{
		open(s);
	}

	void tcpclient::open(const ibrcommon::File &s)
	{
		int len = 0;
		struct sockaddr_un saun;

		/*
		 * Get a socket to work with.  This socket will
		 * be in the UNIX domain, and will be a
		 * stream socket.
		 */
		if ((_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			throw SocketException("Could not create a socket.");
		}

		/*
		 * Create the address we will be connecting to.
		 */
		saun.sun_family = AF_UNIX;
		strcpy(saun.sun_path, s.getPath().c_str());

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

		if (connect(_socket, (struct sockaddr *)&saun, len) < 0) {
			throw SocketException("Could not connect to the named socket.");
		}
	}

	tcpclient::tcpclient(const string &address, const int port, size_t timeout)
	{
		open(address, port, timeout);
	}

	void tcpclient::open(const string &address, const int port, size_t timeout)
	{
		struct addrinfo hints;
		struct addrinfo *walk;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		struct addrinfo *res;
		int ret;

		std::stringstream ss; ss << port; std::string port_string = ss.str();

		if ((ret = getaddrinfo(address.c_str(), port_string.c_str(), &hints, &res)) != 0)
		{
			throw SocketException("getaddrinfo(): " + std::string(gai_strerror(ret)));
		}

		if (res == NULL)
		{
			throw SocketException("Could not connect to the server.");
		}

		try {
			for (walk = res; walk != NULL; walk = walk->ai_next) {
				_socket = socket(walk->ai_family, walk->ai_socktype, walk->ai_protocol);
				if (_socket < 0){
					/* Hier kann eine Fehlermeldung hin, z.B. mit warn() */

					if (walk->ai_next ==  NULL)
					{
						throw SocketException("Could not create a socket.");
					}
					continue;
				}

				if (timeout == 0)
				{
					if (connect(_socket, walk->ai_addr, walk->ai_addrlen) != 0) {
						::close(_socket);
						_socket = -1;
						/* Hier kann eine Fehlermeldung hin, z.B. mit warn() */
						if (walk->ai_next ==  NULL)
						{
							throw SocketException("Could not connect to the server.");
						}
						continue;
					}
				}
				else
				{
					// timeout is requested, set socket to non-blocking
					vsocket::set_non_blocking(_socket);

					// now connect to the host (this returns immediately
					::connect(_socket, walk->ai_addr, walk->ai_addrlen);

					try {
						bool read = false;
						bool write = true;
						bool error = false;

						// now wait until we could write on this socket
						tcpstream::select(_interrupt_pipe_read[1], read, write, error, timeout);

						// set the socket to blocking again
						vsocket::set_non_blocking(_socket, false);

						// check if the attempt was successful
						int err = 0;
						socklen_t err_len = sizeof(err_len);
						::getsockopt(_socket, SOL_SOCKET, SO_ERROR, &err, &err_len);

						if (err != 0)
						{
							throw SocketException("Could not connect to the server.");
						}
					} catch (const select_exception &ex) {
						throw SocketException("Could not connect to the server.");
					}
				}
				break;
			}

			freeaddrinfo(res);
		} catch (ibrcommon::Exception&) {
			freeaddrinfo(res);
			throw;
		}
	}

	tcpclient::~tcpclient()
	{
	}
}
