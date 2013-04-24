/*
 * SQLiteDatabase.cpp
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

#include "storage/SQLiteDatabase.h"
#include "storage/SQLiteConfigure.h"
#include <ibrdtn/data/ScopeControlHopLimitBlock.h>
#include <ibrdtn/data/SchedulingBlock.h>
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace storage
	{
		void sql_tracer(void*, const char * pQuery)
		{
			IBRCOMMON_LOGGER_DEBUG(50) << "sqlite trace: " << pQuery << IBRCOMMON_LOGGER_ENDL;
		}

		const std::string SQLiteDatabase::TAG = "SQLiteDatabase";

		const std::string SQLiteDatabase::_select_names[2] = {
				"source, destination, reportto, custodian, procflags, timestamp, sequencenumber, lifetime, fragmentoffset, appdatalength, hopcount, payloadlength, netpriority",
				"source, timestamp, sequencenumber, fragmentoffset, procflags"
		};

		const std::string SQLiteDatabase::_tables[SQL_TABLE_END] =
				{ "bundles", "blocks", "routing", "routing_bundles", "routing_nodes", "properties" };

		const int SQLiteDatabase::DBSCHEMA_VERSION = 2;
		const std::string SQLiteDatabase::QUERY_SCHEMAVERSION = "SELECT `value` FROM " + SQLiteDatabase::_tables[SQLiteDatabase::SQL_TABLE_PROPERTIES] + " WHERE `key` = 'version' LIMIT 0,1;";
		const std::string SQLiteDatabase::SET_SCHEMAVERSION = "INSERT INTO " + SQLiteDatabase::_tables[SQLiteDatabase::SQL_TABLE_PROPERTIES] + " (`key`, `value`) VALUES ('version', ?);";

		const std::string SQLiteDatabase::_sql_queries[SQL_QUERIES_END] =
		{
			"SELECT " + _select_names[0] + " FROM " + _tables[SQL_TABLE_BUNDLE],
			"SELECT " + _select_names[0] + " FROM " + _tables[SQL_TABLE_BUNDLE] + " ORDER BY priority DESC, timestamp, sequencenumber, fragmentoffset LIMIT ?,?;",
			"SELECT " + _select_names[0] + " FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset IS NULL LIMIT 1;",
			"SELECT " + _select_names[0] + " FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ? LIMIT 1;",
			"SELECT * FROM " + _tables[SQL_TABLE_BUNDLE] + " WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset != NULL ORDER BY fragmentoffset ASC;",

			"SELECT " + _select_names[1] + " FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE expiretime <= ?;",
			"SELECT filename FROM "+ _tables[SQL_TABLE_BUNDLE] +" as a, "+ _tables[SQL_TABLE_BLOCK] +" as b WHERE a.source_id = b.source_id AND a.timestamp = b.timestamp AND a.sequencenumber = b.sequencenumber AND ((a.fragmentoffset = b.fragmentoffset) OR ((a.fragmentoffset IS NULL) AND (b.fragmentoffset IS NULL))) AND a.expiretime <= ?;",
			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE expiretime <= ?;",
			"SELECT expiretime FROM "+ _tables[SQL_TABLE_BUNDLE] +" ORDER BY expiretime ASC LIMIT 1;",

			"SELECT ROWID FROM "+ _tables[SQL_TABLE_BUNDLE] +" LIMIT 1;",
			"SELECT COUNT(ROWID) FROM "+ _tables[SQL_TABLE_BUNDLE] +";",

			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset IS NULL;",
			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ?;",
			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +";",
			"INSERT INTO "+ _tables[SQL_TABLE_BUNDLE] +" (source_id, timestamp, sequencenumber, fragmentoffset, source, destination, reportto, custodian, procflags, lifetime, appdatalength, expiretime, priority, hopcount, payloadlength, netpriority) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);",
			"UPDATE "+ _tables[SQL_TABLE_BUNDLE] +" SET custodian = ? WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset IS NULL;",
			"UPDATE "+ _tables[SQL_TABLE_BUNDLE] +" SET custodian = ? WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ?;",

			"UPDATE "+ _tables[SQL_TABLE_BUNDLE] +" SET procflags = ? WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ?;",

			"SELECT filename, blocktype FROM "+ _tables[SQL_TABLE_BLOCK] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset IS NULL ORDER BY ordernumber ASC;",
			"SELECT filename, blocktype FROM "+ _tables[SQL_TABLE_BLOCK] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ? ORDER BY ordernumber ASC;",
			"SELECT filename, blocktype FROM "+ _tables[SQL_TABLE_BLOCK] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset IS NULL AND ordernumber = ?;",
			"SELECT filename, blocktype FROM "+ _tables[SQL_TABLE_BLOCK] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ? AND ordernumber = ?;",
			"DELETE FROM "+ _tables[SQL_TABLE_BLOCK] +";",
			"INSERT INTO "+ _tables[SQL_TABLE_BLOCK] +" (source_id, timestamp, sequencenumber, fragmentoffset, blocktype, filename, ordernumber) VALUES (?,?,?,?,?,?,?);",

			"VACUUM;",
		};

		const std::string SQLiteDatabase::_db_structure[11] =
		{
			"CREATE TABLE IF NOT EXISTS `" + _tables[SQL_TABLE_BLOCK] + "` ( `key` INTEGER PRIMARY KEY ASC, `source_id` TEXT NOT NULL, `timestamp` INTEGER NOT NULL, `sequencenumber` INTEGER NOT NULL, `fragmentoffset` INTEGER DEFAULT NULL, `blocktype` INTEGER NOT NULL, `filename` TEXT NOT NULL, `ordernumber` INTEGER NOT NULL);",
			"CREATE TABLE IF NOT EXISTS `" + _tables[SQL_TABLE_BUNDLE] + "` ( `key` INTEGER PRIMARY KEY ASC, `source_id` TEXT NOT NULL, `source` TEXT NOT NULL, `destination` TEXT NOT NULL, `reportto` TEXT NOT NULL, `custodian` TEXT NOT NULL, `procflags` INTEGER NOT NULL, `timestamp` INTEGER NOT NULL, `sequencenumber` INTEGER NOT NULL, `lifetime` INTEGER NOT NULL, `fragmentoffset` INTEGER DEFAULT NULL, `appdatalength` INTEGER DEFAULT NULL, `expiretime` INTEGER NOT NULL, `priority` INTEGER NOT NULL, `hopcount` INTEGER DEFAULT NULL, `payloadlength` INTEGER NOT NULL);",
			"create table if not exists "+ _tables[SQL_TABLE_ROUTING] +" (INTEGER PRIMARY KEY ASC, Key int, Routing text);",
			"create table if not exists "+ _tables[SQL_TABLE_BUNDLE_ROUTING_INFO] +" (INTEGER PRIMARY KEY ASC, BundleID text, Key int, Routing text);",
			"create table if not exists "+ _tables[SQL_TABLE_NODE_ROUTING_INFO] +" (INTEGER PRIMARY KEY ASC, EID text, Key int, Routing text);",
			"CREATE TRIGGER IF NOT EXISTS blocks_autodelete AFTER DELETE ON " + _tables[SQL_TABLE_BUNDLE] + " FOR EACH ROW BEGIN DELETE FROM " + _tables[SQL_TABLE_BLOCK] + " WHERE " + _tables[SQL_TABLE_BLOCK] + ".source_id = OLD.source_id AND " + _tables[SQL_TABLE_BLOCK] + ".timestamp = OLD.timestamp AND " + _tables[SQL_TABLE_BLOCK] + ".sequencenumber = OLD.sequencenumber AND ((" + _tables[SQL_TABLE_BLOCK] + ".fragmentoffset IS NULL AND old.fragmentoffset IS NULL) OR (" + _tables[SQL_TABLE_BLOCK] + ".fragmentoffset = old.fragmentoffset)); END;",
			"CREATE UNIQUE INDEX IF NOT EXISTS blocks_bid ON " + _tables[SQL_TABLE_BLOCK] + " (source_id, timestamp, sequencenumber, fragmentoffset);",
			"CREATE INDEX IF NOT EXISTS bundles_destination ON " + _tables[SQL_TABLE_BUNDLE] + " (destination);",
			"CREATE INDEX IF NOT EXISTS bundles_destination_priority ON " + _tables[SQL_TABLE_BUNDLE] + " (destination, priority);",
			"CREATE UNIQUE INDEX IF NOT EXISTS bundles_id ON " + _tables[SQL_TABLE_BUNDLE] + " (source_id, timestamp, sequencenumber, fragmentoffset);"
			"CREATE INDEX IF NOT EXISTS bundles_expire ON " + _tables[SQL_TABLE_BUNDLE] + " (source_id, timestamp, sequencenumber, fragmentoffset, expiretime);",
			"CREATE TABLE IF NOT EXISTS '" + _tables[SQL_TABLE_PROPERTIES] + "' ( `key` TEXT PRIMARY KEY ASC ON CONFLICT REPLACE, `value` TEXT NOT NULL);"
		};

		SQLiteDatabase::SQLBundleQuery::SQLBundleQuery()
		{ }

		SQLiteDatabase::SQLBundleQuery::~SQLBundleQuery()
		{
		}

		SQLiteDatabase::Statement::Statement(sqlite3 *database, const std::string &query)
		 : _database(database), _st(NULL), _query(query)
		{
			prepare();
		}

		SQLiteDatabase::Statement::~Statement()
		{
			if (_st != NULL) {
				sqlite3_finalize(_st);
			}
		}

		sqlite3_stmt* SQLiteDatabase::Statement::operator*()
		{
			return _st;
		}

		void SQLiteDatabase::Statement::reset() throw ()
		{
			if (_st != NULL) {
				sqlite3_reset(_st);
				sqlite3_clear_bindings(_st);
			}
		}

		int SQLiteDatabase::Statement::step() throw (SQLiteQueryException)
		{
			if (_st == NULL)
				throw SQLiteQueryException("statement not prepared");

			int ret = sqlite3_step(_st);

			// check if the return value signals an error
			switch (ret)
			{
			case SQLITE_CORRUPT:
				throw SQLiteQueryException("Database is corrupt.");

			case SQLITE_INTERRUPT:
				throw SQLiteQueryException("Database interrupt.");

			case SQLITE_SCHEMA:
				throw SQLiteQueryException("Database schema error.");

			case SQLITE_ERROR:
				throw SQLiteQueryException("Database error.");
			default:
				return ret;
			}
		}

		void SQLiteDatabase::Statement::prepare() throw (SQLiteDatabase::SQLiteQueryException)
		{
			if (_st != NULL)
				throw SQLiteQueryException("already prepared");

			int err = sqlite3_prepare_v2(_database, _query.c_str(), _query.length(), &_st, 0);

			if ( err != SQLITE_OK )
				throw SQLiteQueryException("failed to prepare statement: " + _query);
		}

		SQLiteDatabase::DatabaseListener::~DatabaseListener() {}

		SQLiteDatabase::SQLiteDatabase(const ibrcommon::File &file, DatabaseListener &listener)
		 : _file(file), _database(NULL), _next_expiration(0), _listener(listener)
		{
		}

		SQLiteDatabase::~SQLiteDatabase()
		{
		}

		int SQLiteDatabase::getVersion() throw (SQLiteDatabase::SQLiteQueryException)
		{
			// Check version of SQLiteDB
			int version = 0;

			// prepare the statement
			Statement st(_database, QUERY_SCHEMAVERSION);

			// execute statement for version query
			int err = st.step();

			// Query finished no table found
			if (err == SQLITE_ROW)
			{
				std::string dbversion = (const char*) sqlite3_column_text(*st, 0);
				std::stringstream ss(dbversion);
				ss >> version;
			}

			return version;
		}

		void SQLiteDatabase::setVersion(int version) throw (SQLiteDatabase::SQLiteQueryException)
		{
			std::stringstream ss; ss << version;
			Statement st(_database, SET_SCHEMAVERSION);

			// bind version text to the statement
			sqlite3_bind_text(*st, 1, ss.str().c_str(), ss.str().length(), SQLITE_TRANSIENT);

			int err = st.step();
			if(err != SQLITE_DONE)
			{
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << "failed to set version " << err << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void SQLiteDatabase::doUpgrade(int oldVersion, int newVersion) throw (SQLiteDatabase::SQLiteQueryException)
		{
			if (oldVersion > newVersion)
			{
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << "Downgrade from version " << oldVersion << " to version " << newVersion << " is not possible." << IBRCOMMON_LOGGER_ENDL;
				throw ibrcommon::Exception("Downgrade not possible.");
			}

			IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, info) << "Upgrade from version " << oldVersion << " to version " << newVersion << IBRCOMMON_LOGGER_ENDL;

			for (int i = oldVersion; i < newVersion; ++i)
			{
				switch (i)
				{
				// if there is no version field, drop all tables
				case 0:
					for (size_t i = 0; i < SQL_TABLE_END; ++i)
					{
						Statement st(_database, "DROP TABLE IF EXISTS " + _tables[i] + ";");
						int err = st.step();
						if(err != SQLITE_DONE)
						{
							IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << "drop table " << _tables[i] << " failed " << err << IBRCOMMON_LOGGER_ENDL;
						}
					}

					// create all tables
					for (size_t i = 0; i < 11; ++i)
					{
						Statement st(_database, _db_structure[i]);
						int err = st.step();
						if(err != SQLITE_DONE)
						{
							IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << "failed to create table " << _tables[i] << "; err: " << err << IBRCOMMON_LOGGER_ENDL;
						}
					}

					// set new database version
					setVersion(1);

					break;

				case 1:
					Statement st(_database, "ALTER TABLE " + _tables[SQL_TABLE_BUNDLE] + " ADD `netpriority` INTEGER NOT NULL DEFAULT 0;");
					int err = st.step();
					if(err != SQLITE_DONE)
					{
						IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << "failed to alter table " << _tables[SQL_TABLE_BUNDLE] << "; err: " << err << IBRCOMMON_LOGGER_ENDL;
					}

					// set new database version
					setVersion(2);
					break;
				}
			}
		}

		void SQLiteDatabase::open() throw (SQLiteDatabase::SQLiteQueryException)
		{
			//Configure SQLite Library
			SQLiteConfigure::configure();

			// check if SQLite is thread-safe
			if (sqlite3_threadsafe() == 0)
			{
				IBRCOMMON_LOGGER(critical) << "sqlite library has not compiled with threading support." << IBRCOMMON_LOGGER_ENDL;
				throw ibrcommon::Exception("need threading support in sqlite!");
			}

			//open the database
			if (sqlite3_open_v2(_file.getPath().c_str(), &_database,  SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
			{
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << "Can't open database: " << sqlite3_errmsg(_database) << IBRCOMMON_LOGGER_ENDL;
				sqlite3_close(_database);
				throw ibrcommon::Exception("Unable to open sqlite database");
			}

			try {
				// check database version and upgrade if necessary
				int version = getVersion();

				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, info) << "Database version " << version << " found." << IBRCOMMON_LOGGER_ENDL;

				if (version != DBSCHEMA_VERSION)
				{
					doUpgrade(version, DBSCHEMA_VERSION);
				}
			} catch (const SQLiteQueryException &ex) {
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, warning) << "upgrade failed, start-over with a fresh database" << IBRCOMMON_LOGGER_ENDL;
				doUpgrade(0, DBSCHEMA_VERSION);
			}

			// disable synchronous mode
			sqlite3_exec(_database, "PRAGMA synchronous = OFF;", NULL, NULL, NULL);

			// enable sqlite tracing if debug level is higher than 50
			if (IBRCOMMON_LOGGER_LEVEL >= 50)
			{
				sqlite3_trace(_database, &sql_tracer, NULL);
			}

			// calculate next Bundleexpiredtime
			update_expire_time();
		}

		void SQLiteDatabase::close()
		{
			//close Databaseconnection
			if (sqlite3_close(_database) != SQLITE_OK)
			{
				IBRCOMMON_LOGGER(error) << "unable to close database" << IBRCOMMON_LOGGER_ENDL;
			}

			// shutdown sqlite library
			SQLiteConfigure::shutdown();
		}

		void SQLiteDatabase::get(const dtn::data::BundleID &id, dtn::data::MetaBundle &meta) const throw (SQLiteDatabase::SQLiteQueryException, NoBundleFoundException)
		{
			// check if the bundle is already on the deletion list
			if (contains_deletion(id)) throw dtn::storage::NoBundleFoundException();

			size_t stmt_key = BUNDLE_GET_ID;
			if (id.fragment) stmt_key = FRAGMENT_GET_ID;

			// lock the prepared statement
			Statement st(_database, _sql_queries[stmt_key]);

			// bind bundle id to the statement
			set_bundleid(st, id);

			// execute the query and check for error
			if (st.step() != SQLITE_ROW)
			{
				stringstream error;
				error << "No Bundle found with BundleID: " << id.toString();
				IBRCOMMON_LOGGER_DEBUG_TAG(SQLiteDatabase::TAG, 15) << error.str() << IBRCOMMON_LOGGER_ENDL;
				throw SQLiteQueryException(error.str());
			}

			// query bundle data
			get(st, meta);
		}

		void SQLiteDatabase::get(Statement &st, dtn::data::MetaBundle &bundle, size_t offset) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			bundle.source = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 0) );
			bundle.destination = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 1) );
			bundle.reportto = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 2) );
			bundle.custodian = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 3) );
			bundle.procflags = sqlite3_column_int(*st, offset + 4);
			bundle.timestamp = sqlite3_column_int64(*st, offset + 5);
			bundle.sequencenumber = sqlite3_column_int64(*st, offset + 6);
			bundle.lifetime = sqlite3_column_int64(*st, offset + 7);
			bundle.expiretime = dtn::utils::Clock::getExpireTime(bundle.timestamp, bundle.lifetime);

			if (bundle.procflags & data::Bundle::FRAGMENT)
			{
				bundle.offset = sqlite3_column_int64(*st, offset + 8);
				bundle.appdatalength = sqlite3_column_int64(*st, offset + 9);
			}

			if (sqlite3_column_type(*st, offset + 10) != SQLITE_NULL)
			{
				bundle.hopcount = sqlite3_column_int64(*st, 10);
			}

			bundle.payloadlength = sqlite3_column_int64(*st, 11);

			// restore net priority
			bundle.net_priority = sqlite3_column_int64(*st, 12);
		}

		void SQLiteDatabase::get(Statement &st, dtn::data::Bundle &bundle, size_t offset) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			bundle._source = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 0) );
			bundle._destination = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 1) );
			bundle._reportto = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 2) );
			bundle._custodian = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 3) );
			bundle._procflags = sqlite3_column_int(*st, offset + 4);
			bundle.timestamp = sqlite3_column_int64(*st, offset + 5);
			bundle._sequencenumber = sqlite3_column_int64(*st, offset + 6);
			bundle._lifetime = sqlite3_column_int64(*st, offset + 7);

			if (bundle._procflags & data::Bundle::FRAGMENT)
			{
				bundle._fragmentoffset = sqlite3_column_int64(*st, offset + 8);
				bundle._appdatalength = sqlite3_column_int64(*st, offset + 9);
			}
		}

		void SQLiteDatabase::iterateAll() throw (SQLiteDatabase::SQLiteQueryException)
		{
			Statement st(_database, _sql_queries[BUNDLE_GET_ITERATOR]);

			// abort if enough bundles are found
			while (st.step() == SQLITE_ROW)
			{
				dtn::data::MetaBundle m;

				// extract the primary values and set them in the bundle object
				get(st, m, 0);

				// call iteration callback
				_listener.iterateDatabase(m);
			}

			st.reset();
		}

		void SQLiteDatabase::get(BundleSelector &cb, BundleResult &ret) throw (NoBundleFoundException, BundleSelectorException, BundleSelectorException)
		{
			size_t items_added = 0;

			const std::string base_query =
					"SELECT " + _select_names[0] + " FROM " + _tables[SQL_TABLE_BUNDLE];

			size_t offset = 0;
			bool unlimited = (cb.limit() <= 0);
			size_t query_limit = (cb.limit() > 0) ? cb.limit() : 10;

			try {
				try {
					SQLBundleQuery &query = dynamic_cast<SQLBundleQuery&>(cb);

					// custom query string
					std::string query_string = base_query + " WHERE " + query.getWhere() + " ORDER BY priority DESC, timestamp, sequencenumber, fragmentoffset LIMIT ?,?;";

					// create statement for custom query
					Statement st(_database, query_string);

					while (unlimited || (items_added < query_limit))
					{
						// bind the statement parameter
						size_t bind_offset = query.bind(*st, 1);

						// query the database
						__get(cb, st, ret, items_added, bind_offset, offset);

						// increment the offset, because we might not have enough
						offset += query_limit;
					}
				} catch (const std::bad_cast&) {
					Statement st(_database, _sql_queries[BUNDLE_GET_FILTER]);

					while (unlimited || (items_added < query_limit))
					{
						// query the database
						__get(cb, st, ret, items_added, 1, offset);

						// increment the offset, because we might not have enough
						offset += query_limit;
					}
				}
			} catch (const SQLiteDatabase::SQLiteQueryException &ex) {
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, critical) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			} catch (const dtn::storage::NoBundleFoundException&) { }

			if (items_added == 0) throw dtn::storage::NoBundleFoundException();
		}

		void SQLiteDatabase::__get(const BundleSelector &cb, Statement &st, BundleResult &ret, size_t &items_added, size_t bind_offset, size_t offset) const throw (SQLiteDatabase::SQLiteQueryException, NoBundleFoundException, BundleSelectorException)
		{
			bool unlimited = (cb.limit() <= 0);
			size_t query_limit = (cb.limit() > 0) ? cb.limit() : 10;

			// limit result according to callback result
			sqlite3_bind_int64(*st, bind_offset, offset);
			sqlite3_bind_int64(*st, bind_offset + 1, query_limit);

			// returned no result
			if (st.step() == SQLITE_DONE)
				throw dtn::storage::NoBundleFoundException();

			// abort if enough bundles are found
			while (unlimited || (items_added < query_limit))
			{
				dtn::data::MetaBundle m;

				// extract the primary values and set them in the bundle object
				get(st, m, 0);

				// check if the bundle is already on the deletion list
				if (!contains_deletion(m))
				{
					// ask the filter if this bundle should be added to the return list
					if (cb.shouldAdd(m))
					{
						IBRCOMMON_LOGGER_DEBUG(40) << "add bundle to query selection list: " << m.toString() << IBRCOMMON_LOGGER_ENDL;

						// add the bundle to the list
						ret.put(m);
						items_added++;
					}
				}

				if (st.step() != SQLITE_ROW) break;
			}

			st.reset();
		}

		void SQLiteDatabase::get(const dtn::data::BundleID &id, dtn::data::Bundle &bundle, blocklist &blocks) const throw (SQLiteDatabase::SQLiteQueryException, NoBundleFoundException)
		{
			int err = 0;

			IBRCOMMON_LOGGER_DEBUG(25) << "get bundle from sqlite storage " << id.toString() << IBRCOMMON_LOGGER_ENDL;

			// if bundle is deleted?
			if (contains_deletion(id)) throw dtn::storage::NoBundleFoundException();

			size_t stmt_key = BUNDLE_GET_ID;
			if (id.fragment) stmt_key = FRAGMENT_GET_ID;

			// do this while db is locked
			Statement st(_database, _sql_queries[stmt_key]);

			// set the bundle key values
			set_bundleid(st, id);

			// execute the query and check for error
			if ((err = st.step()) != SQLITE_ROW)
			{
				IBRCOMMON_LOGGER_DEBUG_TAG(SQLiteDatabase::TAG, 15) << "sql error: " << err << "; No bundle found with id: " << id.toString() << IBRCOMMON_LOGGER_ENDL;
				throw dtn::storage::NoBundleFoundException();
			}

			// read bundle data
			get(st, bundle);

			try {

				int err = 0;
				string file;

				// select the right statement to use
				const size_t stmt_key = id.fragment ? BLOCK_GET_ID_FRAGMENT : BLOCK_GET_ID;

				Statement st(_database, _sql_queries[stmt_key]);

				// set the bundle key values
				set_bundleid(st, id);

				// query the database and step through all blocks
				while ((err = st.step()) == SQLITE_ROW)
				{
					const ibrcommon::File f( (const char*) sqlite3_column_text(*st, 0) );
					int blocktyp = sqlite3_column_int(*st, 1);

					blocks.push_back( blocklist_entry(blocktyp, f) );
				}

				if (err == SQLITE_DONE)
				{
					if (blocks.size() == 0)
					{
						IBRCOMMON_LOGGER(error) << "get_blocks: no blocks found for: " << id.toString() << IBRCOMMON_LOGGER_ENDL;
						throw SQLiteQueryException("no blocks found");
					}
				}
				else
				{
					IBRCOMMON_LOGGER(error) << "get_blocks() failure: "<< err << " " << sqlite3_errmsg(_database) << IBRCOMMON_LOGGER_ENDL;
					throw SQLiteQueryException("can not query for blocks");
				}

			} catch (const ibrcommon::Exception &ex) {
				IBRCOMMON_LOGGER(error) << "could not get bundle blocks: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				throw dtn::storage::NoBundleFoundException();
			}
		}

		void SQLiteDatabase::store(const dtn::data::Bundle &bundle) throw (SQLiteDatabase::SQLiteQueryException)
		{
			int err;

			const dtn::data::EID _sourceid = bundle._source;
			size_t TTL = bundle.timestamp + bundle._lifetime;

			Statement st(_database, _sql_queries[BUNDLE_STORE]);

			set_bundleid(st, bundle);

			sqlite3_bind_text(*st, 5, bundle._source.getString().c_str(), bundle._source.getString().length(), SQLITE_TRANSIENT);
			sqlite3_bind_text(*st, 6, bundle._destination.getString().c_str(), bundle._destination.getString().length(), SQLITE_TRANSIENT);
			sqlite3_bind_text(*st, 7, bundle._reportto.getString().c_str(), bundle._reportto.getString().length(), SQLITE_TRANSIENT);
			sqlite3_bind_text(*st, 8, bundle._custodian.getString().c_str(), bundle._custodian.getString().length(), SQLITE_TRANSIENT);
			sqlite3_bind_int(*st, 9, bundle._procflags);
			sqlite3_bind_int64(*st, 10, bundle._lifetime);

			if (bundle.get(dtn::data::Bundle::FRAGMENT))
			{
				sqlite3_bind_int64(*st, 11, bundle._appdatalength);
			}
			else
			{
				sqlite3_bind_null(*st, 4);
				sqlite3_bind_null(*st, 11);
			}

			sqlite3_bind_int64(*st, 12, TTL);
			sqlite3_bind_int64(*st, 13, dtn::data::MetaBundle(bundle).getPriority());

			try {
				const dtn::data::ScopeControlHopLimitBlock &schl = bundle.find<const dtn::data::ScopeControlHopLimitBlock>();
				sqlite3_bind_int64(*st, 14, schl.getHopsToLive() );
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				sqlite3_bind_null(*st, 14 );
			}

			try {
				const dtn::data::PayloadBlock &pblock = bundle.find<const dtn::data::PayloadBlock>();
				sqlite3_bind_int64(*st, 15, pblock.getLength() );
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				sqlite3_bind_int64(*st, 15, 0 );
			}

			try {
				const dtn::data::SchedulingBlock &sched = bundle.find<const dtn::data::SchedulingBlock>();
				sqlite3_bind_int64(*st, 16, sched.getPriority() );
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				sqlite3_bind_int64(*st, 16, 0 );
			}

			err = st.step();

			if (err == SQLITE_CONSTRAINT)
			{
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, warning) << "Bundle is already in the storage" << IBRCOMMON_LOGGER_ENDL;

				stringstream error;
				error << "store() failure: " << err << " " <<  sqlite3_errmsg(_database);
				throw SQLiteQueryException(error.str());
			}
			else if (err != SQLITE_DONE)
			{
				stringstream error;
				error << "store() failure: " << err << " " <<  sqlite3_errmsg(_database);
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << error.str() << IBRCOMMON_LOGGER_ENDL;

				throw SQLiteQueryException(error.str());
			}

			// set new expire time
			new_expire_time(TTL);
		}

		void SQLiteDatabase::store(const dtn::data::BundleID &id, int index, const dtn::data::Block &block, const ibrcommon::File &file) throw (SQLiteDatabase::SQLiteQueryException)
		{
			int blocktyp = (int)block.getType();

			// protect this query from concurrent access and enable the auto-reset feature
			Statement st(_database, _sql_queries[BLOCK_STORE]);

			// set bundle key data
			set_bundleid(st, id);

			// set the column four to null if this is not a fragment
			if (!id.fragment) sqlite3_bind_null(*st, 4);

			// set the block type
			sqlite3_bind_int(*st, 5, blocktyp);

			// the filename of the block data
			sqlite3_bind_text(*st, 6, file.getPath().c_str(), file.getPath().size(), SQLITE_TRANSIENT);

			// the ordering number
			sqlite3_bind_int(*st, 7, index);

			// execute the query and store the block in the database
			if (st.step() != SQLITE_DONE)
			{
				throw SQLiteQueryException("can not store block of bundle");
			}
		}

		void SQLiteDatabase::transaction() throw (SQLiteDatabase::SQLiteQueryException)
		{
			char *zErrMsg = 0;

			// start a transaction
			int ret = sqlite3_exec(_database, "BEGIN TRANSACTION;", NULL, NULL, &zErrMsg);

			// check if the return value signals an error
			if ( ret != SQLITE_OK )
			{
				sqlite3_free( zErrMsg );
				throw SQLiteQueryException( zErrMsg );
			}
		}

		void SQLiteDatabase::rollback() throw (SQLiteDatabase::SQLiteQueryException)
		{
			char *zErrMsg = 0;

			// rollback the whole transaction
			int ret = sqlite3_exec(_database, "ROLLBACK TRANSACTION;", NULL, NULL, &zErrMsg);

			// check if the return value signals an error
			if ( ret != SQLITE_OK )
			{
				sqlite3_free( zErrMsg );
				throw SQLiteQueryException( zErrMsg );
			}
		}

		void SQLiteDatabase::commit() throw (SQLiteDatabase::SQLiteQueryException)
		{
			char *zErrMsg = 0;

			// commit the transaction
			int ret = sqlite3_exec(_database, "END TRANSACTION;", NULL, NULL, &zErrMsg);

			// check if the return value signals an error
			if ( ret != SQLITE_OK )
			{
				sqlite3_free( zErrMsg );
				throw SQLiteQueryException( zErrMsg );
			}
		}

		void SQLiteDatabase::remove(const dtn::data::BundleID &id) throw (SQLiteDatabase::SQLiteQueryException)
		{
			{
				// select the right statement to use
				const size_t stmt_key = id.fragment ? BLOCK_GET_ID_FRAGMENT : BLOCK_GET_ID;

				// lock the database
				Statement st(_database, _sql_queries[stmt_key]);

				// set the bundle key values
				set_bundleid(st, id);

				// step through all blocks
				while (st.step() == SQLITE_ROW)
				{
					// delete each referenced block file
					ibrcommon::File blockfile( (const char*)sqlite3_column_text(*st, 0) );
					blockfile.remove();
				}
			}

			{
				const size_t stmt_key = id.fragment ? FRAGMENT_DELETE : BUNDLE_DELETE;

				// lock the database
				Statement st(_database, _sql_queries[stmt_key]);

				// then remove the bundle data
				set_bundleid(st, id);
				st.step();

				IBRCOMMON_LOGGER_DEBUG(10) << "bundle " << id.toString() << " deleted" << IBRCOMMON_LOGGER_ENDL;
			}

			//update deprecated timer
			update_expire_time();

			// remove it from the deletion list
			remove_deletion(id);
		}

		void SQLiteDatabase::clear() throw (SQLiteDatabase::SQLiteQueryException)
		{
			Statement vacuum(_database, _sql_queries[VACUUM]);
			Statement bundle_clear(_database, _sql_queries[BUNDLE_CLEAR]);
			Statement block_clear(_database, _sql_queries[BLOCK_CLEAR]);

			bundle_clear.step();
			block_clear.step();

			if (SQLITE_DONE != vacuum.step())
			{
				throw SQLiteQueryException("SQLiteBundleStore: clear(): vacuum failed.");
			}

			reset_expire_time();
		}

		bool SQLiteDatabase::empty() const throw (SQLiteDatabase::SQLiteQueryException)
		{
			Statement st(_database, _sql_queries[EMPTY_CHECK]);

			if (SQLITE_DONE == st.step())
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		size_t SQLiteDatabase::count() const throw (SQLiteDatabase::SQLiteQueryException)
		{
			size_t rows = 0;
			int err = 0;

			Statement st(_database, _sql_queries[COUNT_ENTRIES]);

			if ((err = st.step()) == SQLITE_ROW)
			{
				rows = sqlite3_column_int(*st, 0);
			}
			else
			{
				stringstream error;
				error << "count: failure " << err << " " << sqlite3_errmsg(_database);
				throw SQLiteQueryException(error.str());
			}

			return rows;
		}

		const std::set<dtn::data::EID> SQLiteDatabase::getDistinctDestinations() throw (SQLiteDatabase::SQLiteQueryException)
		{
			std::set<dtn::data::EID> ret;
			// TODO: implement this method!
			return ret;
		}

		void SQLiteDatabase::update_expire_time() throw (SQLiteDatabase::SQLiteQueryException)
		{
			Statement st(_database, _sql_queries[EXPIRE_NEXT_TIMESTAMP]);

			int err = st.step();

			if (err == SQLITE_ROW)
			{
				_next_expiration = sqlite3_column_int64(*st, 0);
			}
			else
			{
				_next_expiration = 0;
			}
		}

		void SQLiteDatabase::expire(size_t timestamp) throw ()
		{
			/*
			 * Only if the actual time is bigger or equal than the time when the next bundle expires, deleteexpired is called.
			 */
			size_t exp_time = get_expire_time();
			if ((timestamp < exp_time) || (exp_time == 0)) return;

			/*
			 * Performanceverbesserung: Damit die Abfragen nicht jede Sekunde ausgeführt werden müssen, speichert man den Zeitpunkt an dem
			 * das nächste Bündel gelöscht werden soll in eine Variable und führt deleteexpired erst dann aus wenn ein Bündel abgelaufen ist.
			 * Nach dem Löschen wird die DB durchsucht und der nächste Ablaufzeitpunkt wird in die Variable gesetzt.
			 */

			try {
				Statement st(_database, _sql_queries[EXPIRE_BUNDLE_FILENAMES]);

				// query for blocks of expired bundles
				sqlite3_bind_int64(*st, 1, timestamp);
				while (st.step() == SQLITE_ROW)
				{
					ibrcommon::File block((const char*)sqlite3_column_text(*st,0));
					block.remove();
				}
			} catch (const SQLiteDatabase::SQLiteQueryException &ex) {
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			try {
				Statement st(_database, _sql_queries[EXPIRE_BUNDLES]);

				// query expired bundles
				sqlite3_bind_int64(*st, 1, timestamp);
				while (st.step() == SQLITE_ROW)
				{
					dtn::data::BundleID id;
					get_bundleid(st, id);
					dtn::core::BundleExpiredEvent::raise(id);

					// raise bundle removed event
					_listener.eventBundleExpired(id);
				}
			} catch (const SQLiteDatabase::SQLiteQueryException &ex) {
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			try {
				Statement st(_database, _sql_queries[EXPIRE_BUNDLE_DELETE]);

				// delete all expired db entries (bundles and blocks)
				sqlite3_bind_int64(*st, 1, timestamp);
				st.step();
			} catch (const SQLiteDatabase::SQLiteQueryException &ex) {
				IBRCOMMON_LOGGER_TAG(SQLiteDatabase::TAG, error) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			//update deprecated timer
			update_expire_time();
		}

		void SQLiteDatabase::vacuum() throw (SQLiteDatabase::SQLiteQueryException)
		{
			Statement st(_database, _sql_queries[VACUUM]);
			st.step();
		}

		void SQLiteDatabase::update(UPDATE_VALUES mode, const dtn::data::BundleID &id, const dtn::data::EID &eid) throw (SQLiteDatabase::SQLiteQueryException)
		{
			if (mode == UPDATE_CUSTODIAN)
			{
				// select query with or without fragmentation extension
				STORAGE_STMT query = BUNDLE_UPDATE_CUSTODIAN;
				if (id.fragment)	query = FRAGMENT_UPDATE_CUSTODIAN;

				Statement st(_database, _sql_queries[query]);

				sqlite3_bind_text(*st, 1, eid.getString().c_str(), eid.getString().length(), SQLITE_TRANSIENT);
				set_bundleid(st, id, 1);

				// update the custodian in the database
				int err = st.step();

				if (err != SQLITE_DONE)
				{
					stringstream error;
					error << "update_custodian() failure: " << err << " " <<  sqlite3_errmsg(_database);
					throw SQLiteDatabase::SQLiteQueryException(error.str());
				}
			}
		}

		void SQLiteDatabase::new_expire_time(size_t ttl) throw ()
		{
			if (_next_expiration == 0 || ttl < _next_expiration)
			{
				_next_expiration = ttl;
			}
		}

		void SQLiteDatabase::reset_expire_time() throw ()
		{
			_next_expiration = 0;
		}

		size_t SQLiteDatabase::get_expire_time() const throw ()
		{
			return _next_expiration;
		}

		void SQLiteDatabase::add_deletion(const dtn::data::BundleID &id) throw ()
		{
			_deletion_list.insert(id);
		}

		void SQLiteDatabase::remove_deletion(const dtn::data::BundleID &id) throw ()
		{
			_deletion_list.erase(id);
		}

		bool SQLiteDatabase::contains_deletion(const dtn::data::BundleID &id) const throw ()
		{
			return (_deletion_list.find(id) != _deletion_list.end());
		}

		void SQLiteDatabase::set_bundleid(Statement &st, const dtn::data::BundleID &id, size_t offset) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			const std::string source_id = id.source.getString();
			sqlite3_bind_text(*st, offset + 1, source_id.c_str(), source_id.length(), SQLITE_TRANSIENT);
			sqlite3_bind_int64(*st, offset + 2, id.timestamp);
			sqlite3_bind_int64(*st, offset + 3, id.sequencenumber);

			if (id.fragment)
			{
				sqlite3_bind_int64(*st, offset + 4, id.offset);
			}
		}

		void SQLiteDatabase::get_bundleid(Statement &st, dtn::data::BundleID &id, size_t offset) const throw (SQLiteDatabase::SQLiteQueryException)
		{
			id.source = dtn::data::EID((const char*)sqlite3_column_text(*st, offset + 0));
			id.timestamp = sqlite3_column_int64(*st, offset + 1);
			id.sequencenumber = sqlite3_column_int64(*st, offset + 2);

			id.fragment = (sqlite3_column_text(*st, offset + 3) != NULL);
			id.offset = sqlite3_column_int64(*st, offset + 3);
		}
	} /* namespace storage */
} /* namespace dtn */
