/*
 * tcpstream.cpp
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
#include "ibrcommon/Logger.h"
#include "ibrcommon/net/tcpstream.h"
#include "ibrcommon/thread/MutexLock.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace ibrcommon
{
	tcpstream::tcpstream(int socket) :
		std::iostream(this), errmsg(ERROR_NONE), _socket(socket), in_buf_(new char[BUFF_SIZE]), out_buf_(new char[BUFF_SIZE]), _nonblocking(false), _timeout(0)
	{
		// create a pipe for interruption
		if (pipe(_interrupt_pipe_read) < 0)
		{
			IBRCOMMON_LOGGER(error) << "Error " << errno << " creating pipe" << IBRCOMMON_LOGGER_ENDL;
			throw ibrcommon::Exception("failed to create pipe");
		}

		// create a pipe for interruption
		if (pipe(_interrupt_pipe_write) < 0)
		{
			::close(_interrupt_pipe_read[0]);
			::close(_interrupt_pipe_read[1]);
			IBRCOMMON_LOGGER(error) << "Error " << errno << " creating pipe" << IBRCOMMON_LOGGER_ENDL;
			throw ibrcommon::Exception("failed to create pipe");
		}

		// prevent SIGPIPE from being thrown
		int set = 1;

#ifdef HAVE_FEATURES_H
		::setsockopt(_socket, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int));
#else
		::setsockopt(_socket, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

		// set the pipe to non-blocking
		vsocket::set_non_blocking(_interrupt_pipe_read[0]);
		vsocket::set_non_blocking(_interrupt_pipe_read[1]);
		vsocket::set_non_blocking(_interrupt_pipe_write[0]);
		vsocket::set_non_blocking(_interrupt_pipe_write[1]);

		// Initialize get pointer.  This should be zero so that underflow is called upon first read.
		setg(0, 0, 0);
		setp(out_buf_, out_buf_ + BUFF_SIZE - 1);
	}

	tcpstream::~tcpstream()
	{
		delete[] in_buf_;
		delete[] out_buf_;

		// finally, close the socket
		close();

		// close all used pipes
		::close(_interrupt_pipe_read[0]);
		::close(_interrupt_pipe_read[1]);
		::close(_interrupt_pipe_write[0]);
		::close(_interrupt_pipe_write[1]);
	}

	string tcpstream::getAddress() const
	{
		struct ::sockaddr_in sa;
		int iLen = sizeof(sa);

		getpeername(_socket, (sockaddr*) &sa, (socklen_t*) &iLen);
		return inet_ntoa(sa.sin_addr);
	}

	int tcpstream::getPort() const
	{
		struct ::sockaddr_in sa;
		int iLen = sizeof(sa);

		getpeername(_socket, (sockaddr*) &sa, (socklen_t*) &iLen);
		return ntohs(sa.sin_port);
	}

	void tcpstream::interrupt()
	{
		::write(_interrupt_pipe_read[1], "i", 1);
		::write(_interrupt_pipe_write[1], "i", 1);
	}

	void tcpstream::close(bool errorcheck)
	{
		static ibrcommon::Mutex close_lock;
		ibrcommon::MutexLock l(close_lock);

		if (_socket == -1)
		{
			if (errorcheck) throw ConnectionClosedException();
			return;
		}

		int sock = _socket;
		_socket = -1;

		// unblock all socket operations
		interrupt();

		if ((::close(sock) == -1) && errorcheck)
		{
			throw ConnectionClosedException();
		}
	}

	int tcpstream::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(
				std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
				: 0;

		return ret;
	}

	int tcpstream::select(int int_pipe, bool &read, bool &write, bool &error, int timeout) throw (select_exception)
	{
		// return if the stream was closed
		if (_socket == -1)
		{
			throw select_exception(select_exception::SELECT_CLOSED);
		}

		if (!_nonblocking)
		{
			// set the tcp socket to non-blocking
			vsocket::set_non_blocking(_socket);
			
			// do not set this mode again
			_nonblocking = true;
		}

		int high_fd = _socket;

		fd_set fds_read, fds_write, fds_error;
		fd_set *fdsp_write = NULL, *fdsp_error = NULL;

		FD_ZERO(&fds_read);
		FD_ZERO(&fds_write);
		FD_ZERO(&fds_error);

		if (read)
		{
			FD_SET(_socket, &fds_read);
		}

		if (write)
		{
			FD_SET(_socket, &fds_write);
			fdsp_write = &fds_write;
		}

		if (error)
		{
			FD_SET(_socket, &fds_error);
			fdsp_error = &fds_error;
		}

		read = false;
		write = false;
		error = false;

		// socket for the self-pipe trick
		FD_SET(int_pipe, &fds_read);
		if (high_fd < int_pipe) high_fd = int_pipe;

		int res = 0;

		// set timeout
		struct timeval tv;
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		bool _continue = true;
		while (_continue)
		{
			if (timeout > 0)
			{
#ifdef HAVE_FEATURES_H
				res = ::select(high_fd + 1, &fds_read, fdsp_write, fdsp_error, &tv);
#else
				res = __nonlinux_select(high_fd + 1, &fds_read, fdsp_write, fdsp_error, &tv);
#endif
			}
			else
			{
				res = ::select(high_fd + 1, &fds_read, fdsp_write, fdsp_error, NULL);
			}

			// check for select error
			if (res < 0)
			{
				throw select_exception(select_exception::SELECT_ERROR);
			}

			// check for timeout
			if (res == 0)
			{
				throw select_exception(select_exception::SELECT_TIMEOUT);
			}

			if (FD_ISSET(int_pipe, &fds_read))
			{
				IBRCOMMON_LOGGER_DEBUG(25) << "unblocked by self-pipe-trick" << IBRCOMMON_LOGGER_ENDL;

				// this was an interrupt with the self-pipe-trick
				char buf[2];
				::read(int_pipe, buf, 2);
			}

			// return if the stream was closed
			if (_socket == -1)
			{
				throw select_exception(select_exception::SELECT_CLOSED);
			}

			if (FD_ISSET(_socket, &fds_read))
			{
				read = true;
				_continue = false;
			}

			if (FD_ISSET(_socket, &fds_write))
			{
				write = true;
				_continue = false;
			}

			if (FD_ISSET(_socket, &fds_error))
			{
				error = true;
				_continue = false;
			}
		}

		return res;
	}

	std::char_traits<char>::int_type tcpstream::overflow(std::char_traits<char>::int_type c)
	{
		char *ibegin = out_buf_;
		char *iend = pptr();

		// mark the buffer as free
		setp(out_buf_, out_buf_ + BUFF_SIZE - 1);

		if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
		{
			*iend++ = std::char_traits<char>::to_char_type(c);
		}

		// if there is nothing to send, just return
		if ((iend - ibegin) == 0)
		{
			IBRCOMMON_LOGGER_DEBUG(90) << "tcpstream::overflow() nothing to sent" << IBRCOMMON_LOGGER_ENDL;
			return std::char_traits<char>::not_eof(c);
		}

		try {
			bool read = false, write = true, error = false;
			select(_interrupt_pipe_write[0], read, write, error, _timeout);

			// bytes to send
			size_t bytes = (iend - ibegin);

			// send the data
			ssize_t ret = ::send(_socket, out_buf_, (iend - ibegin), 0);

			if (ret < 0)
			{
				switch (errno)
				{
				case EPIPE:
					// connection has been reset
					errmsg = ERROR_EPIPE;
					break;

				case ECONNRESET:
					// Connection reset by peer
					errmsg = ERROR_RESET;
					break;

				case EAGAIN:
					// sent failed but we should retry again
					return overflow(c);

				default:
					errmsg = ERROR_WRITE;
					break;
				}

				// failure
				close();
				std::stringstream ss; ss << "<tcpstream> send() in tcpstream failed: " << errno;
				throw ConnectionClosedException(ss.str());
			}
			else
			{
				// check how many bytes are sent
				if ((size_t)ret < bytes)
				{
					// we did not sent all bytes
					char *resched_begin = ibegin + ret;
					char *resched_end = iend;

					// bytes left to send
					size_t bytes_left = resched_end - resched_begin;

					// move the data to the begin of the buffer
					::memcpy(ibegin, resched_begin, bytes_left);

					// new free buffer
					char *buffer_begin = ibegin + bytes_left;

					// mark the buffer as free
					setp(buffer_begin, out_buf_ + BUFF_SIZE - 1);
				}
			}
		} catch (const select_exception &ex) {
			// send timeout
			errmsg = ERROR_WRITE;
			close();
			throw ConnectionClosedException("<tcpstream> send() timed out");
		}

		return std::char_traits<char>::not_eof(c);
	}

	std::char_traits<char>::int_type tcpstream::underflow()
	{
		try {
			bool read = true;
			bool write = false;
			bool error = false;

			select(_interrupt_pipe_read[0], read, write, error, _timeout);

			// read some bytes
			int bytes = ::recv(_socket, in_buf_, BUFF_SIZE, 0);

			// end of stream
			if (bytes == 0)
			{
				errmsg = ERROR_CLOSED;
				close();
				IBRCOMMON_LOGGER_DEBUG(40) << "<tcpstream> recv() returned zero: " << errno << IBRCOMMON_LOGGER_ENDL;
				return std::char_traits<char>::eof();
			}
			else if (bytes < 0)
			{
				switch (errno)
				{
				case EPIPE:
					// connection has been reset
					errmsg = ERROR_EPIPE;
					break;

				default:
					errmsg = ERROR_READ;
					break;
				}

				close();
				IBRCOMMON_LOGGER_DEBUG(40) << "<tcpstream> recv() failed: " << errno << IBRCOMMON_LOGGER_ENDL;
				return std::char_traits<char>::eof();
			}

			// Since the input buffer content is now valid (or is new)
			// the get pointer should be initialized (or reset).
			setg(in_buf_, in_buf_, in_buf_ + bytes);

			return std::char_traits<char>::not_eof((unsigned char) in_buf_[0]);
		} catch (const select_exception &ex) {
			return std::char_traits<char>::eof();
		}
	}

	void tcpstream::setTimeout(unsigned int value)
	{
		_timeout = value;
	}

	void tcpstream::enableKeepalive()
	{
		/* Set the option active */
		int optval = 1;
		if(setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) < 0) {
			throw ibrcommon::vsocket_exception("<tcpstream> can not activate keepalives");
		}
	}

	void tcpstream::enableLinger(int l)
	{
		// set linger option to the socket
		struct linger linger;

		linger.l_onoff = 1;
		linger.l_linger = l;
		::setsockopt(_socket, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
	}

	void tcpstream::enableNoDelay()
	{
		int set = 1;
		::setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&set, sizeof(set));
	}

}
