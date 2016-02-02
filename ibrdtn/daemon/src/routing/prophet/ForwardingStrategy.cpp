/*
 * ForwardingStrategy.cpp
 *
 *  Created on: 11.01.2013
 *      Author: morgenro
 */

#include "routing/prophet/ForwardingStrategy.h"
#include "routing/prophet/ProphetRoutingExtension.h"
#include "routing/prophet/DeliveryPredictabilityMap.h"

namespace dtn
{
	namespace routing
	{
		ForwardingStrategy::ForwardingStrategy()
		 : _prophet_router(NULL)
		{}

		ForwardingStrategy::~ForwardingStrategy()
		{}

		bool ForwardingStrategy::neighborDPIsGreater(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::EID& destination) const
		{
			const dtn::data::EID destnode = destination.getNode();

			try {
				float local_pv = 0.0f;

				// get the local value
				{
					ibrcommon::MutexLock dpm_lock(_prophet_router->_deliveryPredictabilityMap);
					const DeliveryPredictabilityMap& dp_map = _prophet_router->_deliveryPredictabilityMap;
					local_pv = dp_map.get(destnode);
				}

				try {
					// retrieve the value from the DeliveryPredictabilityMap of the neighbor
					float foreign_pv = neighbor_dpm.get(destnode);

					return (foreign_pv > local_pv);
				} catch (const DeliveryPredictabilityMap::ValueNotFoundException&) {
					// if the foreign router has no entry for the destination
					// then compare the local value with a fictitious initial value
					return (_prophet_router->_p_first_threshold > local_pv);
				}
			} catch (const DeliveryPredictabilityMap::ValueNotFoundException&) {
				// always forward if the destination is not in our own predictability map
			}

			return false;
		}

		bool ForwardingStrategy::isBackrouteValid(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::EID& source) const
		{
			ibrcommon::MutexLock dpm_lock(_prophet_router->_deliveryPredictabilityMap);

			// check if we know a way to the source
			try {
				if (_prophet_router->_deliveryPredictabilityMap.get(source.getNode()) > 0.0) return true;
			} catch (const dtn::routing::DeliveryPredictabilityMap::ValueNotFoundException&) { }

			// check if the peer know a way to the source
			try {
				if (neighbor_dpm.get(source.getNode()) > 0.0) return true;
			} catch (const dtn::routing::DeliveryPredictabilityMap::ValueNotFoundException&) { }

			return false;
		}

		void ForwardingStrategy::setProphetRouter(ProphetRoutingExtension *router)
		{
			_prophet_router = router;
		}
	} /* namespace routing */
} /* namespace dtn */
