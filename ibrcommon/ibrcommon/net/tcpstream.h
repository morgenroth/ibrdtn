/*
 * tcpstream.h
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

#ifndef IBRCOMMON_TCPSTREAM_H_
#define IBRCOMMON_TCPSTREAM_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "ibrcommon/net/vsocket.h"

namespace ibrcommon
{
	class ConnectionClosedException : public vsocket_exception
	{
	public:
		ConnectionClosedException(string what = "The connection has been closed.") throw() : ibrcommon::vsocket_exception(what)
		{
		};
	};

	class tcpstream : public std::basic_streambuf<char, std::char_traits<char> >, public std::iostream
	{
	public:
		enum stream_error
		{
			ERROR_NONE = 0,
			ERROR_EPIPE = 1,
			ERROR_CLOSED = 2,
			ERROR_WRITE = 3,
			ERROR_READ = 4,
			ERROR_RESET = 5
		};

		// The size of the input and output buffers.
		static const size_t BUFF_SIZE = 5120;

		tcpstream(int socket = -1);
		virtual ~tcpstream();

		string getAddress() const;
		int getPort() const;

		void close(bool errorcheck = false);

		stream_error errmsg;

		void enableKeepalive();
		void enableLinger(int linger);
		void enableNoDelay();

		void setTimeout(unsigned int value);

	protected:
		virtual int sync();
		virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
		virtual std::char_traits<char>::int_type underflow();
		void interrupt();

		int _socket;

		class select_exception : public ibrcommon::Exception
		{
		public:
			enum EXCEPTION_TYPE
			{
				SELECT_UNKNOWN = 0,
				SELECT_TIMEOUT = 1,
				SELECT_CLOSED = 2,
				SELECT_ERROR = 3
			};

			select_exception(EXCEPTION_TYPE) throw() : ibrcommon::Exception("select exception")
			{
			};
		};

		int select(int int_pipe, bool &read, bool &write, bool &error, int timeout = 0) throw (select_exception);

		int _interrupt_pipe_read[2];
		int _interrupt_pipe_write[2];

	private:
		// Input buffer
		char *in_buf_;
		// Output buffer
		char *out_buf_;

		// flag for non-blocking mode
		bool _nonblocking;

		unsigned int _timeout;
	};
}

#endif /* TCPSTREAM_H_ */
