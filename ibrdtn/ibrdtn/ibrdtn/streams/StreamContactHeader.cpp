/*
 * StreamContactHeader.cpp
 *
 *  Created on: 30.06.2009
 *      Author: morgenro
 */

#include "ibrdtn/streams/StreamContactHeader.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/Exceptions.h"
#include "ibrdtn/data/BundleString.h"
#include <netinet/in.h>
#include <typeinfo>

using namespace dtn::data;

namespace dtn
{
	namespace streams
	{
		StreamContactHeader::StreamContactHeader()
		 : _flags(1), _keepalive(0)
		{
		}

		StreamContactHeader::StreamContactHeader(EID localeid)
		 : _localeid(localeid), _flags(REQUEST_ACKNOWLEDGMENTS), _keepalive(0)
		{
		}

		StreamContactHeader::~StreamContactHeader()
		{
		}

		StreamContactHeader& StreamContactHeader::operator=(const StreamContactHeader &other)
		{
			_localeid = other._localeid;
			_flags = other._flags;
			_keepalive = other._keepalive;
			return *this;
		}

		const EID StreamContactHeader::getEID() const
		{
			return _localeid;
		}

		std::ostream &operator<<(std::ostream &stream, const StreamContactHeader &h)
		{
			//BundleStreamWriter writer(stream);
			stream << "dtn!" << TCPCL_VERSION << h._flags;

			// convert to network byte order
			u_int16_t ka = htons(h._keepalive);
			stream.write( (char*)&ka, 2 );

			dtn::data::BundleString eid(h._localeid.getString());
			stream << eid;

			return stream;
		}

		std::istream &operator>>(std::istream &stream, StreamContactHeader &h)
		{
			char magic[5];

			// wait for magic
			stream.read(magic, 4); magic[4] = '\0';
			string str_magic(magic);

			if (str_magic != "dtn!")
			{
				throw InvalidProtocolException("not talking dtn");
			}

			// version
			char version; stream.get(version);
			if (version != TCPCL_VERSION)
			{
				throw InvalidProtocolException("invalid bundle protocol version");
			}

			// flags
			stream.get(h._flags);

			u_int16_t ka = 0;
			stream.read((char*)&ka, 2);

			// convert from network byte order
			h._keepalive = ntohs(ka);

			dtn::data::BundleString eid;
			stream >> eid;
			h._localeid = EID(eid);

			return stream;
		}
	}
}
