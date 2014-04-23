/*
 * StaticRouteChangeEvent.cpp
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

#include "routing/StaticRouteChangeEvent.h"
#include "core/EventDispatcher.h"
#include <sstream>

namespace dtn
{
	namespace routing
	{
		StaticRouteChangeEvent::StaticRouteChangeEvent(CHANGE_TYPE t)
		 : type(t), nexthop(), pattern(), timeout(0)
		{
		}

		StaticRouteChangeEvent::StaticRouteChangeEvent(CHANGE_TYPE t, const dtn::data::EID &n, const std::string &p, const dtn::data::Number &to)
		 : type(t), nexthop(n), pattern(p), timeout(to)
		{
		}

		StaticRouteChangeEvent::StaticRouteChangeEvent(CHANGE_TYPE t, const dtn::data::EID &n, const dtn::data::EID &d, const dtn::data::Number &to)
		 : type(t), nexthop(n), destination(d), timeout(to)
		{
		}

		StaticRouteChangeEvent::~StaticRouteChangeEvent()
		{
		}

		const std::string StaticRouteChangeEvent::getName() const
		{
			return "StaticRouteChangeEvent";
		}

		std::string StaticRouteChangeEvent::getMessage() const
		{
			std::stringstream ss;

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
				if (timeout > 0) ss << "; timeout = " << timeout.toString();
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
				ss << "route expired ";

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
			dtn::core::EventDispatcher<StaticRouteChangeEvent>::queue( new StaticRouteChangeEvent(t) );
		}

		void StaticRouteChangeEvent::raiseEvent(CHANGE_TYPE t, const dtn::data::EID &n, const std::string &p, const dtn::data::Number &to)
		{
			dtn::core::EventDispatcher<StaticRouteChangeEvent>::queue( new StaticRouteChangeEvent(t, n, p, to) );
		}

		void StaticRouteChangeEvent::raiseEvent(CHANGE_TYPE t, const dtn::data::EID &n, const dtn::data::EID &d, const dtn::data::Number &to)
		{
			dtn::core::EventDispatcher<StaticRouteChangeEvent>::queue( new StaticRouteChangeEvent(t, n, d, to) );
		}
	} /* namespace routing */
} /* namespace dtn */
