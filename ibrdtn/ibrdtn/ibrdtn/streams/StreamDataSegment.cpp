/*
 * StreamDataSegment.cpp
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

#include "ibrdtn/streams/StreamDataSegment.h"
#include "ibrdtn/data/Number.h"

namespace dtn
{
	namespace streams
	{
		StreamDataSegment::StreamDataSegment(SegmentType type, const dtn::data::Number &size)
		 : _value(size), _type(type), _reason(MSG_SHUTDOWN_IDLE_TIMEOUT), _flags(0)
		{
		}

		StreamDataSegment::StreamDataSegment(SegmentType type)
		: _value(0), _type(type), _reason(MSG_SHUTDOWN_IDLE_TIMEOUT), _flags(0)
		{
		}

		StreamDataSegment::StreamDataSegment(ShutdownReason reason, const dtn::data::Number &reconnect)
		: _value(reconnect), _type(MSG_SHUTDOWN), _reason(reason), _flags(3)
		{
		}

		StreamDataSegment::~StreamDataSegment()
		{
		}

		std::ostream &operator<<(std::ostream &stream, const StreamDataSegment &seg)
		{
			char header = seg._flags;
			header |= static_cast<char>((seg._type & 0x0F) << 4);

			// write the header (1-byte)
			stream.put(header);

			switch (seg._type)
			{
			case StreamDataSegment::MSG_DATA_SEGMENT:
				// write the length + data
				stream << seg._value;
				break;

			case StreamDataSegment::MSG_ACK_SEGMENT:
				// write the acknowledged length
				stream << seg._value;
				break;

			case StreamDataSegment::MSG_REFUSE_BUNDLE:
				break;

			case StreamDataSegment::MSG_KEEPALIVE:
				break;

			case StreamDataSegment::MSG_SHUTDOWN:
				// write the reason (char) + reconnect time (SDNV)
				stream.put((char)seg._reason);
				stream << seg._value;
				break;
			}

			return stream;
		}

		std::istream &operator>>(std::istream &stream, StreamDataSegment &seg)
		{
			char header = 0;
			stream.get(header);

			seg._type = StreamDataSegment::SegmentType( (header & 0xF0) >> 4 );
			seg._flags = (header & 0x0F);

			switch (seg._type)
			{
			case StreamDataSegment::MSG_DATA_SEGMENT:
				// read the length
				stream >> seg._value;
				break;

			case StreamDataSegment::MSG_ACK_SEGMENT:
				// read the acknowledged length
				stream >> seg._value;
				break;

			case StreamDataSegment::MSG_REFUSE_BUNDLE:
				break;

			case StreamDataSegment::MSG_KEEPALIVE:
				break;

			case StreamDataSegment::MSG_SHUTDOWN:
				// read the reason (char) + reconnect time (SDNV)
				char reason;
				stream.get(reason);
				seg._reason = StreamDataSegment::ShutdownReason(reason);

				stream >> seg._value;
				break;
			}

			return stream;
		}
	}
}
