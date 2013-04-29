/*
 * NativeSerializer.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "api/NativeSerializer.h"

namespace dtn
{
	namespace api
	{
		NativeCallbackStream::NativeCallbackStream(NativeSerializerCallback &cb)
		 : _output_buf(4096), _callback(cb)
		{
			setp(&_output_buf[0], &_output_buf[4096 - 1]);
		}

		NativeCallbackStream::~NativeCallbackStream() {

		}

		int NativeCallbackStream::sync()
		{
			int ret = std::char_traits<char>::eq_int_type(this->overflow(
					std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
					: 0;

			return ret;
		}

		std::char_traits<char>::int_type NativeCallbackStream::overflow(std::char_traits<char>::int_type c)
		{
			char *ibegin = &_output_buf[0];
			char *iend = pptr();

			// mark the buffer as free
			setp(&_output_buf[0], &_output_buf[4096 - 1]);

			if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
			{
				*iend++ = std::char_traits<char>::to_char_type(c);
			}

			// if there is nothing to send, just return
			if ((iend - ibegin) == 0)
			{
				return std::char_traits<char>::not_eof(c);
			}

			_callback.payload(&_output_buf[0], (iend - ibegin));

			return std::char_traits<char>::not_eof(c);
		}

		NativeSerializer::NativeSerializer(NativeSerializerCallback &cb, DataMode mode)
		 : _callback(cb), _mode(mode) {

		}

		NativeSerializer::~NativeSerializer() {
		}

		NativeSerializer& NativeSerializer::operator<<(const dtn::data::Bundle &obj)
		{
			_callback.beginBundle(obj);
			if (_mode != BUNDLE_HEADER)
			{
				for (dtn::data::Bundle::const_iterator it = obj.begin(); it != obj.end(); ++it) {
					const dtn::data::Block &block = (**it);
					_callback.beginBlock(block, block.getLength());

					if (_mode == BUNDLE_FULL)
					{
						dtn::data::Length len = 0;
						NativeCallbackStream streambuf(_callback);
						std::ostream stream(&streambuf);
						block.serialize(stream, len);
						stream << std::flush;
					}

					_callback.endBlock();
				}
			}
			_callback.endBundle();
			return (*this);
		}
	} /* namespace api */
} /* namespace dtn */
