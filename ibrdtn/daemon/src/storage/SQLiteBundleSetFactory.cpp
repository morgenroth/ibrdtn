/*
 * SQLiteBundleSetFactory.cpp
 *
 *  Created on: May 7, 2013
 *      Author: goltzsch
 */

#include "storage/SQLiteBundleSetFactory.h"
#include "storage/SQLiteBundleSet.h"

namespace dtn {
	namespace storage {
		SQLiteBundleSetFactory::SQLiteBundleSetFactory(SQLiteDatabase& db) : _database(db)
		{
		}
		SQLiteBundleSetFactory::~SQLiteBundleSetFactory()
		{
		}

		dtn::data::BundleSetImpl* SQLiteBundleSetFactory::createBundleSet(dtn::data::BundleSet::Listener* listener, dtn::data::Size bf_size)
		{
			return new SQLiteBundleSet(listener,bf_size,_database);
		}

	} /* namespace storage */
} /* namespace dtn */
