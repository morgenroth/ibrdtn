/*
 * NodeHandshake.cpp
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

#include "routing/NodeHandshake.h"

namespace dtn
{
	namespace routing
	{
		NodeHandshake::NodeHandshake()
		 : _type(HANDSHAKE_INVALID), _lifetime(0)
		{
		}

		NodeHandshake::NodeHandshake(MESSAGE_TYPE type, const dtn::data::SDNV &lifetime)
		 : _type(type), _lifetime(lifetime)
		{
		}

		NodeHandshake::~NodeHandshake()
		{
			clear();
		}

		void NodeHandshake::clear()
		{
			for (std::list<NodeHandshakeItem*>::const_iterator iter = _items.begin(); iter != _items.end(); ++iter)
			{
				delete (*iter);
			}

			// clear raw items too
			_raw_items.clear();
		}

		void NodeHandshake::addRequest(const dtn::data::SDNV &identifier)
		{
			_requests.insert(identifier);
		}

		bool NodeHandshake::hasRequest(const dtn::data::SDNV &identifier) const
		{
			return (_requests.find(identifier) != _requests.end());
		}

		void NodeHandshake::addItem(NodeHandshakeItem *item)
		{
			_items.push_back(item);
		}

		bool NodeHandshake::hasItem(const dtn::data::SDNV &identifier) const
		{
			for (std::list<NodeHandshakeItem*>::const_iterator iter = _items.begin(); iter != _items.end(); ++iter)
			{
				const NodeHandshakeItem &item = (**iter);
				if (item.getIdentifier() == identifier) return true;
			}

			return false;
		}

		NodeHandshakeItem* NodeHandshake::getItem(const dtn::data::SDNV &identifier) const
		{
			for (std::list<NodeHandshakeItem*>::const_iterator iter = _items.begin(); iter != _items.end(); ++iter)
			{
				NodeHandshakeItem *item = (*iter);
				if (item->getIdentifier() == identifier) return item;
			}

			return NULL;
		}

		const dtn::data::SDNV& NodeHandshake::getType() const
		{
			return _type;
		}

		const dtn::data::SDNV& NodeHandshake::getLifetime() const
		{
			return _lifetime;
		}

		const std::string NodeHandshake::toString() const
		{
			std::stringstream ss;

			switch (getType().getValue()) {
				case HANDSHAKE_INVALID:
					ss << "HANDSHAKE_INVALID";
					break;

				case HANDSHAKE_REQUEST:
					ss << "HANDSHAKE_REQUEST";
					break;

				case HANDSHAKE_RESPONSE:
					ss << "HANDSHAKE_RESPONSE";
					break;

				default:
					ss << "HANDSHAKE";
					break;
			}

			ss << "[ttl: " << getLifetime() << ",";

			if (getType() == NodeHandshake::HANDSHAKE_REQUEST)
			{
				for (request_set::const_iterator iter = _requests.begin(); iter != _requests.end(); ++iter)
				{
					const dtn::data::SDNV &item = (*iter);
					ss << " " << item;
				}
			}
			else if (getType() == NodeHandshake::HANDSHAKE_RESPONSE)
			{
				for (std::list<NodeHandshakeItem*>::const_iterator iter = _items.begin(); iter != _items.end(); ++iter)
				{
					const NodeHandshakeItem &item (**iter);
					ss << " " << item.getIdentifier();
				}
			}

			ss << "]";

			return ss.str();
		}

		std::ostream& operator<<(std::ostream &stream, const NodeHandshake &hs)
		{
			// first the type as SDNV
			dtn::data::SDNV type(hs._type);
			stream << type;

			if (hs._type == NodeHandshake::HANDSHAKE_REQUEST)
			{
				// then the number of request items
				dtn::data::SDNV number_of_items(hs._requests.size());
				stream << number_of_items;

				for (NodeHandshake::request_set::const_iterator iter = hs._requests.begin(); iter != hs._requests.end(); ++iter)
				{
					dtn::data::SDNV req(*iter);
					stream << req;
				}
			}
			else if (hs._type == NodeHandshake::HANDSHAKE_RESPONSE)
			{
				// then the lifetime of this data
				dtn::data::SDNV lifetime(hs._lifetime);
				stream << lifetime;

				// then the number of request items
				dtn::data::SDNV number_of_items(hs._items.size());
				stream << number_of_items;

				for (std::list<NodeHandshakeItem*>::const_iterator iter = hs._items.begin(); iter != hs._items.end(); ++iter)
				{
					const NodeHandshakeItem &item (**iter);

					// first the identifier of the item
					dtn::data::SDNV id(item.getIdentifier());
					stream << id;

					// then the length of the payload
					dtn::data::SDNV len(item.getLength());
					stream << len;

					item.serialize(stream);
				}
			}

			return stream;
		}

		std::istream& operator>>(std::istream &stream, NodeHandshake &hs)
		{
			// clear the node handshake object
			hs.clear();

			// first the type as SDNV
			stream >> hs._type;

			if (hs._type.getValue() == NodeHandshake::HANDSHAKE_REQUEST)
			{
				// then the number of request items
				dtn::data::SDNV number_of_items;
				stream >> number_of_items;

				for (size_t i = 0; i < number_of_items.getValue(); ++i)
				{
					dtn::data::SDNV req;
					stream >> req;
					hs._requests.insert(req);
				}
			}
			else if (hs._type == NodeHandshake::HANDSHAKE_RESPONSE)
			{
				// then the lifetime of this data
				stream >> hs._lifetime;

				// then the number of request items
				dtn::data::SDNV number_of_items;
				stream >> number_of_items;

				for (size_t i = 0; i < number_of_items.getValue(); ++i)
				{
					// first the identifier of the item
					dtn::data::SDNV id;
					stream >> id;

					// then the length of the payload
					dtn::data::SDNV len;
					stream >> len;

					// add to raw map and create data container
					//std::stringstream *data = new std::stringstream();
					std::stringstream &data = hs._raw_items.get(id);

					// copy data to stringstream
					ibrcommon::BLOB::copy(data, stream, (size_t)len.getValue());
				}
			}

			return stream;
		}

		NodeHandshake::StreamMap::StreamMap()
		{
		}

		NodeHandshake::StreamMap::~StreamMap()
		{
			clear();
		}

		bool NodeHandshake::StreamMap::has(const dtn::data::SDNV &identifier)
		{
			stream_map::iterator i = _map.find(identifier);
			return (i != _map.end());
		}

		std::stringstream& NodeHandshake::StreamMap::get(const dtn::data::SDNV &identifier)
		{
			stream_map::iterator i = _map.find(identifier);

			if (i == _map.end())
			{
				std::pair<stream_map::iterator, bool> p =
						_map.insert(std::pair<dtn::data::SDNV, std::stringstream* >(identifier, new stringstream()));
				i = p.first;
			}

			return (*(*i).second);
		}

		void NodeHandshake::StreamMap::remove(const dtn::data::SDNV &identifier)
		{
			stream_map::iterator i = _map.find(identifier);

			if (i != _map.end())
			{
				delete (*i).second;
				_map.erase(identifier);
			}
		}

		void NodeHandshake::StreamMap::clear()
		{
			for (stream_map::iterator iter = _map.begin(); iter != _map.end(); ++iter)
			{
				delete (*iter).second;
			}
			_map.clear();
		}

		BloomFilterSummaryVector::BloomFilterSummaryVector(const dtn::data::BundleSet &vector)
		 : _vector(vector)
		{
		}

		BloomFilterSummaryVector::BloomFilterSummaryVector()
		{
		}

		BloomFilterSummaryVector::~BloomFilterSummaryVector()
		{
		}

		const dtn::data::SDNV& BloomFilterSummaryVector::getIdentifier() const
		{
			return identifier;
		}

		size_t BloomFilterSummaryVector::getLength() const
		{
			return _vector.getLength();
		}

		const dtn::data::BundleSet& BloomFilterSummaryVector::getVector() const
		{
			return _vector;
		}

		std::ostream& BloomFilterSummaryVector::serialize(std::ostream &stream) const
		{
			stream << _vector;
			return stream;
		}

		std::istream& BloomFilterSummaryVector::deserialize(std::istream &stream)
		{
			stream >> _vector;
			return stream;
		}

		const dtn::data::SDNV BloomFilterSummaryVector::identifier = NodeHandshakeItem::BLOOM_FILTER_SUMMARY_VECTOR;

		BloomFilterPurgeVector::BloomFilterPurgeVector(const dtn::data::BundleSet &vector)
		 : _vector(vector)
		{
		}

		BloomFilterPurgeVector::BloomFilterPurgeVector()
		{
		}

		BloomFilterPurgeVector::~BloomFilterPurgeVector()
		{
		}

		const dtn::data::SDNV& BloomFilterPurgeVector::getIdentifier() const
		{
			return identifier;
		}

		size_t BloomFilterPurgeVector::getLength() const
		{
			return _vector.getLength();
		}

		const dtn::data::BundleSet& BloomFilterPurgeVector::getVector() const
		{
			return _vector;
		}

		std::ostream& BloomFilterPurgeVector::serialize(std::ostream &stream) const
		{
			stream << _vector;
			return stream;
		}

		std::istream& BloomFilterPurgeVector::deserialize(std::istream &stream)
		{
			stream >> _vector;
			return stream;
		}

		const dtn::data::SDNV BloomFilterPurgeVector::identifier = NodeHandshakeItem::BLOOM_FILTER_PURGE_VECTOR;

	} /* namespace routing */
} /* namespace dtn */
