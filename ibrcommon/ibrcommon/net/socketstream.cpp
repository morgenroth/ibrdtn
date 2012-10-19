/*
 * socketstream.cpp
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

#include "ibrcommon/net/socketstream.h"
#include "ibrcommon/Logger.h"
#include <string.h>

namespace ibrcommon
{
	socketstream::socketstream(clientsocket *sock, size_t buffer_size)
	 : std::iostream(this), errmsg(ERROR_NONE), _bufsize(buffer_size), in_buf_(NULL), out_buf_(NULL)
	{
		in_buf_ = new char[_bufsize];
		out_buf_ = new char[_bufsize];

		// mark the buffer as free
		setp(out_buf_, out_buf_ + _bufsize - 1);
		setg(0, 0, 0);

		_socket.add(sock);
		_socket.up();
	}

	socketstream::~socketstream()
	{
		_socket.destroy();

		delete in_buf_;
		delete out_buf_;
	}

	void socketstream::close()
	{
		_socket.down();
	}

	void socketstream::setTimeout(const timeval &val)
	{
		::memcpy(&_timeout, &val, sizeof _timeout);
	}

	int socketstream::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(
				std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
				: 0;

		return ret;
	}

	std::char_traits<char>::int_type socketstream::overflow(std::char_traits<char>::int_type c)
	{
		char *ibegin = out_buf_;
		char *iend = pptr();

		// mark the buffer as free
		setp(out_buf_, out_buf_ + _bufsize - 1);

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
			socketset writeset;
			_socket.select(NULL, &writeset, NULL, NULL);

			// error checking
			if (writeset.size() == 0) {
				throw socket_exception("no select result returned");
			}

			// bytes to send
			size_t bytes = (iend - ibegin);

			// send the data
			clientsocket &sock = static_cast<clientsocket&>(**(writeset.begin()));

			int ret = sock.send(out_buf_, (iend - ibegin), 0);

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
				setp(buffer_begin, out_buf_ + _bufsize - 1);
			}
		} catch (const socket_error &err) {
			if (err.code() == ERROR_AGAIN) {
				return overflow(c);
			}

			// set the last error code
			errmsg = err.code();

			// close the stream/socket due to failures
			close();

			// create a detailed exception message
			std::stringstream ss; ss << "<tcpstream> send() in tcpstream failed: " << err.code();
			throw stream_exception(ss.str());
		} catch (const socket_exception &ex) {
			// set the last error code
			errmsg = ERROR_WRITE;

			// close the stream/socket due to failures
			close();

			// create a detailed exception message
			throw stream_exception("<tcpstream> send() timed out");
		}

		return std::char_traits<char>::not_eof(c);
	}

	std::char_traits<char>::int_type socketstream::underflow()
	{
		try {
			socketset readset;

			if (timerisset(&_timeout)) {
				timeval to_copy;
				::memcpy(&to_copy, &_timeout, sizeof to_copy);
				_socket.select(&readset, NULL, NULL, &to_copy);
			} else {
				_socket.select(&readset, NULL, NULL, NULL);
			}

			// error checking
			if (readset.size() == 0) {
				throw socket_exception("no select result returned");
			}

			// send the data
			clientsocket &sock = static_cast<clientsocket&>(**(readset.begin()));

			// read some bytes
			int bytes = sock.recv(in_buf_, _bufsize, 0);

			// end of stream
			if (bytes == 0)
			{
				errmsg = ERROR_CLOSED;
				close();
				IBRCOMMON_LOGGER_DEBUG(40) << "<tcpstream> recv() returned zero: " << errno << IBRCOMMON_LOGGER_ENDL;
				return std::char_traits<char>::eof();
			}

			// Since the input buffer content is now valid (or is new)
			// the get pointer should be initialized (or reset).
			setg(in_buf_, in_buf_, in_buf_ + bytes);

			return std::char_traits<char>::not_eof((unsigned char) in_buf_[0]);
		} catch (const socket_error &err) {
			// set the last error code
			errmsg = err.code();

			// close the stream/socket due to failures
			close();

			IBRCOMMON_LOGGER_DEBUG(40) << "<tcpstream> recv() failed: " << err.code() << IBRCOMMON_LOGGER_ENDL;
		} catch (const socket_exception &ex) {
			IBRCOMMON_LOGGER_DEBUG(40) << "<tcpstream> recv() failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
		}

		return std::char_traits<char>::eof();
	}
} /* namespace ibrcommon */
