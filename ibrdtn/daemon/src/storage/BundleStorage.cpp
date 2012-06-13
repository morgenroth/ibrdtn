/*
 * BundleStorage.cpp
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

#include "core/BundleCore.h"
#include "storage/BundleStorage.h"
#include "core/CustodyEvent.h"
#include "core/BundleGeneratedEvent.h"
#include <ibrdtn/data/BundleID.h>

namespace dtn
{
	namespace storage
	{
		BundleStorage::BundleStorage()
		{
		}

		BundleStorage::~BundleStorage()
		{
		}

		void BundleStorage::remove(const dtn::data::Bundle &b)
		{
			remove(dtn::data::BundleID(b));
		};

		dtn::data::MetaBundle BundleStorage::remove(const ibrcommon::BloomFilter&)
		{
			throw dtn::storage::BundleStorage::NoBundleFoundException();
		};

		const dtn::data::EID BundleStorage::acceptCustody(const dtn::data::MetaBundle &meta)
		{
			if (!meta.get(Bundle::CUSTODY_REQUESTED))
				throw ibrcommon::Exception("custody transfer is not requested for this bundle.");

			if (meta.custodian == EID())
				throw ibrcommon::Exception("no previous custodian is set.");

			// create a new bundle
			Bundle custody_bundle;

			// set priority to HIGH
			custody_bundle.set(dtn::data::PrimaryBlock::PRIORITY_BIT1, false);
			custody_bundle.set(dtn::data::PrimaryBlock::PRIORITY_BIT2, true);

			// send a custody signal with accept flag
			CustodySignalBlock &signal = custody_bundle.push_back<CustodySignalBlock>();

			// set the bundle to match
			signal.setMatch(meta);

			// set accepted
			signal._custody_accepted = true;

			custody_bundle.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);
			custody_bundle._destination = meta.custodian;
			custody_bundle._source = dtn::core::BundleCore::local;

			// send the custody accepted bundle
			dtn::core::BundleGeneratedEvent::raise(custody_bundle);

			// raise the custody accepted event
			dtn::core::CustodyEvent::raise(meta, dtn::core::CUSTODY_ACCEPT);

			return dtn::core::BundleCore::local;
		}

		void BundleStorage::rejectCustody(const dtn::data::MetaBundle &meta, dtn::data::CustodySignalBlock::REASON_CODE reason)
		{
			if (!meta.get(Bundle::CUSTODY_REQUESTED))
				throw ibrcommon::Exception("custody transfer is not requested for this bundle.");

			if (meta.custodian == EID())
				throw ibrcommon::Exception("no previous custodian is set.");

			// create a new bundle
			Bundle b;

			// send a custody signal with reject flag
			CustodySignalBlock &signal = b.push_back<CustodySignalBlock>();

			// set the bundle to match
			signal.setMatch(meta);

			// set reason code
			signal._reason = reason;

			b._destination = meta.custodian;
			b._source = dtn::core::BundleCore::local;

			// send the custody rejected bundle
			dtn::core::BundleGeneratedEvent::raise(b);

			// raise the custody rejected event
			dtn::core::CustodyEvent::raise(b, dtn::core::CUSTODY_REJECT);
		}
	}
}
