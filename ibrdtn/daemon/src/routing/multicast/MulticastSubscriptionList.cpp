/*
 * MulticastSubscriptionList.cpp
 *
 * Copyright (C) 2016 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <jm@m-network.de>
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

#include "MulticastSubscriptionList.h"

namespace dtn
{
	namespace routing
	{
		MulticastSubscription::MulticastSubscription(const dtn::data::EID &d, const dtn::data::Timestamp &e)
		 : destination(d), expiretime(e)
		{
		}

		MulticastSubscription::~MulticastSubscription()
		{
		}

		bool MulticastSubscription::isExpired(const dtn::data::Timestamp &timestamp) const
		{
			return expiretime <= timestamp;
		}

		const dtn::data::Number MulticastSubscriptionList::identifier = NodeHandshakeItem::MULTICAST_SUBSCRIPTIONS;

		MulticastSubscriptionList::MulticastSubscriptionList()
		: NeighborDataSetImpl(MulticastSubscriptionList::identifier)
		{
		}

		MulticastSubscriptionList::~MulticastSubscriptionList()
		{
		}
	}
}
