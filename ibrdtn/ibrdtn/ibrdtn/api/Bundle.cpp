/*
 * Bundle.cpp
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

#include "ibrdtn/api/Bundle.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/Exceptions.h"

namespace dtn
{
	namespace api
	{
		Bundle::Bundle()
		{
			setPriority(PRIO_MEDIUM);
			_b._source = dtn::data::EID("api:me");
		}

		Bundle::Bundle(const dtn::data::Bundle &b)
		 : _b(b)
		{
		}

		Bundle::Bundle(const dtn::data::EID &destination)
		{
			setDestination(destination);
			setPriority(PRIO_MEDIUM);
			_b._source = dtn::data::EID("api:me");
		}

		Bundle::~Bundle()
		{
		}

		void Bundle::setSingleton(bool val)
		{
			_b.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, val);
		}

		void Bundle::setLifetime(unsigned int lifetime)
		{
			_b._lifetime = lifetime;
		}

		unsigned int Bundle::getLifetime() const
		{
			return _b._lifetime;
		}

		time_t Bundle::getTimestamp() const
		{
			return _b._timestamp;
		}

		std::string Bundle::toString() const
		{
			return _b.toString();
		}

		void Bundle::requestDeliveredReport()
		{
			_b.set(dtn::data::PrimaryBlock::REQUEST_REPORT_OF_BUNDLE_DELIVERY, true);
		}

		void Bundle::requestForwardedReport()
		{
			_b.set(dtn::data::PrimaryBlock::REQUEST_REPORT_OF_BUNDLE_FORWARDING, true);
		}

		void Bundle::requestDeletedReport()
		{
			_b.set(dtn::data::PrimaryBlock::REQUEST_REPORT_OF_BUNDLE_DELETION, true);
		}

		void Bundle::requestReceptionReport()
		{
			_b.set(dtn::data::PrimaryBlock::REQUEST_REPORT_OF_BUNDLE_RECEPTION, true);
		}

		void Bundle::requestCustodyTransfer()
		{
			_b.set(dtn::data::PrimaryBlock::CUSTODY_REQUESTED, true);
			_b._custodian = dtn::data::EID("api:me");
		}

		void Bundle::requestEncryption()
		{
			_b.set(dtn::data::PrimaryBlock::DTNSEC_REQUEST_ENCRYPT, true);
		}

		void Bundle::requestSigned()
		{
			_b.set(dtn::data::PrimaryBlock::DTNSEC_REQUEST_SIGN, true);
		}

		void Bundle::requestCompression()
		{
			_b.set(dtn::data::PrimaryBlock::IBRDTN_REQUEST_COMPRESSION, true);
		}

		bool Bundle::statusVerified()
		{
			return _b.get(dtn::data::PrimaryBlock::DTNSEC_STATUS_VERIFIED);
		}

		Bundle::BUNDLE_PRIORITY Bundle::getPriority() const
		{
			if (_b.get(dtn::data::Bundle::PRIORITY_BIT1))
			{
				return PRIO_MEDIUM;
			}

			if (_b.get(dtn::data::Bundle::PRIORITY_BIT2))
			{
				return PRIO_HIGH;
			}

			return PRIO_LOW;
		}

		void Bundle::setPriority(Bundle::BUNDLE_PRIORITY p)
		{
			// set the priority to the real bundle
			switch (p)
			{
			case PRIO_LOW:
				_b.set(dtn::data::Bundle::PRIORITY_BIT1, false);
				_b.set(dtn::data::Bundle::PRIORITY_BIT2, false);
				break;

			case PRIO_HIGH:
				_b.set(dtn::data::Bundle::PRIORITY_BIT1, false);
				_b.set(dtn::data::Bundle::PRIORITY_BIT2, true);
				break;

			case PRIO_MEDIUM:
				_b.set(dtn::data::Bundle::PRIORITY_BIT1, true);
				_b.set(dtn::data::Bundle::PRIORITY_BIT2, false);
				break;
			}
		}

		void Bundle::setDestination(const dtn::data::EID &eid, const bool singleton)
		{
			if (singleton)
				_b.set(dtn::data::Bundle::DESTINATION_IS_SINGLETON, true);
			else
				_b.set(dtn::data::Bundle::DESTINATION_IS_SINGLETON, false);

			_b._destination = eid;
		}

		void Bundle::setReportTo(const dtn::data::EID &eid)
		{
			_b._reportto = eid;
		}

		dtn::data::EID Bundle::getReportTo() const
		{
			return _b._reportto;
		}

		dtn::data::EID Bundle::getDestination() const
		{
			return _b._destination;
		}

		dtn::data::EID Bundle::getSource() const
		{
			return _b._source;
		}

		dtn::data::EID Bundle::getCustodian() const
		{
			return _b._custodian;
		}

		ibrcommon::BLOB::Reference Bundle::getData() const throw (dtn::MissingObjectException)
		{
			try {
				const dtn::data::PayloadBlock &p = _b.getBlock<dtn::data::PayloadBlock>();
				return p.getBLOB();
			} catch(const dtn::data::Bundle::NoSuchBlockFoundException&) {
				throw dtn::MissingObjectException("No payload block exists!");
			}
		}

		bool Bundle::operator<(const Bundle& other) const
		{
			return (_b < other._b);
		}

		bool Bundle::operator>(const Bundle& other) const
		{
			return (_b > other._b);
		}
	}
}
