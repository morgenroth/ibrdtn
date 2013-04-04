/*
 * iobuffer.cpp
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

#include "ibrcommon/data/iobuffer.h"
#include "ibrcommon/thread/MutexLock.h"
#include <string.h>

namespace ibrcommon
{
	iobuffer::iobuffer(const size_t buffer)
	 : _eof(false), _data_size(0), _buf_size(buffer), _input_buf(buffer), _interim_buf(buffer), _output_buf(buffer)
	{
		// Initialize get pointer.  This should be zero so that underflow is called upon first read.
		setg(0, 0, 0);
		setp(&_input_buf[0], &_input_buf[0] + _buf_size - 1);
	}

	iobuffer::~iobuffer()
	{
	}

	void iobuffer::finalize()
	{
		ibrcommon::MutexLock l(_data_cond);
		_eof = true;
		_data_cond.signal(true);
	}

	int iobuffer::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(
				std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
				: 0;

		return ret;
	}

	std::char_traits<char>::int_type iobuffer::overflow(std::char_traits<char>::int_type c)
	{
		char *ibegin = &_input_buf[0];
		char *iend = pptr();

		// mark the buffer as free
		setp(&_input_buf[0], &_input_buf[0] + _buf_size - 1);

		if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
		{
			*iend++ = std::char_traits<char>::to_char_type(c);
		}

		// if there is nothing to send, just return
		if ((iend - ibegin) == 0)
		{
			return std::char_traits<char>::not_eof(c);
		}

		ibrcommon::MutexLock l(_data_cond);

		// wait until the buffer is empty
		while (_data_size > 0)
		{
			if (_eof) throw ibrcommon::Exception("iobuffer was finalized");
			_data_cond.wait();
		}

		// set the new output data size
		_data_size = (iend - ibegin);

		// copy the data to the interim buffer
		::memcpy(&_interim_buf[0], &_input_buf[0], _data_size);

		// signal the new data
		_data_cond.signal(true);

		return std::char_traits<char>::not_eof(c);
	}

	std::char_traits<char>::int_type iobuffer::underflow()
	{
		ibrcommon::MutexLock l(_data_cond);

		// wait until some data is available
		while (_data_size == 0)
		{
			// return EOF if the end of the file is reached
			if (_eof) return std::char_traits<char>::eof();

			_data_cond.wait();
		}

		// copy the data to the output buffer
		::memcpy(&_output_buf[0], &_interim_buf[0], _data_size);

		// Since the input buffer content is now valid (or is new)
		// the get pointer should be initialized (or reset).
		setg(&_output_buf[0], &_output_buf[0], &_output_buf[0] + _data_size);

		// signal the empty buffer
		_data_size = 0;
		_data_cond.signal(true);

		return std::char_traits<char>::not_eof(_output_buf[0]);
	}
}
