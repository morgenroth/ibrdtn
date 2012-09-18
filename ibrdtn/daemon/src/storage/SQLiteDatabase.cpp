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

		const std::string SQLiteDatabase::_tables[SQL_TABLE_END] =
				{ "bundles", "blocks", "routing", "routing_bundles", "routing_nodes", "properties" };

		const int SQLiteDatabase::DBSCHEMA_VERSION = 1;
		const std::string SQLiteDatabase::QUERY_SCHEMAVERSION = "SELECT `value` FROM " + SQLiteDatabase::_tables[SQLiteDatabase::SQL_TABLE_PROPERTIES] + " WHERE `key` = 'version' LIMIT 0,1;";
		const std::string SQLiteDatabase::SET_SCHEMAVERSION = "INSERT INTO " + SQLiteDatabase::_tables[SQLiteDatabase::SQL_TABLE_PROPERTIES] + " (`key`, `value`) VALUES ('version', ?);";

		const std::string SQLiteDatabase::_sql_queries[SQL_QUERIES_END] =
		{
			"SELECT source, destination, reportto, custodian, procflags, timestamp, sequencenumber, lifetime, fragmentoffset, appdatalength, hopcount, payloadlength FROM " + _tables[SQL_TABLE_BUNDLE] + " ORDER BY priority DESC LIMIT ?,?;",
			"SELECT source, destination, reportto, custodian, procflags, timestamp, sequencenumber, lifetime, fragmentoffset, appdatalength, hopcount, payloadlength FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset IS NULL LIMIT 1;",
			"SELECT source, destination, reportto, custodian, procflags, timestamp, sequencenumber, lifetime, fragmentoffset, appdatalength, hopcount, payloadlength FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ? LIMIT 1;",
			"SELECT * FROM " + _tables[SQL_TABLE_BUNDLE] + " WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset != NULL ORDER BY fragmentoffset ASC;",

			"SELECT source, timestamp, sequencenumber, fragmentoffset, procflags FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE expiretime <= ?;",
			"SELECT filename FROM "+ _tables[SQL_TABLE_BUNDLE] +" as a, "+ _tables[SQL_TABLE_BLOCK] +" as b WHERE a.source_id = b.source_id AND a.timestamp = b.timestamp AND a.sequencenumber = b.sequencenumber AND ((a.fragmentoffset = b.fragmentoffset) OR ((a.fragmentoffset IS NULL) AND (b.fragmentoffset IS NULL))) AND a.expiretime <= ?;",
			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE expiretime <= ?;",
			"SELECT expiretime FROM "+ _tables[SQL_TABLE_BUNDLE] +" ORDER BY expiretime ASC LIMIT 1;",

			"SELECT ROWID FROM "+ _tables[SQL_TABLE_BUNDLE] +" LIMIT 1;",
			"SELECT COUNT(ROWID) FROM "+ _tables[SQL_TABLE_BUNDLE] +";",

			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset IS NULL;",
			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +" WHERE source_id = ? AND timestamp = ? AND sequencenumber = ? AND fragmentoffset = ?;",
			"DELETE FROM "+ _tables[SQL_TABLE_BUNDLE] +";",
			"INSERT INTO "+ _tables[SQL_TABLE_BUNDLE] +" (source_id, timestamp, sequencenumber, fragmentoffset, source, destination, reportto, custodian, procflags, lifetime, appdatalength, expiretime, priority, hopcount, payloadlength) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);",
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

		void SQLiteDatabase::Statement::reset()
		{
			if (_st != NULL) {
				sqlite3_reset(_st);
				sqlite3_clear_bindings(_st);
			}
		}

		int SQLiteDatabase::Statement::step()
		{
			if (_st == NULL)
				throw SQLiteQueryException("statement not prepared");

			return sqlite3_step(_st);
		}

		void SQLiteDatabase::Statement::prepare()
		{
			if (_st != NULL)
				throw SQLiteQueryException("already prepared");

			int err = sqlite3_prepare_v2(_database, _query.c_str(), _query.length(), &_st, 0);

			if ( err != SQLITE_OK )
				throw SQLiteQueryException("failed to prepare statement: " + _query);
		}

		SQLiteDatabase::SQLiteDatabase(const ibrcommon::File &file)
		 : _file(file), _next_expiration(0)
		{
		}

		SQLiteDatabase::~SQLiteDatabase()
		{
		}

		int SQLiteDatabase::getVersion()
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

		void SQLiteDatabase::setVersion(int version)
		{
			std::stringstream ss; ss << version;
			Statement st(_database, SET_SCHEMAVERSION);

			// bind version text to the statement
			sqlite3_bind_text(*st, 1, ss.str().c_str(), ss.str().length(), SQLITE_TRANSIENT);

			int err = st.step();
			if(err != SQLITE_DONE)
			{
				IBRCOMMON_LOGGER(error) << "SQLiteDatabase: failed to set version " << err << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void SQLiteDatabase::doUpgrade(int oldVersion, int newVersion)
		{
			if (oldVersion > newVersion)
			{
				IBRCOMMON_LOGGER(error) << "SQLiteDatabase: Downgrade from version " << oldVersion << " to version " << newVersion << " is not possible." << IBRCOMMON_LOGGER_ENDL;
				throw ibrcommon::Exception("Downgrade not possible.");
			}

			IBRCOMMON_LOGGER(info) << "SQLiteDatabase: Upgrade from version " << oldVersion << " to version " << newVersion << IBRCOMMON_LOGGER_ENDL;

			for (int i = oldVersion; i < newVersion; i++)
			{
				switch (i)
				{
				// if there is no version field, drop all tables
				case 0:
					for (size_t i = 0; i < SQL_TABLE_END; i++)
					{
						Statement st(_database, "DROP TABLE IF EXISTS " + _tables[i] + ";");
						int err = st.step();
						if(err != SQLITE_DONE)
						{
							IBRCOMMON_LOGGER(error) << "SQLiteDatabase: drop table " << _tables[i] << " failed " << err << IBRCOMMON_LOGGER_ENDL;
						}
					}

					// create all tables
					for (size_t i = 0; i < 11; i++)
					{
						Statement st(_database, _db_structure[i]);
						int err = st.step();
						if(err != SQLITE_DONE)
						{
							IBRCOMMON_LOGGER(error) << "SQLiteDatabase: failed to create table " << _tables[i] << "; err: " << err << IBRCOMMON_LOGGER_ENDL;
						}
					}

					// set new database version
					setVersion(1);

					break;
				}
			}
		}

		void SQLiteDatabase::open()
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
				IBRCOMMON_LOGGER(error) << "Can't open database: " << sqlite3_errmsg(_database) << IBRCOMMON_LOGGER_ENDL;
				sqlite3_close(_database);
				throw ibrcommon::Exception("Unable to open sqlite database");
			}

			try {
				// check database version and upgrade if necessary
				int version = getVersion();

				IBRCOMMON_LOGGER(info) << "SQLiteDatabase: Database version " << version << " found." << IBRCOMMON_LOGGER_ENDL;

				if (version != DBSCHEMA_VERSION)
				{
					doUpgrade(version, DBSCHEMA_VERSION);
				}
			} catch (const SQLiteQueryException&) {
				doUpgrade(0, DBSCHEMA_VERSION);
			}

			// disable synchronous mode
			sqlite3_exec(_database, "PRAGMA synchronous = OFF;", NULL, NULL, NULL);

			// enable sqlite tracing if debug level is higher than 50
			if (IBRCOMMON_LOGGER_LEVEL >= 50)
			{
				sqlite3_trace(_database, &sql_tracer, NULL);
			}

//			//Check if the database is consistent with the filesystem
//			check_consistency();

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

//		void SQLiteDatabase::check_consistency()
//		{
//			/*
//			 * check if the database is consistent with the files on the HDD. Therefore every file of the database is put in a list.
//			 * When the files of the database are scanned it is checked if the actual file is in the list of files of the filesystem. if it is so the entry
//			 * of the list of files of the filesystem is deleted. if not the file is put in a seperate list. After this procedure there are 2 lists containing file
//			 * which are inconsistent and can be deleted.
//			 */
//			set<string> blockFiles,payloadfiles;
//			string datei;
//
//			list<ibrcommon::File> blist;
//			list<ibrcommon::File>::iterator file_it;
//
//			//1. Durchsuche die Dateien
//			_blockPath.getFiles(blist);
//			for(file_it = blist.begin(); file_it != blist.end(); file_it++)
//			{
//				datei = (*file_it).getPath();
//				if(datei[datei.size()-1] != '.')
//				{
//					if(((*file_it).getBasename())[0] == 'b')
//					{
//						blockFiles.insert(datei);
//					}
//					else
//						payloadfiles.insert(datei);
//				}
//			}
//
//			//3. Check consistency of the bundles
//			check_bundles(blockFiles);
//
//			//4. Check consistency of the fragments
//			check_fragments(payloadfiles);
//		}

//		void SQLiteDatabase::check_fragments(std::set<std::string> &payloadFiles)
//		{
//			int err;
//			size_t timestamp, sequencenumber, fragmentoffset;
//			string filename, source, dest, custody, repto;
//			set<string> consistenFiles, inconsistenSources;
//			set<size_t> inconsistentTimestamp, inconsistentSeq_number;
//			set<string>::iterator file_it, consisten_it;
//
//			sqlite3_stmt *getPayloadfiles = prepare("SELECT source, destination, reportto, custodian, procflags, timestamp, sequencenumber, lifetime, fragmentoffset, appdatalength, hopcount FROM "+_tables[SQL_TABLE_BUNDLE]+" WHERE fragmentoffset != NULL;");
//
//			for(err = sqlite3_step(getPayloadfiles); err == SQLITE_ROW; err = sqlite3_step(getPayloadfiles))
//			{
//				filename = (const char*)sqlite3_column_text(getPayloadfiles,10);
//				file_it = payloadFiles.find(filename);
//
//
//				if (file_it == payloadFiles.end())
//				{
//					consisten_it = consistenFiles.find(*file_it);
//
//					//inconsistent DB entry
//					if(consisten_it == consistenFiles.end())
//					{
//						//Generate Report
//						source  = (const char*) sqlite3_column_text(getPayloadfiles, 0);
//						dest    = (const char*) sqlite3_column_text(getPayloadfiles, 1);
//						repto   = (const char*) sqlite3_column_text(getPayloadfiles, 2);
//						custody = (const char*) sqlite3_column_text(getPayloadfiles, 3);
//
//						timestamp = sqlite3_column_int64(getPayloadfiles, 5);
//						sequencenumber = sqlite3_column_int64(getPayloadfiles, 6);
//						fragmentoffset = sqlite3_column_int64(getPayloadfiles, 8);
//
//						dtn::data::BundleID id( dtn::data::EID(source), timestamp, sequencenumber, true, fragmentoffset );
//						dtn::data::MetaBundle mb(id);
//
//						mb.procflags = sqlite3_column_int64(getPayloadfiles, 4);
//						mb.lifetime = sqlite3_column_int64(getPayloadfiles, 7);
//						mb.destination = dest;
//						mb.reportto = repto;
//						mb.custodian = custody;
//						mb.appdatalength = sqlite3_column_int64(getPayloadfiles, 9);
//						mb.expiretime = dtn::utils::Clock::getExpireTime(timestamp, mb.lifetime);
//
//						if (sqlite3_column_type(getPayloadfiles, 10) != SQLITE_NULL)
//						{
//							mb.hopcount = sqlite3_column_int64(getPayloadfiles, 10);
//						}
//
//						dtn::core::BundleEvent::raise(mb, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::DEPLETED_STORAGE);
//					}
//				}
//				//consistent DB entry
//				else
//					consistenFiles.insert(*file_it);
//					payloadFiles.erase(file_it);
//			}
//
//			sqlite3_reset(getPayloadfiles);
//			sqlite3_finalize(getPayloadfiles);
//		}
//
//		void SQLiteDatabase::check_bundles(std::set<std::string> &blockFiles)
//		{
//			std::set<dtn::data::BundleID> corrupt_bundle_ids;
//
//			sqlite3_stmt *blockConistencyCheck = prepare("SELECT source_id, timestamp, sequencenumber, fragmentoffset, filename, ordernumber FROM "+ _tables[SQL_TABLE_BLOCK] +";");
//
//			while (sqlite3_step(blockConistencyCheck) == SQLITE_ROW)
//			{
//				dtn::data::BundleID id;
//
//				// get the bundleid
//				get_bundleid(blockConistencyCheck, id);
//
//				// get the filename
//				std::string filename = (const char*)sqlite3_column_text(blockConistencyCheck, 4);
//
//				// if the filename is not in the list of block files
//				if (blockFiles.find(filename) == blockFiles.end())
//				{
//					// add the bundle to the deletion list
//					corrupt_bundle_ids.insert(id);
//				}
//				else
//				{
//					// file is available, everything is fine
//					blockFiles.erase(filename);
//				}
//			}
//			sqlite3_reset(blockConistencyCheck);
//			sqlite3_finalize(blockConistencyCheck);
//
//			for(std::set<dtn::data::BundleID>::const_iterator iter = corrupt_bundle_ids.begin(); iter != corrupt_bundle_ids.end(); iter++)
//			{
//				try {
//					const dtn::data::BundleID &id = (*iter);
//
//					// get meta data of this bundle
//					const dtn::data::MetaBundle m = get_meta(id);
//
//					// remove the hole bundle
//					remove(id);
//				} catch (const dtn::storage::SQLiteDatabase::SQLiteQueryException&) { };
//			}
//
//			// delete block files still in the blockfile list
//			for (std::set<std::string>::const_iterator iter = blockFiles.begin(); iter != blockFiles.end(); iter++)
//			{
//				ibrcommon::File blockfile(*iter);
//				blockfile.remove();
//			}
//		}

		void SQLiteDatabase::get(const dtn::data::BundleID &id, dtn::data::MetaBundle &meta) const
		{
			// check if the bundle is already on the deletion list
			if (contains_deletion(id)) throw dtn::storage::BundleStorage::NoBundleFoundException();

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
				error << "SQLiteDatabase: No Bundle found with BundleID: " << id.toString();
				IBRCOMMON_LOGGER_DEBUG(15) << error.str() << IBRCOMMON_LOGGER_ENDL;
				throw SQLiteQueryException(error.str());
			}

			// query bundle data
			get(st, meta);
		}

		void SQLiteDatabase::get(Statement &st, dtn::data::MetaBundle &bundle, size_t offset) const
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
		}

		void SQLiteDatabase::get(Statement &st, dtn::data::Bundle &bundle, size_t offset) const
		{
			bundle._source = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 0) );
			bundle._destination = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 1) );
			bundle._reportto = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 2) );
			bundle._custodian = dtn::data::EID( (const char*) sqlite3_column_text(*st, offset + 3) );
			bundle._procflags = sqlite3_column_int(*st, offset + 4);
			bundle._timestamp = sqlite3_column_int64(*st, offset + 5);
			bundle._sequencenumber = sqlite3_column_int64(*st, offset + 6);
			bundle._lifetime = sqlite3_column_int64(*st, offset + 7);

			if (bundle._procflags & data::Bundle::FRAGMENT)
			{
				bundle._fragmentoffset = sqlite3_column_int64(*st, offset + 8);
				bundle._appdatalength = sqlite3_column_int64(*st, offset + 9);
			}
		}

		const std::list<dtn::data::MetaBundle> SQLiteDatabase::get(dtn::storage::BundleStorage::BundleFilterCallback &cb) const
		{
			std::list<dtn::data::MetaBundle> ret;

			const std::string base_query =
					"SELECT source, destination, reportto, custodian, procflags, timestamp, sequencenumber, lifetime, fragmentoffset, appdatalength, hopcount, payloadlength FROM " + _tables[SQL_TABLE_BUNDLE];

			size_t offset = 0;
			bool unlimited = (cb.limit() <= 0);
			size_t query_limit = (cb.limit() > 0) ? cb.limit() : 10;

			try {
				try {
					SQLBundleQuery &query = dynamic_cast<SQLBundleQuery&>(cb);

					// custom query string
					std::string query_string = base_query + " WHERE " + query.getWhere() + " ORDER BY priority DESC LIMIT ?,?;";

					// create statement for custom query
					Statement st(_database, query_string);

					while (unlimited || (ret.size() < query_limit))
					{
						// bind the statement parameter
						size_t bind_offset = query.bind(*st, 1);

						// query the database
						__get(cb, st, ret, bind_offset, offset);

						// increment the offset, because we might not have enough
						offset += query_limit;
					}
				} catch (const std::bad_cast&) {
					Statement st(_database, _sql_queries[BUNDLE_GET_FILTER]);

					while (unlimited || (ret.size() < query_limit))
					{
						// query the database
						__get(cb, st, ret, 1, offset);

						// increment the offset, because we might not have enough
						offset += query_limit;
					}
				}
			} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) { }

			return ret;
		}

		void SQLiteDatabase::__get(const dtn::storage::BundleStorage::BundleFilterCallback &cb, Statement &st, std::list<dtn::data::MetaBundle> &ret, size_t bind_offset, size_t offset) const
		{
			bool unlimited = (cb.limit() <= 0);
			size_t query_limit = (cb.limit() > 0) ? cb.limit() : 10;

			// limit result according to callback result
			sqlite3_bind_int64(*st, bind_offset, offset);
			sqlite3_bind_int64(*st, bind_offset + 1, query_limit);

			// returned no result
			if (st.step() == SQLITE_DONE)
				throw dtn::storage::BundleStorage::NoBundleFoundException();

			// abort if enough bundles are found
			while (unlimited || (ret.size() < query_limit))
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
						ret.push_back(m);
					}
				}

				if (st.step() != SQLITE_ROW) break;
			}

			st.reset();
		}

		void SQLiteDatabase::get(const dtn::data::BundleID &id, dtn::data::Bundle &bundle, blocklist &blocks) const
		{
			int err = 0;

			IBRCOMMON_LOGGER_DEBUG(25) << "get bundle from sqlite storage " << id.toString() << IBRCOMMON_LOGGER_ENDL;

			// if bundle is deleted?
			if (contains_deletion(id)) throw dtn::storage::BundleStorage::NoBundleFoundException();

			size_t stmt_key = BUNDLE_GET_ID;
			if (id.fragment) stmt_key = FRAGMENT_GET_ID;

			// do this while db is locked
			Statement st(_database, _sql_queries[stmt_key]);

			// set the bundle key values
			set_bundleid(st, id);

			// execute the query and check for error
			if ((err = st.step()) != SQLITE_ROW)
			{
				IBRCOMMON_LOGGER_DEBUG(15) << "sql error: " << err << "; No bundle found with id: " << id.toString() << IBRCOMMON_LOGGER_ENDL;
				throw dtn::storage::BundleStorage::NoBundleFoundException();
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
				throw dtn::storage::BundleStorage::NoBundleFoundException();
			}
		}

		void SQLiteDatabase::store(const dtn::data::Bundle &bundle)
		{
			int err;

			const dtn::data::EID _sourceid = bundle._source;
			size_t TTL = bundle._timestamp + bundle._lifetime;

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
				const dtn::data::ScopeControlHopLimitBlock &schl = bundle.getBlock<const dtn::data::ScopeControlHopLimitBlock>();
				sqlite3_bind_int64(*st, 14, schl.getHopsToLive() );
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				sqlite3_bind_null(*st, 14 );
			};

			try {
				const dtn::data::PayloadBlock &pblock = bundle.getBlock<const dtn::data::PayloadBlock>();
				sqlite3_bind_int64(*st, 15, pblock.getLength() );
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				sqlite3_bind_int64(*st, 15, 0 );
			};

			err = st.step();

			if (err == SQLITE_CONSTRAINT)
			{
				IBRCOMMON_LOGGER(warning) << "Bundle is already in the storage" << IBRCOMMON_LOGGER_ENDL;

				stringstream error;
				error << "SQLiteDatabase: store() failure: " << err << " " <<  sqlite3_errmsg(_database);
				throw SQLiteQueryException(error.str());
			}
			else if (err != SQLITE_DONE)
			{
				stringstream error;
				error << "SQLiteDatabase: store() failure: " << err << " " <<  sqlite3_errmsg(_database);
				IBRCOMMON_LOGGER(error) << error.str() << IBRCOMMON_LOGGER_ENDL;

				throw SQLiteQueryException(error.str());
			}

			// set new expire time
			new_expire_time(TTL);
		}

		void SQLiteDatabase::store(const dtn::data::BundleID &id, int index, const dtn::data::Block &block, const ibrcommon::File &file)
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

		void SQLiteDatabase::transaction()
		{
			// start a transaction
			sqlite3_exec(_database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
		}

		void SQLiteDatabase::rollback()
		{
			// rollback the whole transaction
			sqlite3_exec(_database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
		}

		void SQLiteDatabase::commit()
		{
			// commit the transaction
			sqlite3_exec(_database, "END TRANSACTION;", NULL, NULL, NULL);
		}

		void SQLiteDatabase::remove(const dtn::data::BundleID &id)
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

		void SQLiteDatabase::clear()
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

		bool SQLiteDatabase::empty() const
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

		unsigned int SQLiteDatabase::count() const
		{
			int rows = 0;
			int err = 0;

			Statement st(_database, _sql_queries[COUNT_ENTRIES]);

			if ((err = st.step()) == SQLITE_ROW)
			{
				rows = sqlite3_column_int(*st, 0);
			}
			else
			{
				stringstream error;
				error << "SQLiteDatabase: count: failure " << err << " " << sqlite3_errmsg(_database);
				IBRCOMMON_LOGGER(error) << error.str() << IBRCOMMON_LOGGER_ENDL;
				throw SQLiteQueryException(error.str());
			}

			return rows;
		}

		const std::set<dtn::data::EID> SQLiteDatabase::getDistinctDestinations()
		{
			std::set<dtn::data::EID> ret;
			return ret;
		}

		void SQLiteDatabase::update_expire_time()
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

		void SQLiteDatabase::expire(size_t timestamp)
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

			{
				Statement st(_database, _sql_queries[EXPIRE_BUNDLE_FILENAMES]);

				// query for blocks of expired bundles
				sqlite3_bind_int64(*st, 1, timestamp);
				while (st.step() == SQLITE_ROW)
				{
					ibrcommon::File block((const char*)sqlite3_column_text(*st,0));
					block.remove();
				}
			}

			{
				Statement st(_database, _sql_queries[EXPIRE_BUNDLES]);

				// query expired bundles
				sqlite3_bind_int64(*st, 1, timestamp);
				while (st.step() == SQLITE_ROW)
				{
					dtn::data::BundleID id;
					get_bundleid(st, id);
					dtn::core::BundleExpiredEvent::raise(id);
				}
			}

			{
				Statement st(_database, _sql_queries[EXPIRE_BUNDLE_DELETE]);

				// delete all expired db entries (bundles and blocks)
				sqlite3_bind_int64(*st, 1, timestamp);
				st.step();
			}

			//update deprecated timer
			update_expire_time();
		}

		void SQLiteDatabase::vacuum()
		{
			Statement st(_database, _sql_queries[VACUUM]);
			st.step();
		}

		void SQLiteDatabase::update(UPDATE_VALUES mode, const dtn::data::BundleID &id, const dtn::data::EID &eid)
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
					error << "SQLiteDatabase: update_custodian() failure: " << err << " " <<  sqlite3_errmsg(_database);
					IBRCOMMON_LOGGER(error) << error.str() << IBRCOMMON_LOGGER_ENDL;
				}
			}
		}

		void SQLiteDatabase::new_expire_time(size_t ttl)
		{
			if (_next_expiration == 0 || ttl < _next_expiration)
			{
				_next_expiration = ttl;
			}
		}

		void SQLiteDatabase::reset_expire_time()
		{
			_next_expiration = 0;
		}

		size_t SQLiteDatabase::get_expire_time()
		{
			return _next_expiration;
		}

		void SQLiteDatabase::add_deletion(const dtn::data::BundleID &id)
		{
			_deletion_list.insert(id);
		}

		void SQLiteDatabase::remove_deletion(const dtn::data::BundleID &id)
		{
			_deletion_list.erase(id);
		}

		bool SQLiteDatabase::contains_deletion(const dtn::data::BundleID &id) const
		{
			return (_deletion_list.find(id) != _deletion_list.end());
		}

		void SQLiteDatabase::set_bundleid(Statement &st, const dtn::data::BundleID &id, size_t offset) const
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

		void SQLiteDatabase::get_bundleid(Statement &st, dtn::data::BundleID &id, size_t offset) const
		{
			id.source = dtn::data::EID((const char*)sqlite3_column_text(*st, offset + 0));
			id.timestamp = sqlite3_column_int64(*st, offset + 1);
			id.sequencenumber = sqlite3_column_int64(*st, offset + 2);

			id.fragment = (sqlite3_column_text(*st, offset + 3) != NULL);
			id.offset = sqlite3_column_int64(*st, offset + 3);
		}
	} /* namespace storage */
} /* namespace dtn */
