/*
 * appstreambuf.cpp
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
#include "ibrcommon/appstreambuf.h"

namespace ibrcommon
{
	appstreambuf::appstreambuf(const std::string &command, appstreambuf::Mode mode)
	 : m_buf( BUF_SIZE+1 )
	{
		// execute the command
		if (mode == MODE_READ)
		{
			setg(0, 0, 0);
			_handle = popen(command.c_str(), "r");
		}
		else
		{
			setp( &m_buf[0], &m_buf[0] + (m_buf.size() - 1) );
			_handle = popen(command.c_str(), "w");
		}
	}

	appstreambuf::~appstreambuf()
	{
		// close the handle
		pclose(_handle);
	}

	std::char_traits<char>::int_type appstreambuf::underflow()
	{
		// read the stdout of the process
		size_t ret = 0;

		if (feof(_handle))
			return std::char_traits<char>::eof();

		ret = fread(&m_buf[0], sizeof(char), m_buf.size(), _handle);

		// Since the input buffer content is now valid (or is new)
		// the get pointer should be initialized (or reset).
		setg(&m_buf[0], &m_buf[0], &m_buf[0] + ret);

		return std::char_traits<char>::not_eof((unsigned char) m_buf[0]);
	}

	int appstreambuf::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(std::char_traits<char>::eof()),
				std::char_traits<char>::eof()) ? -1 : 0;

		return ret;
	}

	std::char_traits<char>::int_type appstreambuf::overflow( std::char_traits<char>::int_type m )
	{
		char *ibegin = pbase();
		char *iend = pptr();

		// if there is nothing to send, just return
		if ( iend <= ibegin ) return std::char_traits<char>::not_eof(m);

		// mark the buffer as free
		setp( &m_buf[0], &m_buf[0] + (m_buf.size() - 1) );

		if(!std::char_traits<char>::eq_int_type(m, std::char_traits<char>::eof())) {
			*iend++ = std::char_traits<char>::to_char_type(m);
		}

		const size_t avail_data = (iend - ibegin);

		// write the data
		if (fwrite(ibegin, sizeof(char), avail_data, _handle) != avail_data)
		{
			return std::char_traits<char>::eof();
		}

		return std::char_traits<char>::not_eof(m);
	}
}
