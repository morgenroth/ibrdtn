/*
 * MD5Stream.h
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

#ifndef MD5STREAM_H_
#define MD5STREAM_H_

#include "ibrcommon/ssl/HashStream.h"
#include <openssl/md5.h>

namespace ibrcommon
{
	class MD5Stream : public ibrcommon::HashStream
	{
	public:
		// The size of the input and output buffers.
		static const size_t BUFF_SIZE = 2048;

		MD5Stream();
		virtual ~MD5Stream();

	protected:
		virtual void update(char *buf, const size_t size);
		virtual void finalize(char * hash, unsigned int &size);

	private:
		MD5_CTX ctx_;
		bool finalized_;
	};
} /* namespace ibrcommon */

#endif /* MD5STREAM_H_ */
