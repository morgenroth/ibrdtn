/*
 * SQLiteDatabase.h
 *
 *  Created on: 26.03.2012
 *      Author: morgenro
 */

#ifndef SQLITEDATABASE_H_
#define SQLITEDATABASE_H_

#include "core/BundleExpiredEvent.h"
#include "storage/BundleStorage.h"
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/MetaBundle.h>
#include <ibrcommon/data/File.h>
#include <map>
#include <set>
#include <list>
#include <sqlite3.h>

namespace dtn
{
	namespace storage
	{
		class SQLiteDatabase
		{
			enum SQL_TABLES
			{
				SQL_TABLE_BUNDLE = 0,
				SQL_TABLE_BLOCK = 1,
				SQL_TABLE_ROUTING = 2,
				SQL_TABLE_BUNDLE_ROUTING_INFO = 3,
				SQL_TABLE_NODE_ROUTING_INFO = 4,
				SQL_TABLE_PROPERTIES = 5,
				SQL_TABLE_END = 6
			};

			// enum of all possible statements
			enum STORAGE_STMT
			{
				BUNDLE_GET_FILTER,
				BUNDLE_GET_ID,
				FRAGMENT_GET_ID,
				BUNDLE_GET_FRAGMENT,

				EXPIRE_BUNDLES,
				EXPIRE_BUNDLE_FILENAMES,
				EXPIRE_BUNDLE_DELETE,
				EXPIRE_NEXT_TIMESTAMP,

				EMPTY_CHECK,
				COUNT_ENTRIES,

				BUNDLE_DELETE,
				FRAGMENT_DELETE,
				BUNDLE_CLEAR,
				BUNDLE_STORE,
				BUNDLE_UPDATE_CUSTODIAN,
				FRAGMENT_UPDATE_CUSTODIAN,

				PROCFLAGS_SET,

				BLOCK_GET_ID,
				BLOCK_GET_ID_FRAGMENT,
				BLOCK_GET,
				BLOCK_GET_FRAGMENT,
				BLOCK_CLEAR,
				BLOCK_STORE,

				VACUUM,
				SQL_QUERIES_END
			};

			static const int DBSCHEMA_VERSION;
			static const std::string QUERY_SCHEMAVERSION;
			static const std::string SET_SCHEMAVERSION;

			static const std::string _tables[SQL_TABLE_END];

			// array of sql queries
			static const std::string _sql_queries[SQL_QUERIES_END];

			// array of the db structure as sql
			static const std::string _db_structure[11];

		public:
			enum UPDATE_VALUES
			{
				UPDATE_CUSTODIAN
			};

			class SQLBundleQuery
			{
			public:
				SQLBundleQuery();
				virtual ~SQLBundleQuery() = 0;

				/**
				 * returns the user-defined sql query
				 * @return
				 */
				virtual const std::string getWhere() const = 0;

				/**
				 * bind all custom values to the statement
				 * @param st
				 * @param offset
				 * @return
				 */
				virtual size_t bind(sqlite3_stmt*, size_t offset) const
				{
					return offset;
				}
			};

			class SQLiteQueryException : public ibrcommon::Exception
			{
			public:
				SQLiteQueryException(string what = "Unable to execute Querry.") throw(): Exception(what)
				{
				}
			};

			class Statement
			{
			public:
				Statement(sqlite3 *database, const std::string&);
				~Statement();

				sqlite3_stmt* operator*();
				void prepare();
				void reset();
				int step();

			private:
				sqlite3 *_database;
				sqlite3_stmt *_st;
				const std::string _query;
			};

			typedef std::list<std::pair<int, const ibrcommon::File> > blocklist;
			typedef std::pair<int, const ibrcommon::File> blocklist_entry;

			SQLiteDatabase(const ibrcommon::File &file, const size_t &size);
			virtual ~SQLiteDatabase();

			/**
			 * open the database
			 */
			void open();

			/**
			 * close the database
			 * @return
			 */
			void close();

