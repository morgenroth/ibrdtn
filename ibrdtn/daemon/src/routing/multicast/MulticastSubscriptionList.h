/*
 * MulticastSubscriptionList.h
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

#ifndef MULTICASTSUBSCRIPTIONLIST_H_
#define MULTICASTSUBSCRIPTIONLIST_H_

#include "routing/NodeHandshake.h"
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/Number.h>

#include <ibrcommon/thread/Mutex.h>

#include <map>
#include <list>

namespace dtn
{
	namespace routing
	{
		class MulticastSubscription
		{
		public:
			MulticastSubscription(const dtn::data::EID &destination, const dtn::data::Timestamp &expiretime);
			virtual ~MulticastSubscription();

			/**
			 * Determine if the entry is expired
			 */
			bool isExpired(const dtn::data::Timestamp &timestamp) const;

			const dtn::data::EID destination;
			dtn::data::Timestamp expiretime;
		};

		class MulticastSubscriptionList :
				public NeighborDataSetImpl,
				public NodeHandshakeItem,
				public ibrcommon::Mutex
		{
		public:
			static const dtn::data::Number identifier;

			MulticastSubscriptionList();
			virtual ~MulticastSubscriptionList();

			class SubscriptionNotFoundException : public ibrcommon::Exception
			{
			public:
				SubscriptionNotFoundException()
				: ibrcommon::Exception("The requested subscription is not available.") { };

				virtual ~SubscriptionNotFoundException() throw () { };
			};

			virtual const dtn::data::Number& getIdentifier() const; ///< \see NodeHandshakeItem::getIdentifier
			virtual dtn::data::Length getLength() const; ///< \see NodeHandshakeItem::getLength
			virtual std::ostream& serialize(std::ostream& stream) const; ///< \see NodeHandshakeItem::serialize
			virtual std::istream& deserialize(std::istream& stream); ///< \see NodeHandshakeItem::deserialize

			/**
			 * Iterator methods and definitions
			 */
			typedef std::list<MulticastSubscription> subscription_list;
			typedef std::map<dtn::data::EID, subscription_list> subscription_map;
			typedef ibrcommon::key_iterator<subscription_map> const_iterator;

			const_iterator begin() const;
			const_iterator end() const;

			/**
			 * Add received subscription
			 */
			void set(const dtn::data::EID &group, const dtn::data::EID &singleton, size_t lifetime);

			/**
			 * Merge another list into this list
			 */
			void merge(const MulticastSubscriptionList &list);

			/**
			 * Find and return a list of subscriptions for a group
			 */
			const subscription_list& find(const dtn::data::EID &group) throw (SubscriptionNotFoundException);

			/**
			 * Expire out-dated entries
			 */
			void expire(const dtn::data::Timestamp &timestamp);

			/**
			 * Clear all subscriptions
			 */
			void clear();

			/**
			 * writes the subscription map to a stream
			 */
			void store(std::ostream &output) const;

			/**
			 * reads the subscription map from a stream
			 */
			void restore(std::istream &input);

		private:
			subscription_map _map;
		};
	}
}

#endif /* MULTICASTSUBSCRIPTIONLIST_H_ */
