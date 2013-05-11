/*
 * RoutingExtension.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "routing/RoutingExtension.h"
#include "routing/BaseRouter.h"
#include "core/BundleCore.h"
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace routing
	{
		BaseRouter *RoutingExtension::_router = NULL;
		const std::string RoutingExtension::TAG = "RoutingExtension";

		/**
		 * base implementation of the Extension class
		 */
		RoutingExtension::RoutingExtension()
		{ }

		RoutingExtension::~RoutingExtension()
		{ }

		BaseRouter& RoutingExtension::operator*()
		{
			return *_router;
		}

		/**
		 * Transfer one bundle to another node.
		 * @param destination The EID of the other node.
		 * @param id The ID of the bundle to transfer. This bundle must be stored in the storage.
		 */
		void RoutingExtension::transferTo(const dtn::data::EID &destination, const dtn::data::MetaBundle &meta)
		{
			// lock the list of neighbors
			ibrcommon::MutexLock l(_router->getNeighborDB());

			// get the neighbor entry for the next hop
			NeighborDatabase::NeighborEntry &entry = _router->getNeighborDB().get(destination);

			// transfer bundle to the neighbor
			transferTo(entry, meta);
		}

		void RoutingExtension::transferTo(NeighborDatabase::NeighborEntry &entry, const dtn::data::MetaBundle &meta)
		{
			// acquire the transfer of this bundle, could throw already in transit or no resource left exception
			entry.acquireTransfer(meta);

			try {
				// TODO: switch to new method for checking the amount of ongoing transfers

				dtn::net::BundleTransfer transfer(entry.eid, meta);

				// transfer the bundle to the next hop
				dtn::core::BundleCore::getInstance().transferTo(transfer);

				IBRCOMMON_LOGGER_DEBUG_TAG(RoutingExtension::TAG, 20) << "bundle " << meta.toString() << " queued for " << entry.eid.getString() << IBRCOMMON_LOGGER_ENDL;
			} catch (const dtn::core::P2PDialupException&) {
				// release the transfer
				entry.releaseTransfer(meta);

				// and abort the query
				throw NeighborDatabase::NeighborNotAvailableException();
			}
		}

		bool RoutingExtension::isRouting(const dtn::data::EID &eid)
		{
			if (eid.getApplication() == "routing")
			{
				return true;
			}
			else if ((eid.getScheme() == dtn::data::EID::CBHE_SCHEME) && (eid.getApplication() == "50"))
			{
				return true;
			}

			return false;
		}

	} /* namespace routing */
} /* namespace dtn */
