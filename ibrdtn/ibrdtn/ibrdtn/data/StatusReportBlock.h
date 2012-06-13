/*
 * StatusReportBlock.h
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

#ifndef STATUSREPORTBLOCK_H_
#define STATUSREPORTBLOCK_H_

#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/EID.h"
#include "ibrcommon/data/BLOB.h"
#include "ibrdtn/data/DTNTime.h"

namespace dtn
{
	namespace data
	{
		class StatusReportBlock : public Block
		{
		public:
			enum TYPE
			{
				RECEIPT_OF_BUNDLE = 1 << 0,
				CUSTODY_ACCEPTANCE_OF_BUNDLE = 1 << 1,
				FORWARDING_OF_BUNDLE = 1 << 2,
				DELIVERY_OF_BUNDLE = 1 << 3,
				DELETION_OF_BUNDLE = 1 << 4
			};

			enum REASON_CODE
			{
				NO_ADDITIONAL_INFORMATION = 0x00,
				LIFETIME_EXPIRED = 0x01,
				FORWARDED_OVER_UNIDIRECTIONAL_LINK = 0x02,
				TRANSMISSION_CANCELED = 0x03,
				DEPLETED_STORAGE = 0x04,
				DESTINATION_ENDPOINT_ID_UNINTELLIGIBLE = 0x05,
				NO_KNOWN_ROUTE_TO_DESTINATION_FROM_HERE = 0x06,
				NO_TIMELY_CONTACT_WITH_NEXT_NODE_ON_ROUTE = 0x07,
				BLOCK_UNINTELLIGIBLE = 0x08
			};

			StatusReportBlock();
			virtual ~StatusReportBlock();

			virtual size_t getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const;
			virtual std::istream &deserialize(std::istream &stream, const size_t length);

			char _admfield;
			char _status;
			char _reasoncode;
			SDNV _fragment_offset;
			SDNV _fragment_length;
			DTNTime _timeof_receipt;
			DTNTime _timeof_custodyaccept;
			DTNTime _timeof_forwarding;
			DTNTime _timeof_delivery;
			DTNTime _timeof_deletion;
			SDNV _bundle_timestamp;
			SDNV _bundle_sequence;
			EID _source;
		};
	}
}

#endif /* STATUSREPORTBLOCK_H_ */
