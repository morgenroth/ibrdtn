/*
 * Base64Reader.h
 *
 *  Created on: 21.06.2011
 *      Author: morgenro
 */

#ifndef BASE64READER_H_
#define BASE64READER_H_

#include "ibrcommon/data/Base64.h"
#include <iostream>
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
		char *data_buf_;

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
