/*
 * CipherStream.h
 *
 *  Created on: 13.07.2010
 *      Author: morgenro
 */

#ifndef CIPHERSTREAM_H_
#define CIPHERSTREAM_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include <iostream>

namespace ibrcommon
{
	class CipherStream : public std::basic_streambuf<char, std::char_traits<char> >, public std::ostream
	{
	public:
		enum CipherMode
		{
			CIPHER_ENCRYPT = 0,
			CIPHER_DECRYPT = 1
		};

		CipherStream(std::ostream &stream, const CipherMode mode = CIPHER_DECRYPT, const size_t buffer = 2048);
		virtual ~CipherStream();

		/**
		 * encrypt a seekable stream by reading and writing into the same stream
		 * @param stream
		 * @param key
		 * @param salt
		 */
		void encrypt(std::iostream& stream);

		/**
		 * * decrypt a seekable stream by reading and writing into the same stream
		 * @param stream
		 * @param key
		 * @param salt
		 * @param iv
		 * @param tag
		 */
		void decrypt(std::iostream& stream);

	protected:
		virtual void encrypt(char *buf, const size_t size) = 0;
		virtual void decrypt(char *buf, const size_t size) = 0;
		virtual void encrypt_final() {};
		virtual void decrypt_final() {};

		virtual int sync();
		virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());

		CipherMode _mode;

	private:
		std::ostream &_stream;

		// Output buffer
		char *data_buf_;

		// length of the data buffer
		size_t data_size_;
	};
}

#endif /* CIPHERSTREAM_H_ */
