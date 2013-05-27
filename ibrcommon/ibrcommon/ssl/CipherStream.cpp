/*
 * CipherStream.cpp
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

#include "ibrcommon/ssl/CipherStream.h"

namespace ibrcommon
{
	CipherStream::CipherStream(std::ostream &stream, const CipherMode mode, const size_t buffer)
	 : std::ostream(this), _mode(mode), _stream(stream), data_buf_(buffer), data_size_(buffer)
	{
		setp(&data_buf_[0], &data_buf_[0] + data_size_ - 1);
	}

	CipherStream::~CipherStream()
	{
	}

	void CipherStream::encrypt(std::iostream& stream)
	{
		char buf[4096];
		while (!stream.eof())
		{
			std::ios::pos_type pos = stream.tellg();

			stream.read((char*)&buf, 4096);
			size_t bytes = stream.gcount();

			encrypt(buf, bytes);

			// clear the error flags if we reached the end of the file
			// but need to write some data
			if (stream.eof() && (bytes > 0)) stream.clear();

			stream.seekp(pos, std::ios::beg);
			stream.write((char*)&buf, bytes);
		}

		stream.flush();
		encrypt_final();
	}

	void CipherStream::decrypt(std::iostream& stream)
	{
		char buf[4096];
		while (!stream.eof())
		{
			std::ios::pos_type pos = stream.tellg();

			stream.read((char*)&buf, 4096);
			size_t bytes = stream.gcount();

			decrypt(buf, bytes);

			// clear the error flags if we reached the end of the file
			// but need to write some data
			if (stream.eof() && (bytes > 0)) stream.clear();

			stream.seekp(pos, std::ios::beg);
			stream.write((char*)&buf, bytes);
		}

		stream.flush();
		decrypt_final();
	}

	int CipherStream::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(
				std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
				: 0;

		// do cipher stuff
		switch (_mode)
		{
		case CIPHER_ENCRYPT:
			encrypt_final();
			break;

		case CIPHER_DECRYPT:
			decrypt_final();
			break;
		}

		return ret;
	}

	std::char_traits<char>::int_type CipherStream::overflow(std::char_traits<char>::int_type c)
	{
		char *ibegin = &data_buf_[0];
		char *iend = pptr();

		// mark the buffer as free
		setp(&data_buf_[0], &data_buf_[0] + data_size_ - 1);

		if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
		{
			*iend++ = std::char_traits<char>::to_char_type(c);
		}

		// if there is nothing to send, just return
		if ((iend - ibegin) == 0)
		{
			return std::char_traits<char>::not_eof(c);
		}

		// do cipher stuff
		switch (_mode)
		{
		case CIPHER_ENCRYPT:
			encrypt(&data_buf_[0], (iend - ibegin));
			break;

		case CIPHER_DECRYPT:
			decrypt(&data_buf_[0], (iend - ibegin));
			break;
		}

		// write result to stream
		_stream.write(&data_buf_[0], (iend - ibegin));

		return std::char_traits<char>::not_eof(c);
	}
}
