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
		 : _admfield(32), _custody_accepted(false), _reason(NO_ADDITIONAL_INFORMATION), _fragment_offset(0),
		 _fragment_length(0), _timeofsignal(), _bundle_timestamp(0), _bundle_sequence(0)
		{
		}

		CustodySignalBlock::~CustodySignalBlock()
		{
		}

		void CustodySignalBlock::read(const dtn::data::PayloadBlock &p) throw (WrongRecordException)
		{
			ibrcommon::BLOB::Reference r = p.getBLOB();
			ibrcommon::BLOB::iostream stream = r.iostream();
			(*stream) >> _admfield;

			// check type field
			if ((_admfield >> 4) != 2) throw WrongRecordException();

			char status; (*stream) >> status;

			// decode custody acceptance
			_custody_accepted = (status & 0x01);

			// decode reason flag
			_reason = REASON_CODE(status >> 1);

			if ( _admfield & 0x01 )
			{
				(*stream) >> _fragment_offset;
				(*stream) >> _fragment_length;
			}

			(*stream) >> _timeofsignal;
			(*stream) >> _bundle_timestamp;
			(*stream) >> _bundle_sequence;

			BundleString source;
			(*stream) >> source;
			_source = EID(source);
		}

		void CustodySignalBlock::write(dtn::data::PayloadBlock &p) const
		{
			ibrcommon::BLOB::Reference r = p.getBLOB();
			ibrcommon::BLOB::iostream stream = r.iostream();

			// clear the whole data first
			stream.clear();

			// write the content
			(*stream) << _admfield;

			// encode reason flag
			char status = (_reason << 1);

			// encode custody acceptance
			if (_custody_accepted) status |= 0x01;

			// write the status byte
			(*stream) << status;

			if ( _admfield & 0x01 )
			{
				(*stream) << _fragment_offset;
				(*stream) << _fragment_length;
			}

			BundleString sourceid(_source.getString());

			(*stream) << _timeofsignal
			   << _bundle_timestamp
			   << _bundle_sequence
			   << sourceid;
		}

		void CustodySignalBlock::setMatch(const dtn::data::MetaBundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				_fragment_offset = other.offset;
				_fragment_length = other.appdatalength;

				if (!(_admfield & 1)) _admfield += 1;
			}

			_bundle_timestamp = other.timestamp;
			_bundle_sequence = other.sequencenumber;
			_source = other.source;
		}

		void CustodySignalBlock::setMatch(const dtn::data::Bundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				_fragment_offset = other._fragmentoffset;
				_fragment_length = other._appdatalength;

				if (!(_admfield & 1)) _admfield += 1;
			}

			_bundle_timestamp = other._timestamp;
			_bundle_sequence = other._sequencenumber;
			_source = other._source;
		}

		bool CustodySignalBlock::match(const Bundle& other) const
		{
			if (_bundle_timestamp != other._timestamp) return false;
			if (_bundle_sequence != other._sequencenumber) return false;
			if (_source != other._source) return false;

			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				if (!(_admfield & 1)) return false;
				if (_fragment_offset != other._fragmentoffset) return false;
				if (_fragment_length != other._appdatalength) return false;
			}

			return true;
		}

	}
}
