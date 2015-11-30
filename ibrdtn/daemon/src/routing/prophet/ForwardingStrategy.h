/*
 * ForwardingStrategy.h
 *
 *  Created on: 11.01.2013
 *      Author: morgenro
 */

#ifndef FORWARDINGSTRATEGY_H_
#define FORWARDINGSTRATEGY_H_

#include "routing/prophet/DeliveryPredictabilityMap.h"
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/EID.h>

namespace dtn
{
	namespace routing
	{
		class ProphetRoutingExtension;

		/*!
		 * \brief This class is a abstract base class for all prophet forwarding strategies.
		 */
		class ForwardingStrategy
		{
		public:
			ForwardingStrategy();
			virtual ~ForwardingStrategy() = 0;

			/*!
			 * The prophetRoutingExtension calls this function for every bundle that can be forwarded to a neighbor
			 * and forwards it depending on the return value.
			 * \param neighbor the neighbor to forward to
			 * \param bundle the bundle that can be forwarded
			 * \param prophet_router Reference to the ProphetRoutingExtension to access its parameters
			 * \return true if the bundle should be forwarded
			 */
			virtual bool shallForward(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::MetaBundle& bundle) const = 0;

			/*!
			 * checks if the deliveryPredictability of the neighbor is higher than that of the destination of the bundle.
			 */
			bool neighborDPIsGreater(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::EID& destination) const;

			/*!
			 * checks if a backwards route exists to the source
			 */
			bool isBackrouteValid(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::EID& source) const;

			/**
			 * Set back-reference to the prophet router
			 */
			void setProphetRouter(ProphetRoutingExtension *router);

		protected:
			ProphetRoutingExtension *_prophet_router;
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* FORWARDINGSTRATEGY_H_ */
