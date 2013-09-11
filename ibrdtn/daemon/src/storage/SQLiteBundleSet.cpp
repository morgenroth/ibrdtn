/*
 * SQLiteBundleSet.cpp
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

#include "storage/SQLiteBundleSet.h"
#include <ibrdtn/utils/Random.h>


namespace dtn
{
	namespace storage
	{
		SQLiteBundleSet::SQLiteBundleSet(const int id, dtn::data::BundleSet::Listener *listener, dtn::data::Size bf_size, dtn::storage::SQLiteDatabase& database)
		 : _name_id(id), _bf(bf_size * 8), _listener(listener), _consistent(true),_database(database), _persistent(true)
		{
		}

		SQLiteBundleSet::~SQLiteBundleSet()
		{
			clear();
		}

		void SQLiteBundleSet::add(const dtn::data::MetaBundle &bundle) throw ()
		{
			// insert bundle id into database
			add_seen_bundle(_name_id, bundle);

			//update expiretime, if necessary
			new_expire_time(bundle.expiretime);

			// add bundle to the bloomfilter
			_bf.insert(bundle.toString());
		}

		void SQLiteBundleSet::clear() throw ()
		{
			clear_seen_bundles(_name_id);
		}

		bool SQLiteBundleSet::has(const dtn::data::BundleID &bundle) const throw ()
		{
			// check bloom-filter first
			if (_bf.contains(bundle.toString())) {
				// Return true if the bloom-filter is not consistent with
				// the bundles set. This happen if the MemoryBundleSet gets deserialized.
				if (!_consistent) return true;

				return contains_seen_bundle(bundle);
			}

			return false;
		}

		void SQLiteBundleSet::expire(const dtn::data::Timestamp timestamp) throw ()
		{
			//only write changes in bloomfilter, if it's time to do it
			bool commit = _next_expiration <= timestamp;

			// we can not expire bundles if we have no idea of time
			if (timestamp == 0) return;

			// expire in database
			_database.expire(timestamp);

			// raise expired event
			if (_listener != NULL)
				_listener->eventBundleExpired(dtn::data::MetaBundle()); //TODO was sinnvolles übergeben

			if (commit)
				rebuild_bloom_filter();
		}

		dtn::data::Size SQLiteBundleSet::size() const throw ()
		{
			return count_seen_bundles(_name_id);
		}

		dtn::data::Length SQLiteBundleSet::getLength() const throw ()
		{
			return dtn::data::Number(_bf.size()).getLength() + _bf.size();
		}

		const ibrcommon::BloomFilter& SQLiteBundleSet::getBloomFilter() const throw ()
		{
			return _bf;
		}

		std::set<dtn::data::MetaBundle> SQLiteBundleSet::getNotIn(const ibrcommon::BloomFilter &filter) const throw ()
		{
			std::set<dtn::data::MetaBundle> ret;

			//get bundles from db
			std::set<dtn::data::MetaBundle> bundles = get_all_seen_bundles();

			// iterate through all items to find the differences
			for (std::set<dtn::data::MetaBundle>::const_iterator iter = bundles.begin(); iter != bundles.end(); iter++)
			{
				if (!filter.contains( (*iter).toString() ) )
				{
					ret.insert( (*iter) );
				}
			}

			return ret;
		}

		std::ostream& SQLiteBundleSet::serialize(std::ostream &stream) const
		{
			dtn::data::Number size(_bf.size());
			stream << size;

			const char *data = reinterpret_cast<const char*>(_bf.table());
			stream.write(data, _bf.size());

			return stream;
		}

		std::istream& SQLiteBundleSet::deserialize(std::istream &stream)
		{
			dtn::data::Number count;
			stream >> count;

			std::vector<char> buffer(count.get<size_t>());

			stream.read(&buffer[0], buffer.size());

			SQLiteBundleSet::clear();
			_bf.load((unsigned char*)&buffer[0], buffer.size());

			// set the set to in-consistent mode
			_consistent = false;

			return stream;
		}

		void SQLiteBundleSet::new_expire_time(const dtn::data::Timestamp &ttl) throw ()
		{
			if (_next_expiration == 0 || ttl < _next_expiration)
			{
				_next_expiration = ttl;
			}
		}

		void SQLiteBundleSet::rebuild_bloom_filter()
		{
			// rebuild the bloom-filter
			_bf.clear();
			std::set<dtn::data::MetaBundle> bundles = get_all_seen_bundles();
			for (std::set<dtn::data::MetaBundle>::const_iterator iter = bundles.begin(); iter != bundles.end(); iter++)
			{
				_bf.insert( (*iter).toString() );
			}
			_consistent = true;
		}

		void SQLiteBundleSet::add_seen_bundle(int name_id, const dtn::data::MetaBundle &bundle) throw (SQLiteDatabase::SQLiteQueryException)
		{
			//inform database about (potentially) new expiretime
			dtn::data::Timestamp TTL = bundle.timestamp + bundle.lifetime;
			new_expire_time(TTL);

			SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_ADD]);

			const std::string source_id = bundle.source.getString();

			sqlite3_bind_text(*st,1,source_id.c_str(),source_id.length(),SQLITE_TRANSIENT);
			sqlite3_bind_int64(*st,2,bundle.timestamp.get<uint64_t>());
			sqlite3_bind_int64(*st,3,bundle.sequencenumber.get<uint64_t>());
			sqlite3_bind_int64(*st,4,bundle.offset.get<uint64_t>());
			sqlite3_bind_int64(*st,5,bundle.expiretime.get<uint64_t>());
			sqlite3_bind_int64(*st,6,name_id);

			st.step();

		}

		bool SQLiteBundleSet::contains_seen_bundle(const dtn::data::BundleID &id) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			SQLiteDatabase::Statement st( const_cast<sqlite3*>(_database._database), _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_GET]);

			const std::string source_id = id.source.getString();
			sqlite3_bind_text(*st,1,source_id.c_str(),source_id.length(),SQLITE_TRANSIENT);
			sqlite3_bind_int64(*st,2,id.timestamp.get<uint64_t>());
			sqlite3_bind_int64(*st,3,id.sequencenumber.get<uint64_t>());
			sqlite3_bind_int64(*st,4,id.offset.get<uint64_t>());

			if (st.step() == SQLITE_ROW)
				return true;
			return false;

		}

		void SQLiteBundleSet::clear_seen_bundles(int name_id) throw (SQLiteDatabase::SQLiteQueryException)
		{
			SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_CLEAR]);
			sqlite3_bind_int64(*st,1,name_id);

			st.step();
		}

		std::set<dtn::data::MetaBundle> SQLiteBundleSet::get_all_seen_bundles() const throw (SQLiteDatabase::SQLiteQueryException)
		{
			SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_GETALL]);

			std::set<dtn::data::MetaBundle> ret;

			while(st.step() == SQLITE_ROW){

				const char* source_id_ptr = reinterpret_cast<const char*> (sqlite3_column_text(*st,0));
				std::string source_id(source_id_ptr);
				int timestamp = sqlite3_column_int64(*st,1);
				int sequencenumber = sqlite3_column_int64(*st,2);
				int fragmenetoffset = sqlite3_column_int64(*st,3);

				dtn::data::BundleID id(dtn::data::EID(source_id),timestamp,sequencenumber,fragmenetoffset);
				dtn::data::MetaBundle bundle = dtn::data::MetaBundle::mockUp(id);

				ret.insert(bundle);
			}

			return ret;
		}

		dtn::data::Size SQLiteBundleSet::count_seen_bundles(int name_id) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			int rows = 0;

			SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_COUNT]);
			sqlite3_bind_int64(*st,1,name_id);

			if (( st.step()) == SQLITE_ROW)
			{
				rows = sqlite3_column_int(*st, 0);
			}

			return rows;
		}
	} /* namespace data */
} /* namespace dtn */
