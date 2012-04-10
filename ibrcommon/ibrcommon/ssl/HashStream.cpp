/*
 * HashStream.cpp
 *
 *  Created on: 12.07.2010
 *      Author: morgenro
 */

#include "HashStream.h"
#include <sstream>

namespace ibrcommon
{
	HashStream::HashStream(const size_t hash, const size_t buffer)
	 : iostream(this), data_buf_(new char[buffer]), data_size_(buffer), hash_buf_(new char[hash]), hash_size_(hash), final_(false)
	{
		// Initialize get pointer.  This should be zero so that underflow is called upon first read.
		setg(0, 0, 0);
		setp(data_buf_, data_buf_ + data_size_ - 1);
	}

	HashStream::~HashStream()
	{
		delete[] data_buf_;
		delete[] hash_buf_;
	}

	std::string HashStream::extract(std::istream &stream)
	{
		std::stringstream buf;
		buf << stream.rdbuf();
		return buf.str();
	}

	int HashStream::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(
				std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
				: 0;

		return ret;
	}

	std::char_traits<char>::int_type HashStream::overflow(std::char_traits<char>::int_type c)
	{
		char *ibegin = data_buf_;
		char *iend = pptr();

		// mark the buffer as free
		setp(data_buf_, data_buf_ + data_size_ - 1);

		if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
		{
			*iend++ = std::char_traits<char>::to_char_type(c);
		}

		// if there is nothing to send, just return
		if ((iend - ibegin) == 0)
		{
			return std::char_traits<char>::not_eof(c);
		}

		// hashing
		update(data_buf_, (iend - ibegin));

		return std::char_traits<char>::not_eof(c);
	}

	std::char_traits<char>::int_type HashStream::underflow()
	{
		// TODO: add seek mechanisms to reset the istream

		if (!final_)
		{
			finalize(hash_buf_, hash_size_);
			final_ = true;

			// Since the input buffer content is now valid (or is new)
			// the get pointer should be initialized (or reset).
			setg(hash_buf_, hash_buf_, hash_buf_ + hash_size_);

			return std::char_traits<char>::not_eof((unsigned char) hash_buf_[0]);
		}

		return std::char_traits<char>::eof();
	}
}
