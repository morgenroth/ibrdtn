/*
 * InputCipherStream.h
 *
 *  Created on: 13.07.2010
 *      Author: morgenro
 */

#ifndef INPUTCIPHERSTREAM_H_
#define INPUTCIPHERSTREAM_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include <iostream>
#include <iostream>

namespace ibrcommon
{
	class InputCipherStream : public std::basic_streambuf<char, std::char_traits<char> >, public std::istream
	{
	public:
		enum CipherMode
		{
			CIPHER_ENCRYPT = 0,
			CIPHER_DECRYPT = 1
		};

		InputCipherStream(std::istream &stream, const CipherMode mode = CIPHER_DECRYPT, const size_t buffer = 2048);
		virtual ~InputCipherStream();

	protected:
		virtual void encrypt(char *buf, const size_t size) = 0;
		virtual void decrypt(char *buf, const size_t size) = 0;

		virtual int sync();
		virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
		virtual std::char_traits<char>::int_type underflow();

	private:
		std::istream &_stream;

		CipherMode _mode;

		// Output buffer
		char *data_buf_;

		// length of the data buffer
		size_t data_size_;
	};
}

#endif /* INPUTCIPHERSTREAM_H_ */
