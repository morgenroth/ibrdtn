/*
 * StaticRouteChangeEvent.h
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

#ifndef STATICROUTECHANGEEVENT_H_
#define STATICROUTECHANGEEVENT_H_

#include "core/Event.h"
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/Number.h>

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

			std::string getMessage() const;

			static void raiseEvent(CHANGE_TYPE type);
			static void raiseEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const std::string &pattern, const dtn::data::Number &timeout = 0);
			static void raiseEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const dtn::data::EID &destination, const dtn::data::Number &timeout = 0);

			CHANGE_TYPE type;
			const dtn::data::EID nexthop;
			const dtn::data::EID destination;
			const std::string pattern;
			dtn::data::Number timeout;

		private:
			StaticRouteChangeEvent(CHANGE_TYPE type);
			StaticRouteChangeEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const std::string &pattern, const dtn::data::Number &timeout = 0);
			StaticRouteChangeEvent(CHANGE_TYPE type, const dtn::data::EID &nexthop, const dtn::data::EID &destination, const dtn::data::Number &timeout = 0);
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* STATICROUTECHANGEEVENT_H_ */
