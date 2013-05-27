/*
 * Base64Stream.cpp
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

#include "ibrcommon/data/Base64Stream.h"
#include "ibrcommon/data/Base64.h"
#include <string.h>

namespace ibrcommon
{
	Base64Stream::Base64Stream(std::ostream &stream, bool decode, const size_t linebreak, const size_t buffer)
	 : std::ostream(this), _decode(decode), _stream(stream), data_buf_(buffer), data_size_(buffer), _base64_state(0), _char_counter(0), _base64_padding(0), _linebreak(linebreak)
	{
		setp(&data_buf_[0], &data_buf_[0] + data_size_ - 1);
	}

	Base64Stream::~Base64Stream()
	{
	}

	void Base64Stream::set_byte(char val)
	{
		switch (_base64_state)
		{
		case 0:
			_group.set_0(val);
			_base64_state++;
			break;

		case 1:
			_group.set_1(val);
			_base64_state++;
			break;

		case 2:
			_group.set_2(val);
			_base64_state++;
			break;
		}
	}

	void Base64Stream::set_b64(char val)
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

	int Base64Stream::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(
				std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
				: 0;

		if (!_decode && (_base64_state > 0))
		{
			__flush_encoder__();
		}

		return ret;
	}

	std::char_traits<char>::int_type Base64Stream::overflow(std::char_traits<char>::int_type c)
	{
		char *ibegin = &data_buf_[0];
		char *iend = pptr();

		if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
		{
			*iend++ = std::char_traits<char>::to_char_type(c);
		}

		// mark the buffer as free
		setp(&data_buf_[0], &data_buf_[0] + data_size_ - 1);

		// available data
		size_t len = (iend - ibegin);

		// if there is nothing to send, just return
		if (len == 0)
		{
			return std::char_traits<char>::not_eof(c);
		}

		// for each byte...
		for (size_t i = 0; i < len; ++i)
		{
			// do cipher stuff
			if (_decode)
			{
				const int c = Base64::getCharType( ibegin[i] );

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
					if (_base64_padding == 0)
					{
						_stream.put( _group.get_0() );
						_stream.put( _group.get_1() );
						_stream.put( _group.get_2() );
					}
					else if (_base64_padding == 3)
					{
						_stream.put( _group.get_0() );
						_stream.put( _group.get_1() );
					}
					else if (_base64_padding == 2)
					{
						_stream.put( _group.get_0() );
					}

					_base64_state = 0;
					_base64_padding = 0;
					_group.zero();
				}
			}
			else
			{
				// put char into the encode buffer
				set_byte(ibegin[i]);

				if (_base64_state == 3)
				{
					__flush_encoder__();
					_group.zero();
				}
			}
		}

		return std::char_traits<char>::not_eof(c);
	}

	void Base64Stream::__flush_encoder__()
	{
		switch (_base64_state)
		{
		default:
			break;

		case 1:
			_stream.put( Base64::encodeCharacterTable[ _group.b64_0() ] );
			_stream.put( Base64::encodeCharacterTable[ _group.b64_1() ] );
			_stream.put( '=' );
			_stream.put( '=' );
			break;

		case 2:
			_stream.put( Base64::encodeCharacterTable[ _group.b64_0() ] );
			_stream.put( Base64::encodeCharacterTable[ _group.b64_1() ] );
			_stream.put( Base64::encodeCharacterTable[ _group.b64_2() ] );
			_stream.put( '=' );
			break;

		case 3:
			_stream.put( Base64::encodeCharacterTable[ _group.b64_0() ] );
			_stream.put( Base64::encodeCharacterTable[ _group.b64_1() ] );
			_stream.put( Base64::encodeCharacterTable[ _group.b64_2() ] );
			_stream.put( Base64::encodeCharacterTable[ _group.b64_3() ] );
			break;
		}

		_base64_state = 0;
		_char_counter += 4;

		if (_char_counter >= _linebreak)
		{
			_stream.put('\n');
			_char_counter = 0;
		}
	}
}
