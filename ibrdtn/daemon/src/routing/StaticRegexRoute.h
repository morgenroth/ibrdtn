/*
 * StaticRegexRoute.h
 *
 *  Created on: 28.11.2012
 *      Author: morgenro
 */

#ifndef STATICREGEXROUTE_H_
#define STATICREGEXROUTE_H_

#include "routing/StaticRoute.h"
#include <regex.h>

namespace dtn
{
	namespace routing
	{
		class StaticRegexRoute : public StaticRoute {
		public:
			StaticRegexRoute(const std::string &regex, const dtn::data::EID &dest);
			virtual ~StaticRegexRoute();

			bool match(const dtn::data::EID &eid) const;
			const dtn::data::EID& getDestination() const;
			const dtn::data::Timestamp& getExpiration() const;

			/**
			 * Raise the StaticRouteChangeEvent for expiration
			 */
			void raiseExpired() const;

			/**
			 * Compare this static route with another one
			 */
			bool equals(const StaticRoute &route) const;

			/**
			 * copy and assignment operators
			 * @param obj The object to copy
			 * @return
			 */
			StaticRegexRoute(const StaticRegexRoute &obj);
			StaticRegexRoute& operator=(const StaticRegexRoute &obj);

			/**
			 * Describe this route as a one-line-string.
			 * @return
			 */
			const std::string toString() const;

		private:
			dtn::data::EID _dest;
			std::string _regex_str;
			regex_t _regex;
			bool _invalid;
			const dtn::data::Timestamp _expire;
		};
	}
}

#endif /* STATICREGEXROUTE_H_ */
