/*
 * StaticRoute.h
 *
 *  Created on: 28.11.2012
 *      Author: morgenro
 */

#ifndef STATICROUTE_H_
#define STATICROUTE_H_

#include <ibrdtn/data/EID.h>
#include <string>

namespace dtn
{
	namespace routing
	{
		class StaticRoute
		{
		public:
			virtual ~StaticRoute() = 0;
			virtual bool match(const dtn::data::EID &eid) const = 0;
			virtual const dtn::data::EID& getDestination() const = 0;
			virtual const std::string toString() const = 0;
			virtual const dtn::data::Timestamp& getExpiration() const = 0;

			/**
			 * Raise the StaticRouteChangeEvent for expiration
			 */
			virtual void raiseExpired() const = 0;

			/**
			 * Compare this static route with another one
			 */
			virtual bool equals(const StaticRoute &route) const = 0;
		};
	}
}

#endif /* STATICROUTE_H_ */
