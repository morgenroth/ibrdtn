/*
 * MetaStorage.h
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

#ifndef METASTORAGE_H_
#define METASTORAGE_H_

#include <storage/BundleSelector.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/BundleList.h>
#include <ibrcommon/data/BloomFilter.h>
#include <stdint.h>

namespace dtn
{
	namespace storage
	{
		class MetaStorage : public dtn::data::BundleList::Listener
		{
		public:
			struct CMP_BUNDLE_PRIORITY
			{
				bool operator() (const dtn::data::MetaBundle& lhs, const dtn::data::MetaBundle& rhs) const
				{
					if (lhs.getPriority() > rhs.getPriority())
						return true;

					if (lhs.getPriority() != rhs.getPriority())
						return false;

					if (lhs.timestamp < rhs.timestamp)
						return true;

					if (lhs.timestamp != rhs.timestamp)
						return false;

					if (lhs.sequencenumber < rhs.sequencenumber)
						return true;

					if (lhs.sequencenumber != rhs.sequencenumber)
						return false;

					if (rhs.fragment)
					{
						if (!lhs.fragment) return true;
						return (lhs.offset < rhs.offset);
					}

					return false;
				}
			};

			typedef std::set<dtn::data::MetaBundle, CMP_BUNDLE_PRIORITY> priority_set;

		private:
			priority_set _priority_index;

			dtn::data::BundleList::Listener &_expire_listener;

			// bundle list
			dtn::data::BundleList _list;

			typedef std::map<dtn::data::BundleID, dtn::data::Length> size_map;
			size_map _bundle_lengths;

			typedef std::set<dtn::data::BundleID> id_set;
			id_set _removal_set;

		public:
			MetaStorage(dtn::data::BundleList::Listener &expire_listener);
			virtual ~MetaStorage();

			typedef priority_set::iterator iterator;
			typedef priority_set::const_iterator const_iterator;

			iterator begin() { return _priority_index.begin(); }
			iterator end() { return _priority_index.end(); }

			const_iterator begin() const { return _priority_index.begin(); }
			const_iterator end() const { return _priority_index.end(); }

			bool empty() throw ();
			size_t size() throw ();

			bool has(const dtn::data::MetaBundle &m) const throw ();
			void expire(const dtn::data::Timestamp &timestamp) throw ();

			template<class T>
			const dtn::data::MetaBundle& find(const T &id) const throw (NoBundleFoundException)
			{
				dtn::data::BundleList::const_iterator it = _list.find(id);
				if (it == _list.end())
					throw NoBundleFoundException();

				return (*it);
			}

			const dtn::data::MetaBundle& find(const ibrcommon::BloomFilter &filter) const throw (NoBundleFoundException);

			std::set<dtn::data::EID> getDistinctDestinations() const throw ();

			void store(const dtn::data::MetaBundle &meta, const dtn::data::Length &space) throw ();

			void remove(const dtn::data::MetaBundle &meta) throw ();

			/**
			 * Mark a bundle as removed. Such a bundle is still reachable
			 * using find(id) and getSize()
			 */
			void markRemoved(const dtn::data::MetaBundle &meta) throw ();

			/**
			 * Delete all bundles
			 */
			void clear() throw ();

			/**
			 * Get the size of the bundle
			 */
			dtn::data::Length getSize(const dtn::data::MetaBundle &meta) throw (NoBundleFoundException);

		protected:
			virtual void eventBundleExpired(const dtn::data::MetaBundle &b) throw ();
		};
	} /* namespace storage */
} /* namespace dtn */
#endif /* METASTORAGE_H_ */
