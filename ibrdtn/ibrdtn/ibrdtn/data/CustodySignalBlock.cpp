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
		 : AdministrativeBlock(32), _custody_accepted(false), _reason(NO_ADDITIONAL_INFORMATION),
		 _fragment_length(0), _timeofsignal()
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
			_custody_accepted = (status & 0x01);

			// decode reason flag
			_reason = REASON_CODE(status >> 1);

			if ( refsFragment() )
			{
				_bundleid.fragment = true;

				dtn::data::SDNV frag_offset;
				(*stream) >> frag_offset;
				_bundleid.offset = frag_offset.getValue();

				(*stream) >> _fragment_length;
			}

			(*stream) >> _timeofsignal;

			dtn::data::SDNV timestamp;
			(*stream) >> timestamp;
			_bundleid.timestamp = timestamp.getValue();

			dtn::data::SDNV seqno;
			(*stream) >> seqno;
			_bundleid.sequencenumber = seqno.getValue();

			BundleString source;
			(*stream) >> source;
			_bundleid.source = EID(source);
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
			char status = (_reason << 1);

			// encode custody acceptance
			if (_custody_accepted) status |= 0x01;

			// write the status byte
			(*stream).put(status);

			if ( refsFragment() )
			{
				(*stream) << dtn::data::SDNV(_bundleid.offset);
				(*stream) << _fragment_length;
			}

			BundleString sourceid(_bundleid.source.getString());

			(*stream) << _timeofsignal
			   << dtn::data::SDNV(_bundleid.timestamp)
			   << dtn::data::SDNV(_bundleid.sequencenumber)
			   << sourceid;
		}

		void CustodySignalBlock::setMatch(const dtn::data::MetaBundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				_bundleid.offset = other.offset;
				_fragment_length = other.appdatalength;

				setFragment(true);
				_bundleid.fragment = true;
			}

			_bundleid.timestamp = other.timestamp;
			_bundleid.sequencenumber = other.sequencenumber;
			_bundleid.source = other.source;
		}

		void CustodySignalBlock::setMatch(const dtn::data::Bundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				_bundleid.offset = other.fragmentoffset;
				_fragment_length = other.appdatalength;

				setFragment(true);
				_bundleid.fragment = true;
			}

			_bundleid.timestamp = other.timestamp;
			_bundleid.sequencenumber = other.sequencenumber;
			_bundleid.source = other.source;
		}

		bool CustodySignalBlock::match(const Bundle& other) const
		{
			if (_bundleid.timestamp != other.timestamp) return false;
			if (_bundleid.sequencenumber != other.sequencenumber) return false;
			if (_bundleid.source != other.source) return false;

			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				if (!_bundleid.fragment) return false;
				if (_bundleid.offset != other.fragmentoffset) return false;
				if (_fragment_length != other.appdatalength) return false;
			}

			return true;
		}

	}
}
