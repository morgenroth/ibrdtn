/*
 * Base64.cpp
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

#include "ibrcommon/data/Base64.h"

#define _0000_0011 0x03
#define _1111_1100 0xFC
#define _1111_0000 0xF0
#define _0011_0000 0x30
#define _0011_1100 0x3C
#define _0000_1111 0x0F
#define _1100_0000 0xC0
#define _0011_1111 0x3F

namespace ibrcommon
{
	const char Base64::encodeCharacterTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	size_t Base64::getLength(size_t length)
	{
		// encoding = byte * (4/3)
		// calculate base64 length for given plaintext length
		// all bytes we have to read
		size_t ret = (length / 3) * 4;

		// plus one group if there are carry bytes
		if ((length % 3) > 0) ret += 4;

		return ret;
	}

	Base64::Group::Group()
	{
		zero();
	}

	Base64::Group::~Group()
	{
	}

	void Base64::Group::zero()
	{
		_data[0] = 0;
		_data[1] = 0;
		_data[2] = 0;
	}

	uint8_t Base64::Group::get_0() const
	{
		return _data[0];
	}

	uint8_t Base64::Group::get_1() const
	{
		return _data[1];
	}

	uint8_t Base64::Group::get_2() const
	{
		return _data[2];
	}

	void Base64::Group::set_0(uint8_t val)
	{
		_data[0] = val;
	}

	void Base64::Group::set_1(uint8_t val)
	{
		_data[1] = val;
	}

	void Base64::Group::set_2(uint8_t val)
	{
		_data[2] = val;
	}

	int Base64::Group::b64_0() const
	{
		return (_data[0] & _1111_1100) >> 2;
	}

	int Base64::Group::b64_1() const
	{
		return ((_data[0] & _0000_0011) << 4) + ((_data[1] & _1111_0000)>>4);
	}

	int Base64::Group::b64_2() const
	{
		return ((_data[1] & _0000_1111) << 2) + ((_data[2] & _1100_0000)>>6);
	}

	int Base64::Group::b64_3() const
	{
		return (_data[2] & _0011_1111);
	}

	void Base64::Group::b64_0(int val)
	{
		_data[0] = static_cast<uint8_t>( ((val & _0011_1111) << 2) | (_0000_0011 & _data[0]) );
	}

	void Base64::Group::b64_1(int val)
	{
		_data[0] = static_cast<uint8_t>( ((val & _0011_0000) >> 4) | (_1111_1100 & _data[0]) );
		_data[1] = static_cast<uint8_t>( ((val & _0000_1111) << 4) | (_0000_1111 & _data[1]) );
	}

	void Base64::Group::b64_2(int val)
	{
		_data[1] = static_cast<uint8_t>( ((val & _0011_1100) >> 2) | (_1111_0000 & _data[1]) );
		_data[2] = static_cast<uint8_t>( ((val & _0000_0011) << 6) | (_0011_1111 & _data[2]) );
	}

	void Base64::Group::b64_3(int val)
	{
		_data[2] = static_cast<uint8_t>( (val & _0011_1111) | (_1100_0000 & _data[2]) );
	}

	int Base64::getCharType(int _C)
	{
		if(encodeCharacterTable[62] == _C)
			return 62;

		if(encodeCharacterTable[63] == _C)
			return 63;

		if((encodeCharacterTable[0] <= _C) && (encodeCharacterTable[25] >= _C))
			return _C - encodeCharacterTable[0];

		if((encodeCharacterTable[26] <= _C) && (encodeCharacterTable[51] >= _C))
			return _C - encodeCharacterTable[26] + 26;

		if((encodeCharacterTable[52] <= _C) && (encodeCharacterTable[61] >= _C))
			return _C - encodeCharacterTable[52] + 52;

		if(_C == ((int)'='))
			return Base64::EQUAL_CHAR;

		return Base64::UNKOWN_CHAR;
	}
} /* namespace dtn */
