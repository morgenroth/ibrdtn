/*
 * Base64Stream.h
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

#ifndef BASE64STREAM_H_
#define BASE64STREAM_H_

#include "ibrcommon/data/Base64.h"
#include <iostream>
#include <streambuf>
#include <vector>
#include <stdint.h>

namespace ibrcommon
{
	class Base64Stream : public std::basic_streambuf<char, std::char_traits<char> >, public std::ostream
	{
	public:
		Base64Stream(std::ostream &stream, bool decode = false, const size_t linebreak = 0, const size_t buffer = 2048);
		virtual ~Base64Stream();

	protected:
		virtual int sync();
		virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());

	private:
		void set_b64(char val);
		void set_byte(char val);

		void __flush_encoder__();

		bool _decode;

		std::ostream &_stream;

		// Output buffer
		std::vector<char> data_buf_;

		// length of the data buffer
		size_t data_size_;

		uint8_t _base64_state;

		size_t _char_counter;

		Base64::Group _group;
		char _base64_padding;

		const size_t _linebreak;
	};
}

#endif /* BASE64STREAM_H_ */
