/*
 * SQLiteBundleSetFactory.cpp
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
 *  Created on: May 7, 2013
 */

#include "storage/SQLiteBundleSetFactory.h"
#include "storage/SQLiteBundleSet.h"

namespace dtn
{
	namespace storage
	{
		SQLiteBundleSetFactory::SQLiteBundleSetFactory(SQLiteDatabase& db)
		 : _database(db)
		{
		}

		SQLiteBundleSetFactory::~SQLiteBundleSetFactory()
		{
		}

		dtn::data::BundleSetImpl* SQLiteBundleSetFactory::createBundleSet(dtn::data::BundleSet::Listener* listener, dtn::data::Size bf_size)
		{
			return new SQLiteBundleSet(create(), false, listener, bf_size, _database);
		}

		dtn::data::BundleSetImpl* SQLiteBundleSetFactory::createBundleSet(const std::string &name, dtn::data::BundleSet::Listener* listener, dtn::data::Size bf_size)
		{
			return new SQLiteBundleSet(create(name), true, listener, bf_size, _database);
		}

		int SQLiteBundleSetFactory::create() const throw (SQLiteDatabase::SQLiteQueryException)
		{
			std::string name;
			dtn::utils::Random rand;
			do {
				name = rand.gen_chars(32);
			} while (exists(name));

			// TODO: solve race condition between exists() and create

			return create(name);
		}

		int SQLiteBundleSetFactory::create(const std::string &name) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			SQLiteDatabase::Statement st1(_database._database, _database._sql_queries[SQLiteDatabase::BUNDLENAME_ADD]);
			sqlite3_bind_text(*st1,1,name.c_str(),name.length(),SQLITE_TRANSIENT);

			st1.step();

			SQLiteDatabase::Statement st2(_database._database, _database._sql_queries[SQLiteDatabase::BUNDLENAME_GET_NAME_ID]);
			sqlite3_bind_text(*st2,1,name.c_str(),name.length(),SQLITE_TRANSIENT);
			st2.step();
			return sqlite3_column_int64(*st2,0);
		}

		bool SQLiteBundleSetFactory::exists(const std::string &name) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			int rows = 0;
			SQLiteDatabase::Statement st(_database._database, _database._sql_queries[SQLiteDatabase::BUNDLENAME_COUNT]);
			sqlite3_bind_text(*st,1,name.c_str(),name.length(),SQLITE_TRANSIENT);

			if (( st.step()) == SQLITE_ROW){
				rows = sqlite3_column_int(*st, 0);
			}

			return rows > 0;
		}
	} /* namespace storage */
} /* namespace dtn */
