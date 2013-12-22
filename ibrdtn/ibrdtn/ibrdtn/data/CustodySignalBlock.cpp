/*
 * CustodySignalBlock.cpp
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

#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/CustodySignalBlock.h"
#include "ibrdtn/data/BundleString.h"
#include <stdlib.h>
#include <sstream>

namespace dtn
{
	namespace data
	{
		CustodySignalBlock::CustodySignalBlock()
		 : AdministrativeBlock(32), custody_accepted(false), reason(NO_ADDITIONAL_INFORMATION),
		 fragment_length(0), timeofsignal()
		{
		}

		CustodySignalBlock::~CustodySignalBlock()
		{
		}

		void CustodySignalBlock::read(const dtn::data::PayloadBlock &p) throw (WrongRecordException)
		{
			ibrcommon::BLOB::Reference r = p.getBLOB();
			ibrcommon::BLOB::iostream stream = r.iostream();
			(*stream).get(_admfield);

			// check type field
			if ((_admfield >> 4) != 2) throw WrongRecordException();

			char status = 0;
			(*stream).get(status);

			// decode custody acceptance
			custody_accepted = (status & 0x01);

			// decode reason flag
			reason = REASON_CODE(status >> 1);

			if ( refsFragment() )
			{
				bundleid.setFragment(true);

				(*stream) >> bundleid.fragmentoffset;
				(*stream) >> fragment_length;
			}

			(*stream) >> timeofsignal;
			(*stream) >> bundleid.timestamp;
			(*stream) >> bundleid.sequencenumber;

			BundleString source;
			(*stream) >> source;
			bundleid.source = EID(source);
		}

		void CustodySignalBlock::write(dtn::data::PayloadBlock &p) const
		{
			ibrcommon::BLOB::Reference r = p.getBLOB();
			ibrcommon::BLOB::iostream stream = r.iostream();

			// clear the whole data first
			stream.clear();

			// write the content
			(*stream).put(_admfield);

			// encode reason flag
			char status = static_cast<char>(reason << 1);

			// encode custody acceptance
			if (custody_accepted) status |= 0x01;

			// write the status byte
			(*stream).put(status);

			if ( refsFragment() )
			{
				(*stream) << bundleid.fragmentoffset;
				(*stream) << fragment_length;
			}

			BundleString sourceid(bundleid.source.getString());

			(*stream) << timeofsignal
			   << bundleid.timestamp
			   << bundleid.sequencenumber
			   << sourceid;
		}

		void CustodySignalBlock::setMatch(const dtn::data::MetaBundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				bundleid.fragmentoffset = other.fragmentoffset;
				fragment_length = other.appdatalength;

				bundleid.setFragment(true);
			}

			bundleid.timestamp = other.timestamp;
			bundleid.sequencenumber = other.sequencenumber;
			bundleid.source = other.source;
		}

		void CustodySignalBlock::setMatch(const dtn::data::Bundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				bundleid.fragmentoffset = other.fragmentoffset;
				fragment_length = other.appdatalength;

				bundleid.setFragment(true);
			}

			bundleid.timestamp = other.timestamp;
			bundleid.sequencenumber = other.sequencenumber;
			bundleid.source = other.source;
		}

		bool CustodySignalBlock::match(const Bundle& other) const
		{
			if (bundleid.timestamp != other.timestamp) return false;
			if (bundleid.sequencenumber != other.sequencenumber) return false;
			if (bundleid.source != other.source) return false;

			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				if (!bundleid.isFragment()) return false;
				if (bundleid.fragmentoffset != other.fragmentoffset) return false;
				if (fragment_length != other.appdatalength) return false;
			}

			return true;
		}

	}
}
