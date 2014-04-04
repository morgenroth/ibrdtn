/*
 * MD5Stream.cpp
 *
 * Copyright (C) 2014 IBR, TU Braunschweig
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

#include "ibrcommon/ssl/MD5Stream.h"

namespace ibrcommon
{
	MD5Stream::MD5Stream()
	 : HashStream(MD5_DIGEST_LENGTH, BUFF_SIZE), finalized_(false)
	{
		MD5_Init(&ctx_);
	}

	MD5Stream::~MD5Stream()
	{
		if (!finalized_) {
			char buf[MD5_DIGEST_LENGTH];
			MD5_Final((unsigned char*)&buf, &ctx_);
		}
	}

	void MD5Stream::update(char *buf, const size_t size)
	{
		// hashing
		MD5_Update(&ctx_, (unsigned char*)buf, size);
	}

	void MD5Stream::finalize(char * hash, unsigned int &size)
	{
		if (!finalized_) {
			MD5_Final((unsigned char*)hash, &ctx_);
			size = MD5_DIGEST_LENGTH;
			finalized_ = true;
		}
	}
} /* namespace ibrcommon */
