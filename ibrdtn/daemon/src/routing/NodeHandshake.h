/*
 * NodeHandshake.h
 *
 *  Created on: 09.12.2011
 *      Author: morgenro
 */

#ifndef NODEHANDSHAKE_H_
#define NODEHANDSHAKE_H_

#include "routing/SummaryVector.h"
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
				BLOOM_FILTER_SUMMARY_VECTOR = 1,
				BLOOM_FILTER_PURGE_VECTOR = 2,
				DELIVERY_PREDICTABILITY_MAP = 3,
				PROPHET_ACKNOWLEDGEMENT_SET = 4
			};

			virtual ~NodeHandshakeItem() { };
			virtual size_t getIdentifier() const = 0;
			virtual size_t getLength() const = 0;
			virtual std::ostream& serialize(std::ostream&) const = 0;
			virtual std::istream& deserialize(std::istream&) = 0;
		};

		class BloomFilterSummaryVector : public NodeHandshakeItem
		{
		public:
			BloomFilterSummaryVector();
			BloomFilterSummaryVector(const SummaryVector &vector);
			virtual ~BloomFilterSummaryVector();
			size_t getIdentifier() const;
			size_t getLength() const;
			std::ostream& serialize(std::ostream&) const;
			std::istream& deserialize(std::istream&);
			static size_t identifier;

			const SummaryVector& getVector() const;

		private:
			SummaryVector _vector;
		};

		class BloomFilterPurgeVector : public NodeHandshakeItem
		{
		public:
			BloomFilterPurgeVector();
			BloomFilterPurgeVector(const SummaryVector &vector);
			virtual ~BloomFilterPurgeVector();
			size_t getIdentifier() const;
			size_t getLength() const;
			std::ostream& serialize(std::ostream&) const;
			std::istream& deserialize(std::istream&);
			static size_t identifier;

			const SummaryVector& getVector() const;

		private:
			SummaryVector _vector;
		};

		class NodeHandshake
		{
		public:
			enum MESSAGE_TYPE
			{
				HANDSHAKE_INVALID = 0,
				HANDSHAKE_REQUEST = 1,
				HANDSHAKE_RESPONSE = 2
			};

			NodeHandshake();
			NodeHandshake(MESSAGE_TYPE type, size_t lifetime = 60);

			virtual ~NodeHandshake();

			void addRequest(const size_t identifier);
			bool hasRequest(const size_t identifier) const;
			void addItem(NodeHandshakeItem *item);
			bool hasItem(const size_t identifier) const;

			size_t getType() const;
			size_t getLifetime() const;

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
				std::stringstream& get(size_t identifier);
				void remove(size_t identifier);
				bool has(size_t identifier);

			private:
				std::map<size_t, std::stringstream* > _map;
			};

			NodeHandshakeItem* getItem(const size_t identifier) const;
			void clear();

			size_t _type;
			size_t _lifetime;

			std::set<size_t> _requests;
			std::list<NodeHandshakeItem*> _items;
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
