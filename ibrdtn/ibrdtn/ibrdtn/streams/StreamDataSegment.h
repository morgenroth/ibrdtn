/*
 * StreamDataSegment.h
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


#ifndef STREAMDATASEGMENT_H_
#define STREAMDATASEGMENT_H_

#include <ibrdtn/data/Number.h>
#include <stdlib.h>
#include <iostream>
#include <stdint.h>

namespace dtn
{
	namespace streams
	{
		class StreamDataSegment
		{
		public:
			enum SegmentType
			{
				MSG_DATA_SEGMENT = 0x1,
				MSG_ACK_SEGMENT = 0x2,
				MSG_REFUSE_BUNDLE = 0x3,
				MSG_KEEPALIVE = 0x4,
				MSG_SHUTDOWN = 0x5
			};

			enum SegmentMark
			{
				MSG_MARK_BEGINN = 0x02,
				MSG_MARK_END = 0x01
			};

			enum ShutdownReason
			{
				MSG_SHUTDOWN_NONE = 0xff,
				MSG_SHUTDOWN_IDLE_TIMEOUT = 0x00,
				MSG_SHUTDOWN_VERSION_MISSMATCH = 0x01,
				MSG_SHUTDOWN_BUSY = 0x02
			};

			StreamDataSegment(SegmentType type, const dtn::data::Number &size); // creates a ACK or DATA segment
			StreamDataSegment(SegmentType type = MSG_KEEPALIVE); // Creates a Keep-Alive Message
			StreamDataSegment(ShutdownReason reason, const dtn::data::Number &reconnect = 0);  // Creates a Shutdown Message

			virtual ~StreamDataSegment();

			dtn::data::Number _value;
			SegmentType _type;
			ShutdownReason _reason;
			uint8_t _flags;

			friend std::ostream &operator<<(std::ostream &stream, const StreamDataSegment &seg);
			friend std::istream &operator>>(std::istream &stream, StreamDataSegment &seg);
		};
	}
}

#endif /* STREAMDATASEGMENT_H_ */
