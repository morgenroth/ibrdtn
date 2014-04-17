/*
 * SHA256Stream.cpp
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

#include "ibrcommon/ssl/SHA256Stream.h"

namespace ibrcommon
{
	SHA256Stream::SHA256Stream()
	 : HashStream(SHA256_DIGEST_LENGTH, BUFF_SIZE), finalized_(false)
	{
		SHA256_Init(&ctx_);
	}

	SHA256Stream::~SHA256Stream()
	{
		if (!finalized_) {
			char buf[SHA256_DIGEST_LENGTH];
			SHA256_Final((unsigned char*)&buf, &ctx_);
		}
	}

	void SHA256Stream::update(char *buf, const size_t size)
	{
		// hashing
		SHA256_Update(&ctx_, (unsigned char*)buf, size);
	}

	void SHA256Stream::finalize(char * hash, unsigned int &size)
	{
		if (!finalized_) {
			SHA256_Final((unsigned char*)hash, &ctx_);
			size = SHA256_DIGEST_LENGTH;
			finalized_ = true;
		}
	}
} /* namespace ibrcommon */
