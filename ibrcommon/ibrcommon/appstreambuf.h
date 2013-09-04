/*
 * appstreambuf.h
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

#ifndef IBRCOMMON_APPSTREAMBUF_H_
#define IBRCOMMON_APPSTREAMBUF_H_

#include <streambuf>
#include <vector>
#include <string>
#include <stdio.h>

namespace ibrcommon
{
	/**
	 * @class appstreambuf
	 *
	 * @brief Stream buffer for external applications.
	 *
	 * A appstreambuf is a buffer provides access to the standard input / output of a system call.
	 * Embedded in an iostream objects it is possible to read and write to a external application
	 * like to a common stream object.
	 *
	 * This stream buffer is limited to read only or write only. A bidirectional access is not possible.
	 */
	class appstreambuf : public std::basic_streambuf<char, std::char_traits<char> >
	{
	public:
		enum { BUF_SIZE = 512 };

		enum Mode
		{
			MODE_READ = 0,
			MODE_WRITE = 1
		};

		/**
		 * Constructor of the appstreambuf
		 * @param command A command to execute and connect to with output or input.
		 * @param mode Specifies the mode to work read only or write only.
		 */
		appstreambuf(const std::string &command, appstreambuf::Mode mode);
		virtual ~appstreambuf();

	protected:
		virtual std::char_traits<char>::int_type underflow();
		virtual int sync();
		virtual std::char_traits<char>::int_type overflow( std::char_traits<char>::int_type m = traits_type::eof() );

	private:
		std::vector< char_type > m_buf;
		FILE *_handle;
	};
}

#endif /* APPSTREAMBUF_H_ */
