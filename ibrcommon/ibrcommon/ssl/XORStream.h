/*
 * XORStream.h
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

#ifndef XORSTREAM_H_
#define XORSTREAM_H_

#include "ibrcommon/ssl/CipherStream.h"
#include <string>

namespace ibrcommon
{
	class XORStream : public CipherStream
	{
	public:
		XORStream(std::ostream &stream, const CipherMode mode, std::string key);
		virtual ~XORStream();

	protected:
		virtual void encrypt(char *buf, const size_t size);
		virtual void decrypt(char *buf, const size_t size);

	private:
		std::string _key;
		size_t _key_pos;
	};
}

#endif /* XORSTREAM_H_ */
