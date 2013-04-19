/*
 * XORStream.cpp
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

		for (size_t i = 0; i < size; ++i)
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

		for (size_t i = 0; i < size; ++i)
		{
			buf[i] = buf[i] ^ keydata[_key_pos];
			_key_pos++;
			if (_key_pos > keylength) _key_pos = 0;
		}
	}
}
