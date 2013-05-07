/*
 * SQLiteBundleSet.h
 *
 *  Created on: 19.04.2013
 *      Author: goltzsch
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
			SQLiteBundleSet(dtn::data::BundleSet::Listener *listener, dtn::data::Size bf_size, dtn::storage::SQLiteDatabase& database);
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
			dtn::data::Size getLength() const throw();

			const ibrcommon::BloomFilter& getBloomFilter() const throw();

			std::set<dtn::data::MetaBundle> getNotIn(ibrcommon::BloomFilter &filter) const throw();

			virtual std::ostream &serialize(std::ostream &stream) const;
			virtual std::istream &deserialize(std::istream &stream);

		private:

			ibrcommon::BloomFilter _bf;

			dtn::data::BundleSet::Listener *_listener;

			bool _consistent;

			dtn::storage::SQLiteDatabase& _database;
		};
	} /* namespace data */
} /* namespace dtn */
#endif /* MEMORYBUNDLESET_H_ */
