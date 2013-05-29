/*
 * Base64Reader.cpp
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

#include "ibrcommon/data/Base64Reader.h"
#include "ibrcommon/Logger.h"
#include <stdio.h>
#include <vector>

namespace ibrcommon
{
	Base64Reader::Base64Reader(std::istream &stream, const size_t limit, const size_t buffer)
	 : std::istream(this), _stream(stream), data_buf_(buffer), data_size_(buffer), _base64_state(0), _base64_padding(0), _byte_read(0), _byte_limit(limit)
	{
		setg(0, 0, 0);
	}

	Base64Reader::~Base64Reader()
	{
	}

	void Base64Reader::set_b64(char val)
	{
		switch (_base64_state)
		{
		case 0:
			_group.b64_0(val);
			_base64_state++;
			break;

		case 1:
			_group.b64_1(val);
			_base64_state++;
			break;

		case 2:
			_group.b64_2(val);
			_base64_state++;
			break;

		case 3:
			_group.b64_3(val);
			_base64_state++;
			break;
		}
	}

	std::char_traits<char>::int_type Base64Reader::underflow()
	{
		// signal EOF if end of stream is reached
		if (_stream.eof())
		{
			return std::char_traits<char>::eof();
		}

		if ((_byte_limit > 0) && (_byte_read >= _byte_limit))
		{
			return std::char_traits<char>::eof();
		}

		// read some data
		std::vector<char> buffer(data_size_);

		if (_byte_limit > 0)
		{
			// get the remaining bytes
			size_t bytes_to_read = _byte_limit - _byte_read;

			// calculate base64 length for given plaintext length
			bytes_to_read = Base64::getLength(bytes_to_read);

			// minus pending bytes in the buffer
			bytes_to_read -= _base64_state;

			// debug
			IBRCOMMON_LOGGER_DEBUG_TAG("Base64Reader", 60) <<
					"[Base64Reader] base64 bytes to read: " << bytes_to_read <<
					"; payload bytes: " << (_byte_limit - _byte_read) <<
					"; byte in buffer: " << (int)_base64_state << IBRCOMMON_LOGGER_ENDL;

			// choose buffer size of remaining bytes
			if (bytes_to_read > data_size_) bytes_to_read = data_size_;

			// read from the stream
			_stream.read((char*)&buffer[0], bytes_to_read);
		}
		else
		{
			_stream.read((char*)&buffer[0], data_size_);
		}

		size_t len = _stream.gcount();

		// position in array
		size_t decoded_bytes = 0;

		for (size_t i = 0; i < len; ++i)
		{
			const int c = Base64::getCharType( buffer[i] );

			switch (c)
			{
				case Base64::UNKOWN_CHAR:
					// skip unknown chars
					continue;

				case Base64::EQUAL_CHAR:
				{
					switch (_base64_state)
					{
					case 0:
						// error - first character can not be a '='
						return std::char_traits<char>::eof();

					case 1:
						// error - second character can not be a '='
						return std::char_traits<char>::eof();

					case 2:
						// only one byte left
						_base64_padding = 2;
						break;

					case 3:
						// only one byte left
						if (_base64_padding == 0)
						{
							_base64_padding = 3;
						}
						break;
					}

					set_b64(0);
					break;
				}

				default:
				{
					// put char into the decode buffer
					set_b64(static_cast<char>(c));
					break;
				}
			}

			// when finished
			if (_base64_state == 4)
			{
				switch (_base64_padding)
				{
				case 0:
					data_buf_[decoded_bytes] = _group.get_0(); decoded_bytes++;
					data_buf_[decoded_bytes] = _group.get_1(); decoded_bytes++;
					data_buf_[decoded_bytes] = _group.get_2(); decoded_bytes++;
					break;

				case 3:
					data_buf_[decoded_bytes] = _group.get_0(); decoded_bytes++;
					data_buf_[decoded_bytes] = _group.get_1(); decoded_bytes++;
					break;

				case 2:
					data_buf_[decoded_bytes] = _group.get_0(); decoded_bytes++;
					break;
				}

				_base64_state = 0;
				_base64_padding = 0;
				_group.zero();
			}
		}

		// if we could not decode any byte, we have to get more input bytes
		if (decoded_bytes == 0)
		{
			// call underflow() recursively
			return underflow();
		}

		// Since the input buffer content is now valid (or is new)
		// the get pointer should be initialized (or reset).
		setg(&data_buf_[0], &data_buf_[0], &data_buf_[0] + decoded_bytes);

		if (_byte_limit > 0)
		{
			_byte_read += decoded_bytes;
		}

		return std::char_traits<char>::not_eof(data_buf_[0]);
	}
}
