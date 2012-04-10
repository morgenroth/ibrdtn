/*
 * HashStream.h
 *
 *  Created on: 12.07.2010
 *      Author: morgenro
 */

#ifndef HASHSTREAM_H_
#define HASHSTREAM_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include <iostream>

namespace ibrcommon
{
	class HashStream : public std::basic_streambuf<char, std::char_traits<char> >, public std::iostream
	{
	public:
		HashStream(const size_t hash, const size_t buffer = 2048);
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
		char *data_buf_;

		// length of the data buffer
		size_t data_size_;

		// Input buffer (contains the hash after finalize)
		char *hash_buf_;

		// length of the hash
		unsigned int hash_size_;

		bool final_;
	};
}

#endif /* HASHSTREAM_H_ */
