/*
 * StaticRouteChangeEvent.cpp
 *
 *  Created on: 08.02.2012
 *      Author: morgenro
 */

#include "routing/StaticRouteChangeEvent.h"
#include <sstream>

namespace dtn
{
	namespace routing
	{
		StaticRouteChangeEvent::StaticRouteChangeEvent(CHANGE_TYPE t)
		 : type(t)
		{
		}

		StaticRouteChangeEvent::StaticRouteChangeEvent(CHANGE_TYPE t, const dtn::data::EID &n, const std::string p, size_t to)
		 : type(t), nexthop(n), pattern(p), timeout(to)
		{
		}

		StaticRouteChangeEvent::StaticRouteChangeEvent(CHANGE_TYPE t, const dtn::data::EID &n, const dtn::data::EID &d, size_t to)
		 : type(t), nexthop(n), destination(d), timeout(to)
		{
		}

		StaticRouteChangeEvent::~StaticRouteChangeEvent()
		{
		}

		const std::string StaticRouteChangeEvent::getName() const
		{
			return StaticRouteChangeEvent::className;
		}

		std::string StaticRouteChangeEvent::toString() const
		{
			std::stringstream ss;
			ss << getName() << ": ";

			switch (type)
			{
			case ROUTE_ADD:
				ss << "add route ";

				if (pattern.length() > 0)
				{
					ss << pattern;
				}
				else
				{
					ss << destination.getString();
				}

				ss << " => " << nexthop.getString();
				if (timeout > 0) ss << "; timeout = " << timeout;
				break;
			case ROUTE_DEL:
				ss << "del route ";

				if (pattern.length() > 0)
				{
					ss << pattern;
				}
				else
				{
					ss << destination.getString();
				}

				ss << " => " << nexthop.getString();
				break;
			case ROUTE_EXPIRED:
				ss << "route expired";
				break;
			case ROUTE_CLEAR:
				ss << "routes cleared";
				break;
			default:
				ss << "unknown";
				break;
			}

			return ss.str();
		}

		void StaticRouteChangeEvent::raiseEvent(CHANGE_TYPE t)
		{
			dtn::core::Event::raiseEvent( new StaticRouteChangeEvent(t) );
		}

		void StaticRouteChangeEvent::raiseEvent(CHANGE_TYPE t, const dtn::data::EID &n, const std::string p, size_t to)
		{
			dtn::core::Event::raiseEvent( new StaticRouteChangeEvent(t, n, p, to) );
		}

		void StaticRouteChangeEvent::raiseEvent(CHANGE_TYPE t, const dtn::data::EID &n, const dtn::data::EID &d, size_t to)
		{
			dtn::core::Event::raiseEvent( new StaticRouteChangeEvent(t, n, d, to) );
		}

		const string StaticRouteChangeEvent::className = "StaticRouteChangeEvent";
	} /* namespace routing */
} /* namespace dtn */
