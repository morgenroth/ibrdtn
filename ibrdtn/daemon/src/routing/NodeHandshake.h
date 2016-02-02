/*
 * NodeHandshake.h
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

#ifndef NODEHANDSHAKE_H_
#define NODEHANDSHAKE_H_

#include "routing/NeighborDataset.h"
#include <ibrdtn/data/BundleSet.h>
#include <ibrdtn/data/SDNV.h>
#include <iostream>
#include <sstream>
#include <list>
#include <set>
#include <map>

namespace dtn
{
	namespace routing
	{
		class NodeHandshakeItem
		{
		public:
			enum IDENTIFIER
			{
				REQUEST_COMPRESSED_ANSWER = 0xff,
				BLOOM_FILTER_SUMMARY_VECTOR = 1,
				BLOOM_FILTER_PURGE_VECTOR = 2,
				DELIVERY_PREDICTABILITY_MAP = 3,
				PROPHET_ACKNOWLEDGEMENT_SET = 4,
				ROUTING_LIMITATIONS = 5
			};

			virtual ~NodeHandshakeItem() { };
			virtual const dtn::data::Number& getIdentifier() const = 0;
			virtual dtn::data::Length getLength() const = 0;
			virtual std::ostream& serialize(std::ostream&) const = 0;
			virtual std::istream& deserialize(std::istream&) = 0;
		};

		class BloomFilterSummaryVector : public NodeHandshakeItem
		{
		public:
			BloomFilterSummaryVector();
			BloomFilterSummaryVector(const dtn::data::BundleSet &vector);
			virtual ~BloomFilterSummaryVector();
			const dtn::data::Number& getIdentifier() const;
			dtn::data::Length getLength() const;
			std::ostream& serialize(std::ostream&) const;
			std::istream& deserialize(std::istream&);
			static const dtn::data::Number identifier;

			const dtn::data::BundleSet& getVector() const;

		private:
			dtn::data::BundleSet _vector;
		};

		class BloomFilterPurgeVector : public NodeHandshakeItem
		{
		public:
			BloomFilterPurgeVector();
			BloomFilterPurgeVector(const dtn::data::BundleSet &vector);
			virtual ~BloomFilterPurgeVector();
			const dtn::data::Number& getIdentifier() const;
			dtn::data::Length getLength() const;
			std::ostream& serialize(std::ostream&) const;
			std::istream& deserialize(std::istream&);
			static const dtn::data::Number identifier;

			const dtn::data::BundleSet& getVector() const;

		private:
			dtn::data::BundleSet _vector;
		};

		class RoutingLimitations : public NeighborDataSetImpl, public NodeHandshakeItem
		{
		public:
			RoutingLimitations();
			virtual ~RoutingLimitations();
			const dtn::data::Number& getIdentifier() const;
			dtn::data::Length getLength() const;
			std::ostream& serialize(std::ostream&) const;
			std::istream& deserialize(std::istream&);
			static const dtn::data::Number identifier;

			enum LimitIndex {
				LIMIT_BLOCKSIZE = 0,
				LIMIT_FOREIGN_BLOCKSIZE = 1,
				LIMIT_SINGLETON_ONLY = 2,
				LIMIT_LOCAL_ONLY = 3
			};

			void setLimit(LimitIndex index, ssize_t value);
			ssize_t getLimit(LimitIndex index) const;

		private:
			std::map<size_t, ssize_t> _limits;
		};

		class NodeHandshake
		{
		public:
			enum MESSAGE_TYPE
			{
				HANDSHAKE_INVALID = 0,
				HANDSHAKE_REQUEST = 1,
				HANDSHAKE_RESPONSE = 2,
				HANDSHAKE_NOTIFICATION = 3
			};

			typedef std::set<dtn::data::Number> request_set;
			typedef std::list<NodeHandshakeItem*> item_set;

			NodeHandshake();
			NodeHandshake(MESSAGE_TYPE type, const dtn::data::Number &lifetime = 60);

			virtual ~NodeHandshake();

			void addRequest(const dtn::data::Number &identifier);
			bool hasRequest(const dtn::data::Number &identifier) const;
			const request_set& getRequests() const;

			void addItem(NodeHandshakeItem *item);
			bool hasItem(const dtn::data::Number &identifier) const;
			const item_set& getItems() const;

			MESSAGE_TYPE getType() const;
			const dtn::data::Number& getLifetime() const;

			const std::string toString() const;

			friend std::ostream& operator<<(std::ostream&, const NodeHandshake&);
			friend std::istream& operator>>(std::istream&, NodeHandshake&);

			template<class T>
			T& get();

		private:
			class StreamMap
			{
			public:
				StreamMap();
				~StreamMap();
				void clear();
				std::stringstream& get(const dtn::data::Number &identifier);
				void remove(const dtn::data::Number &identifier);
				bool has(const dtn::data::Number &identifier);

			private:
				typedef std::map<dtn::data::Number, std::stringstream* > stream_map;
				stream_map _map;
			};

			NodeHandshakeItem* getItem(const dtn::data::Number &identifier) const;
			void clear();

			dtn::data::Number _type;
			dtn::data::Number _lifetime;

			request_set _requests;
			item_set _items;

			StreamMap _raw_items;

			// deny copying
			NodeHandshake& operator=( const NodeHandshake& ) { return *this; };
		};

		template<class T>
		T& NodeHandshake::get()
		{
			NodeHandshakeItem *item = getItem(T::identifier);

			if (item == NULL)
			{
				// check if the item exists
				if (!_raw_items.has(T::identifier))
					throw ibrcommon::Exception("item not available");

				T *item_template = new T();
				item_template->deserialize( _raw_items.get(T::identifier) );

				// remove the raw item
				_raw_items.remove(T::identifier);

				// put item into the itemMap
				_items.push_back(item_template);

				return (*item_template);
			}

			return dynamic_cast<T&>(*item);
		}
	} /* namespace routing */
} /* namespace dtn */
#endif /* NODEHANDSHAKE_H_ */
