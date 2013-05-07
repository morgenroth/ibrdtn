/*
 * SQLiteBundleSetFactory.h
 * Created on: May 7, 2013
 * Author: goltzsch
 *
 */

#ifndef SQLITEBUNDLESETFACTORY_H_
#define SQLITEBUNDLESETFACTORY_H_

#include "ibrdtn/data/BundleSetFactory.h"
#include "storage/SQLiteDatabase.h"

namespace dtn
{
	namespace storage
	{

		class SQLiteBundleSetFactory : public dtn::data::BundleSetFactory{
			public:
				SQLiteBundleSetFactory(SQLiteDatabase& db);
				~SQLiteBundleSetFactory();

			protected:
				dtn::data::BundleSetImpl* createBundleSet(dtn::data::BundleSet::Listener* listener, dtn::data::Size bf_size);

			private:
				SQLiteDatabase& _database;
		};

	} /* namespace storage */
} /* namespace dtn */
#endif /* SQLITEBUNDLESETFACTORY_H_ */
