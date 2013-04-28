/*
 * MetaStorage.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "storage/MetaStorage.h"

namespace dtn
{
	namespace storage
	{
		MetaStorage::MetaStorage(dtn::data::BundleList::Listener &expire_listener)
		 : _expire_listener(expire_listener), _list(this)
		{
		}

		MetaStorage::~MetaStorage()
		{
		}

		void MetaStorage::eventBundleExpired(const dtn::data::MetaBundle &b) throw ()
		{
			_expire_listener.eventBundleExpired(b);

			// remove the bundle out of all lists
			_priority_index.erase(b);
			_bundle_lengths.erase(b);
			_removal_set.erase(b);
		}

		bool MetaStorage::has(const dtn::data::MetaBundle &m) const throw ()
		{
			dtn::data::BundleList::const_iterator it = _list.find(m);
			return (it != _list.end());
		}

		void MetaStorage::expire(const dtn::data::Timestamp &timestamp) throw ()
		{
			_list.expire(timestamp);
		}

		const dtn::data::MetaBundle& MetaStorage::find(const ibrcommon::BloomFilter &filter) const throw (NoBundleFoundException)
		{
			for (const_iterator iter = begin(); iter != end(); ++iter)
			{
				const dtn::data::MetaBundle &bundle = (*iter);

				// skip removal-marked bundles
				if (_removal_set.find(bundle) != _removal_set.end()) continue;

				if (filter.contains(bundle.toString()))
				{
					return bundle;
				}
			}

			throw NoBundleFoundException();
		}

		std::set<dtn::data::EID> MetaStorage::getDistinctDestinations() const throw ()
		{
			std::set<dtn::data::EID> ret;

			for (dtn::data::BundleList::const_iterator iter = begin(); iter != end(); ++iter)
			{
				const dtn::data::MetaBundle &bundle = (*iter);

				// skip removal-marked bundles
				if (_removal_set.find(bundle) != _removal_set.end()) continue;

				ret.insert(bundle.destination);
			}

			return ret;
		}

		void MetaStorage::store(const dtn::data::MetaBundle &meta, size_t space) throw ()
		{
			// increment the storage size
			_bundle_lengths[meta] = space;

			// add it to the bundle list
			_list.add(meta);

			// add bundle to priority list
			_priority_index.insert(meta);
		}

		void MetaStorage::remove(const dtn::data::MetaBundle &meta) throw ()
		{
			_bundle_lengths.erase(meta);
			_removal_set.erase(meta);

			const dtn::data::MetaBundle mcopy = meta;
			_list.remove(mcopy);
			_priority_index.erase(mcopy);
		}

		void MetaStorage::markRemoved(const dtn::data::MetaBundle &meta) throw ()
		{
			if (has(meta)) {
				_removal_set.insert(meta);
			}
		}

		bool MetaStorage::empty() throw ()
		{
			if ( _priority_index.empty() )
			{
				return true;
			}

			return size() == 0;
		}

		size_t MetaStorage::size() throw ()
		{
			return _priority_index.size() - _removal_set.size();
		}

		void MetaStorage::clear() throw ()
		{
			_priority_index.clear();
			_list.clear();
			_bundle_lengths.clear();
			_removal_set.clear();
		}

		size_t MetaStorage::getSize(const dtn::data::MetaBundle &meta) throw (NoBundleFoundException)
		{
			size_map::const_iterator it = _bundle_lengths.find(meta);
			if (it == _bundle_lengths.end())
				throw NoBundleFoundException();

			return (*it).second;
		}
	} /* namespace storage */
} /* namespace dtn */
