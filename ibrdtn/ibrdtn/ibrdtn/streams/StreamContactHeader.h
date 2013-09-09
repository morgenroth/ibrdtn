/*
 * StreamContactHeader.h
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



#ifndef STREAMCONTACTHEADER_H_
#define STREAMCONTACTHEADER_H_


#include "ibrdtn/data/EID.h"
#include "ibrdtn/data/Exceptions.h"
#include <sys/types.h>
#include <stdint.h>


using namespace dtn::data;

namespace dtn
{
	namespace streams
	{
		static const unsigned char TCPCL_VERSION = 3;

		class StreamContactHeader
		{
		public:
			enum HEADER_BITS
			{
				REQUEST_ACKNOWLEDGMENTS = 1 << 0,
				REQUEST_FRAGMENTATION = 1 << 1,
				REQUEST_NEGATIVE_ACKNOWLEDGMENTS = 1 << 2,
				/* this flag is implementation specific and not in the draft */
				REQUEST_TLS = 1 << 7,
				HANDSHAKE_SENDONLY = 0x80//!< The client only send bundle and do not want to received any bundle.
			};

			StreamContactHeader();
			StreamContactHeader(EID localeid);
			virtual ~StreamContactHeader();

			StreamContactHeader& operator=(const StreamContactHeader &other);

			const EID getEID() const;

			EID _localeid;
			dtn::data::Bitset<HEADER_BITS> _flags;
			uint16_t _keepalive;

			friend std::ostream &operator<<(std::ostream &stream, const StreamContactHeader &h);
			friend std::istream &operator>>(std::istream &stream, StreamContactHeader &h);
		};
	}
}

#endif /* STREAMCONTACTHEADER_H_ */
