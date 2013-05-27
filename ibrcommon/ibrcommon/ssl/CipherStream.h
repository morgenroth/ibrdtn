/*
 * CipherStream.h
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

#ifndef CIPHERSTREAM_H_
#define CIPHERSTREAM_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include <iostream>
#include <vector>

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
		std::vector<char> data_buf_;

		// length of the data buffer
		size_t data_size_;
	};
}

#endif /* CIPHERSTREAM_H_ */
