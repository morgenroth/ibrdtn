/*
 * HashStream.h
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

#ifndef HASHSTREAM_H_
#define HASHSTREAM_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include <iostream>
#include <vector>

namespace ibrcommon
{
	class HashStream : public std::basic_streambuf<char, std::char_traits<char> >, public std::iostream
	{
	public:
		HashStream(const unsigned int hash, const size_t buffer = 2048);
		virtual ~HashStream();

		static std::string extract(std::istream &stream);

	protected:
		virtual void update(char *buf, const size_t size) = 0;
		virtual void finalize(char * hash, unsigned int &size) = 0;

		virtual int sync();
		virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
		virtual std::char_traits<char>::int_type underflow();

	private:
		// Output buffer
		std::vector<char> data_buf_;

		// length of the data buffer
		size_t data_size_;

		// Input buffer (contains the hash after finalize)
		std::vector<char> hash_buf_;

		// length of the hash
		unsigned int hash_size_;

		bool final_;
	};
}

#endif /* HASHSTREAM_H_ */
