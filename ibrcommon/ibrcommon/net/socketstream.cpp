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

namespace ibrcommon
{
	socketstream::socketstream(basesocket *sock, size_t buffer_size)
	 : std::iostream(this), errmsg(ERROR_NONE), _bufsize(buffer_size), in_buf_(new char[buffer_size]), out_buf_(new char[buffer_size])
	{
		_socket.add(sock);
	}

	socketstream::~socketstream()
	{
		_socket.close();
	}

	void socketstream::close()
	{
		_socket.close();
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

	std::char_traits<char>::int_type socketstream::underflow()
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
} /* namespace ibrcommon */
