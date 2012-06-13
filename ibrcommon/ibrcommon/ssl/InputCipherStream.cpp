/*
 * InputCipherStream.cpp
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

#include "ibrcommon/ssl/InputCipherStream.h"

namespace ibrcommon
{
	InputCipherStream::InputCipherStream(std::istream &stream, const CipherMode mode, const size_t buffer)
	 : std::istream(this), _stream(stream), _mode(mode), data_buf_(new char[buffer]), read_buf_(new char[buffer]), data_size_(buffer)
	{
		// Initialize get pointer.  This should be zero so that underflow is called upon first read.
		setg(0, 0, 0);
		setp(data_buf_, data_buf_ + data_size_ - 1);
	}

	InputCipherStream::~InputCipherStream()
	{
		delete[] data_buf_;
	}

	std::char_traits<char>::int_type InputCipherStream::underflow()
	{
		_stream.read(data_buf_, data_size_);
		int bytes = _stream.gcount();

		if (bytes == 0)
		{
			return std::char_traits<char>::eof();
		}

		// Since the input buffer content is now valid (or is new)
		// the get pointer should be initialized (or reset).
		setg(data_buf_, data_buf_, data_buf_ + bytes);

		return std::char_traits<char>::not_eof((unsigned char) data_buf_[0]);
	}
}
