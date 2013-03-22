/*
 * Base64Reader.h
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

#ifndef BASE64READER_H_
#define BASE64READER_H_

#include "ibrcommon/data/Base64.h"
#include <iostream>
#include <vector>
#include <string.h>
#include <stdint.h>

namespace ibrcommon
{
	class Base64Reader : public std::basic_streambuf<char, std::char_traits<char> >, public std::istream
	{
	public:
		Base64Reader(std::istream &stream, const size_t limit = 0, const size_t buffer = 2048);
		virtual ~Base64Reader();

	protected:
		virtual std::char_traits<char>::int_type underflow();

	private:
		void set_b64(char val);

		std::istream &_stream;

		// Output buffer
		std::vector<char> data_buf_;

		// length of the data buffer
		size_t data_size_;

		uint8_t _base64_state;

		Base64::Group _group;
		char _base64_padding;

		size_t _byte_read;
		size_t _byte_limit;
	};
}

#endif /* BASE64READER_H_ */
