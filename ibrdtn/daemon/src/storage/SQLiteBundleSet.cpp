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
		SQLiteBundleSet::Factory::Factory(SQLiteDatabase& db)
		 : _database(db)
		{
		}

		SQLiteBundleSet::Factory::~Factory()
		{
		}

		dtn::data::BundleSetImpl* SQLiteBundleSet::Factory::create(dtn::data::BundleSet::Listener* listener, dtn::data::Size bf_size)
		{
			return new SQLiteBundleSet(create(), false, listener, bf_size, _database);
		}

		dtn::data::BundleSetImpl* SQLiteBundleSet::Factory::create(const std::string &name, dtn::data::BundleSet::Listener* listener, dtn::data::Size bf_size)
		{
			return new SQLiteBundleSet(create(name), true, listener, bf_size, _database);
		}

		int SQLiteBundleSet::Factory::create() const throw (SQLiteDatabase::SQLiteQueryException)
		{
			std::string name;
			dtn::utils::Random rand;
			do {
				name = rand.gen_chars(32);
			} while (exists(name));

			// TODO: solve race condition between exists() and create

			return create(name);
		}

		int SQLiteBundleSet::Factory::create(const std::string &name) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			SQLiteDatabase::Statement st1(_database._database, _database._sql_queries[SQLiteDatabase::BUNDLENAME_ADD]);
			sqlite3_bind_text(*st1,1,name.c_str(),name.length(),SQLITE_TRANSIENT);

			st1.step();

			SQLiteDatabase::Statement st2(_database._database, _database._sql_queries[SQLiteDatabase::BUNDLENAME_GET_NAME_ID]);
			sqlite3_bind_text(*st2,1,name.c_str(),name.length(),SQLITE_TRANSIENT);
			st2.step();
			return sqlite3_column_int64(*st2,0);
		}

		bool SQLiteBundleSet::Factory::exists(const std::string &name) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			int rows = 0;
			SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::BUNDLENAME_COUNT]);
			sqlite3_bind_text(*st,1,name.c_str(),name.length(),SQLITE_TRANSIENT);

			if (( st.step()) == SQLITE_ROW){
				rows = sqlite3_column_int(*st, 0);
			}

			return rows > 0;
		}

		SQLiteBundleSet::SQLiteBundleSet(const int id, bool persistant, dtn::data::BundleSet::Listener *listener, dtn::data::Size bf_size, dtn::storage::SQLiteDatabase& database)
		 : _name_id(id), _bf(bf_size * 8), _listener(listener), _consistent(true),_database(database), _persistent(persistant)
		{
		}

		SQLiteBundleSet::~SQLiteBundleSet()
		{
			clear();
		}

		/**
		 * copies the current bundle-set into a new temporary one
		 */
		refcnt_ptr<dtn::data::BundleSetImpl> SQLiteBundleSet::copy() const
		{
			return refcnt_ptr<dtn::data::BundleSetImpl>(NULL);
		}

		/**
		 * clears the bundle-set and copy all entries from the given
		 * one into this bundle-set
		 */
		void SQLiteBundleSet::assign(const refcnt_ptr<BundleSetImpl>&)
		{

		}

		void SQLiteBundleSet::add(const dtn::data::MetaBundle &bundle) throw ()
		{
			// inform database about (potentially) new expiretime
			dtn::data::Timestamp TTL = bundle.timestamp + bundle.lifetime;
			new_expire_time(TTL);

			try {
				// insert bundle id into database
				SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_ADD]);

				const std::string source_id = bundle.source.getString();

				sqlite3_bind_text(*st,1,source_id.c_str(),source_id.length(),SQLITE_TRANSIENT);
				sqlite3_bind_int64(*st,2,bundle.timestamp.get<uint64_t>());
				sqlite3_bind_int64(*st,3,bundle.sequencenumber.get<uint64_t>());
				sqlite3_bind_int64(*st,4,bundle.offset.get<uint64_t>());
				sqlite3_bind_int64(*st,5,bundle.expiretime.get<uint64_t>());
				sqlite3_bind_int64(*st,6, _name_id);

				st.step();

				// update expiretime, if necessary
				new_expire_time(bundle.expiretime);

				// add bundle to the bloomfilter
				_bf.insert(bundle.toString());
			} catch (const SQLiteDatabase::SQLiteQueryException&) {
				// error
			}
		}

		void SQLiteBundleSet::clear() throw ()
		{
			try {
				SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_CLEAR]);
				sqlite3_bind_int64(*st,1, _name_id);

				st.step();
			} catch (const SQLiteDatabase::SQLiteQueryException&) {
				// error
			}
		}

		bool SQLiteBundleSet::has(const dtn::data::BundleID &id) const throw ()
		{
			// check bloom-filter first
			if (!_bf.contains(id.toString())) return false;

			// Return true if the bloom-filter is not consistent with
			// the bundles set. This happen if the MemoryBundleSet gets deserialized.
			if (!_consistent) return true;

			try {
				SQLiteDatabase::Statement st( const_cast<sqlite3*>(_database._database), _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_GET]);

				const std::string source_id = id.source.getString();
				sqlite3_bind_text(*st,1,source_id.c_str(),source_id.length(),SQLITE_TRANSIENT);
				sqlite3_bind_int64(*st,2,id.timestamp.get<uint64_t>());
				sqlite3_bind_int64(*st,3,id.sequencenumber.get<uint64_t>());
				sqlite3_bind_int64(*st,4,id.offset.get<uint64_t>());

				// TODO: check for name_id ?

				if (st.step() == SQLITE_ROW)
					return true;
			} catch (const SQLiteDatabase::SQLiteQueryException&) {
				// error
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
			// TODO: hand-over the listener to the database to let it
			// trigger the bundle expired event
			_database.expire(timestamp, _listener);

			if (commit)
				rebuild_bloom_filter();
		}

		dtn::data::Size SQLiteBundleSet::size() const throw ()
		{
			int rows = 0;

			try {
				SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_COUNT]);
				sqlite3_bind_int64(*st, 1, _name_id);

				if (( st.step()) == SQLITE_ROW)
				{
					rows = sqlite3_column_int(*st, 0);
				}
			} catch (const SQLiteDatabase::SQLiteQueryException&) {
				// error
			}

			return rows;
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

			try {
				SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_GETALL]);

				std::set<dtn::data::MetaBundle> ret;

				while (st.step() == SQLITE_ROW)
				{
					const char* source_id_ptr = reinterpret_cast<const char*> (sqlite3_column_text(*st,0));
					std::string source_id(source_id_ptr);
					int timestamp = sqlite3_column_int64(*st,1);
					int sequencenumber = sqlite3_column_int64(*st,2);
					int fragmenetoffset = sqlite3_column_int64(*st,3);

					// TODO: check for name_id ?

					dtn::data::BundleID id(dtn::data::EID(source_id),timestamp,sequencenumber,fragmenetoffset);
					dtn::data::MetaBundle bundle = dtn::data::MetaBundle::mockUp(id);

					if (!filter.contains( bundle.toString() ) )
					{
						ret.insert( bundle );
					}
				}
			} catch (const SQLiteDatabase::SQLiteQueryException&) {
				// error
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

			// update global expire time
			_database.new_expire_time(ttl);
			// TODO: this should no longer necessary if the expiration
			// is no longer done by the database
		}

		void SQLiteBundleSet::rebuild_bloom_filter()
		{
			// rebuild the bloom-filter
			_bf.clear();

			try {
				SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::SEEN_BUNDLE_GETALL]);

				std::set<dtn::data::MetaBundle> ret;

				while (st.step() == SQLITE_ROW)
				{
					const char* source_id_ptr = reinterpret_cast<const char*> (sqlite3_column_text(*st,0));
					std::string source_id(source_id_ptr);
					int timestamp = sqlite3_column_int64(*st,1);
					int sequencenumber = sqlite3_column_int64(*st,2);
					int fragmenetoffset = sqlite3_column_int64(*st,3);

					// TODO: check for name_id ?

					dtn::data::BundleID id(dtn::data::EID(source_id),timestamp,sequencenumber,fragmenetoffset);
					dtn::data::MetaBundle bundle = dtn::data::MetaBundle::mockUp(id);

					_bf.insert( bundle.toString() );
				}

				_consistent = true;
			} catch (const SQLiteDatabase::SQLiteQueryException&) {
				// error
			}
		}
	} /* namespace data */
} /* namespace dtn */
