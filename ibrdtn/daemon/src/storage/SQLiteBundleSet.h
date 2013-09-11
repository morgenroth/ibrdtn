/*
 * SQLiteBundleSet.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: David Goltzsche <goltzsch@ibr.cs.tu-bs.de>
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
 *  Created on: 19.04.2013
 */

#ifndef SQLITEBUNDLESET_H_
#define SQLITEBUNDLESET_H_

#include "ibrdtn/data/BundleSetImpl.h"
#include "ibrdtn/data/BundleSet.h"
#include "storage/SQLiteDatabase.h"
#include "ibrdtn/data/MetaBundle.h"
#include <ibrcommon/data/BloomFilter.h>

namespace dtn
{
	namespace storage
	{
		class SQLiteBundleSet : public dtn::data::BundleSetImpl
		{
		public:
			/**
			 * @param bf_size Initial size fo the bloom-filter.
			 */
			SQLiteBundleSet(const int id, bool persistant, dtn::data::BundleSet::Listener *listener, dtn::data::Size bf_size, dtn::storage::SQLiteDatabase& database);

			virtual ~SQLiteBundleSet();

			virtual void add(const dtn::data::MetaBundle &bundle) throw ();

			virtual void clear() throw ();

			virtual bool has(const dtn::data::BundleID &bundle) const throw ();

			virtual void expire(const dtn::data::Timestamp timestamp) throw ();

			/**
			 * Returns the number of elements in this set
			 */
			virtual dtn::data::Size size() const throw();

			/**
			 * Returns the data length of the serialized BundleSet
			 */
			dtn::data::Length getLength() const throw();

			const ibrcommon::BloomFilter& getBloomFilter() const throw();

			std::set<dtn::data::MetaBundle> getNotIn(const ibrcommon::BloomFilter &filter) const throw();

			virtual std::ostream &serialize(std::ostream &stream) const;
			virtual std::istream &deserialize(std::istream &stream);

		private:
			/**
			 * add a named seen bundle to the database
			 * @param name name, bundle bundle
			 */
			void add_seen_bundle(int name_id, const dtn::data::MetaBundle &bundle) throw (SQLiteDatabase::SQLiteQueryException);

			/**
			 * returns true, if database contains the bundle, false if not
			 */
			bool contains_seen_bundle(const dtn::data::BundleID &id) const throw (SQLiteDatabase::SQLiteQueryException);

			/*
			 * removes all bundles from a named bundleset
			 */
			void clear_seen_bundles(int name_id) throw (SQLiteDatabase::SQLiteQueryException);

			/*
			 * removes specific bundle from database
			 */
			void erase_seen_bundle(const dtn::data::BundleID &id) throw (SQLiteDatabase::SQLiteQueryException);

			/*
			 * returns a set of all bundles contained in the database
			 */
			std::set<dtn::data::MetaBundle> get_all_seen_bundles() const throw (SQLiteDatabase::SQLiteQueryException);

			/*
			 * returns number of seen bundles
			 */
			dtn::data::Size count_seen_bundles(int name_id) const throw (SQLiteDatabase::SQLiteQueryException);

			const int _name_id;

			ibrcommon::BloomFilter _bf;

			dtn::data::BundleSet::Listener *_listener;

			bool _consistent;

			dtn::storage::SQLiteDatabase& _database;

			dtn::data::Timestamp _next_expiration;

			void new_expire_time(const dtn::data::Timestamp &ttl) throw();

			void rebuild_bloom_filter();

			const bool _persistent;
		};
	} /* namespace data */
} /* namespace dtn */
#endif /* MEMORYBUNDLESET_H_ */