			/**
			 * Expire all bundles with a lifetime lower than the given timestamp.
			 * @param timestamp
			 */
			void expire(size_t timestamp);

			/**
			 * Shrink down the database.
			 */
			void vacuum();

			/**
			 * Update
			 * @param id
			 * @param
			 */
			void update(UPDATE_VALUES, const dtn::data::BundleID &id, const dtn::data::EID&);

			/**
			 * Delete an entry in the database.
			 * @param id
			 */
			void remove(const dtn::data::BundleID &id);

			/**
			 * Get meta data of several bundles using a callback filter.
			 * @param cb
			 * @return
			 */
			const std::list<dtn::data::MetaBundle> get(dtn::storage::BundleStorage::BundleFilterCallback &cb) const;

			/**
			 * Retrieve the meta data of a given bundle
			 * @param id
			 * @return
			 */
			void get(const dtn::data::BundleID &id, dtn::data::MetaBundle &meta) const;

			/**
			 * Retrieve the data of a given bundle
			 * @param id
			 * @return
			 */
			void get(const dtn::data::BundleID &id, dtn::data::Bundle &bundle, blocklist &blocks) const;

			/**
			 *
			 * @param bundle
			 */
			void store(const dtn::data::Bundle &bundle);
			void store(const dtn::data::BundleID &id, int index, const dtn::data::Block &block, const ibrcommon::File &file);
			void transaction();
			void rollback();
			void commit();

			bool empty() const;

			unsigned int count() const;

			void clear();

			/**
			 * @see BundleStorage::getDistinctDestinations()
			 */
			virtual const std::set<dtn::data::EID> getDistinctDestinations();

		private:
			/**
			 * Retrieve meta data from the database and put them into a meta bundle structure.
			 * @param st
			 * @param bundle
			 * @param offset
			 */
			void get(Statement &st, dtn::data::MetaBundle &bundle, size_t offset = 0) const;

			/**
			 * Retrieve meta data from the database and put them into a bundle structure.
			 * @param st
			 * @param bundle
			 * @param offset
			 */
			void get(Statement &st, dtn::data::Bundle &bundle, size_t offset = 0) const;

			/**
			 *
			 * @param st
			 * @param ret
			 * @param bind_offset
			 * @param limit
			 */
			void __get(const dtn::storage::BundleStorage::BundleFilterCallback &cb, Statement &st, std::list<dtn::data::MetaBundle> &ret, size_t bind_offset, size_t offset) const;

//			/**
//			 * Checks the files on the filesystem against the filenames in the database
//			 */
//			void check_consistency();
//
//			void check_fragments(set<string> &payloadFiles);
//
//			void check_bundles(set<string> &blockFiles);

			/**
			 * updates the nextExpiredTime. The calling function has to have the databaselock.
			 */
			void update_expire_time();

			/**
			 * lower the next expire time if the ttl is lower than the current expire time
			 * @param ttl
			 */
			void new_expire_time(size_t ttl);
			void reset_expire_time();
			size_t get_expire_time();

			void set_bundleid(Statement &st, const dtn::data::BundleID &id, size_t offset = 0) const;
			void get_bundleid(Statement &st, dtn::data::BundleID &id, size_t offset = 0) const;

			/**
			 * get database version
			 */
			int getVersion();

			/**
			 * set database version
			 * @param version
			 */
			void setVersion(int version);

			/**
			 * upgrade the database to new version
			 * @param oldVersion Current version of the database.
			 * @param newVersion Required version.
			 */
			void doUpgrade(int oldVersion, int newVersion);

			ibrcommon::File _file;
			int _size;

			// holds the database handle
			sqlite3 *_database;

			// next expiration
			size_t _next_expiration;

			void add_deletion(const dtn::data::BundleID &id);
			void remove_deletion(const dtn::data::BundleID &id);
			bool contains_deletion(const dtn::data::BundleID &id) const;

			// set of bundles to delete
			std::set<dtn::data::BundleID> _deletion_list;
		};
	} /* namespace storage */
} /* namespace dtn */
#endif /* SQLITEDATABASE_H_ */
