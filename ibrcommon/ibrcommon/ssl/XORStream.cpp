/*
 * XORStream.cpp
 *
 *  Created on: 13.07.2010
 *      Author: morgenro
 */

#include "ibrcommon/ssl/XORStream.h"

namespace ibrcommon
{
	XORStream::XORStream(std::ostream &stream, const CipherMode mode, std::string key)
	 : CipherStream(stream, mode), _key(key), _key_pos(0)
	{
	}

	XORStream::~XORStream()
	{
	}

	void XORStream::encrypt(char *buf, const size_t size)
	{
		const char *keydata = _key.c_str();
		const size_t keylength = _key.length();

		for (size_t i = 0; i < size; i++)
		{
			buf[i] = buf[i] ^ keydata[_key_pos];
			_key_pos++;
			if (_key_pos > keylength) _key_pos = 0;
		}
	}

	void XORStream::decrypt(char *buf, const size_t size)
	{
		const char *keydata = _key.c_str();
		const size_t keylength = _key.length();

		for (size_t i = 0; i < size; i++)
		{
			buf[i] = buf[i] ^ keydata[_key_pos];
			_key_pos++;
			if (_key_pos > keylength) _key_pos = 0;
		}
	}
}
