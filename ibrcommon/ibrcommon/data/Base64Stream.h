/*
 * Base64Stream.h
 *
 *  Created on: 16.06.2011
 *      Author: morgenro
 */

#ifndef BASE64STREAM_H_
#define BASE64STREAM_H_

#include "ibrcommon/data/Base64.h"
#include <iostream>
#include <streambuf>
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
		char *data_buf_;

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
