/*
 * StatusReportBlock.cpp
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

#include "ibrdtn/data/StatusReportBlock.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/BundleString.h"
#include "ibrdtn/data/PayloadBlock.h"
#include <stdlib.h>
#include <sstream>

namespace dtn
{
	namespace data
	{
		StatusReportBlock::StatusReportBlock()
		 : AdministrativeBlock(16), _status(0), _reasoncode(0),
		 _fragment_offset(0), _fragment_length(0), _timeof_receipt(),
		 _timeof_custodyaccept(), _timeof_forwarding(), _timeof_delivery(),
		 _timeof_deletion(), _bundle_timestamp(0), _bundle_sequence(0)
		{
		}

		StatusReportBlock::~StatusReportBlock()
		{
		}

		void StatusReportBlock::write(dtn::data::PayloadBlock &p) const
		{
			ibrcommon::BLOB::Reference r = p.getBLOB();
			ibrcommon::BLOB::iostream stream = r.iostream();

			// clear the whole data first
			stream.clear();

			(*stream) << _admfield;
			(*stream) << _status;
			(*stream) << _reasoncode;

			if ( refsFragment() )
			{
				(*stream) << _fragment_offset;
				(*stream) << _fragment_length;
			}

			if (_status & RECEIPT_OF_BUNDLE)
				(*stream) << _timeof_receipt;

			if (_status & CUSTODY_ACCEPTANCE_OF_BUNDLE)
				(*stream) << _timeof_custodyaccept;

			if (_status & FORWARDING_OF_BUNDLE)
				(*stream) << _timeof_forwarding;

			if (_status & DELIVERY_OF_BUNDLE)
				(*stream) << _timeof_delivery;

			if (_status & DELETION_OF_BUNDLE)
				(*stream) << _timeof_deletion;

			(*stream) << _bundle_timestamp;
			(*stream) << _bundle_sequence;
			(*stream) << BundleString(_source.getString());
		}

		void StatusReportBlock::read(const dtn::data::PayloadBlock &p) throw (WrongRecordException)
		{
			ibrcommon::BLOB::Reference r = p.getBLOB();
			ibrcommon::BLOB::iostream stream = r.iostream();

			(*stream) >> _admfield;

			// check type field
			if ((_admfield >> 4) != 1) throw WrongRecordException();

			(*stream) >> _status;
			(*stream) >> _reasoncode;

			if ( refsFragment() )
			{
				(*stream) >> _fragment_offset;
				(*stream) >> _fragment_length;
			}

			if (_status & RECEIPT_OF_BUNDLE)
				(*stream) >> _timeof_receipt;

			if (_status & CUSTODY_ACCEPTANCE_OF_BUNDLE)
				(*stream) >> _timeof_custodyaccept;

			if (_status & FORWARDING_OF_BUNDLE)
				(*stream) >> _timeof_forwarding;

			if (_status & DELIVERY_OF_BUNDLE)
				(*stream) >> _timeof_delivery;

			if (_status & DELETION_OF_BUNDLE)
				(*stream) >> _timeof_deletion;

			(*stream) >> _bundle_timestamp;
			(*stream) >> _bundle_sequence;

			BundleString source;
			(*stream) >> source;
			_source = EID(source);
		}

	}
}
