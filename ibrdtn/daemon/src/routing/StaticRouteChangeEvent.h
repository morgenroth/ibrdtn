/*
 * StaticRouteChangeEvent.h
 *
 *  Created on: 08.02.2012
 *      Author: morgenro
 */

#ifndef STATICROUTECHANGEEVENT_H_
#define STATICROUTECHANGEEVENT_H_

#include "core/Event.h"
#include <ibrdtn/data/EID.h>

namespace dtn
{
	namespace routing
	{
		class StaticRouteChangeEvent : public dtn::core::Event
		{
		public:
			enum CHANGE_TYPE
			{
				ROUTE_ADD = 0,
				ROUTE_DEL = 1,
				ROUTE_EXPIRED = 2,
				ROUTE_CLEAR = 3
			};

			virtual ~StaticRouteChangeEvent();

			const std::string getName() const;

			std::string toString() const;

			static void raiseEvent(CHANGE_TYPE type);
			static void raiseEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const std::string pattern, size_t timeout = 0);
			static void raiseEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const dtn::data::EID &destination, size_t timeout = 0);

			CHANGE_TYPE type;
			const dtn::data::EID nexthop;
			const dtn::data::EID destination;
			const std::string pattern;
			size_t timeout;

			static const std::string className;

		private:
			StaticRouteChangeEvent(CHANGE_TYPE type);
			StaticRouteChangeEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const std::string pattern, size_t timeout = 0);
			StaticRouteChangeEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const dtn::data::EID &destination, size_t timeout = 0);
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* STATICROUTECHANGEEVENT_H_ */
