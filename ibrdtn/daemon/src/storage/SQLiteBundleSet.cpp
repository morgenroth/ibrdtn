/*
 * SQLiteBundleSet.cpp
 *
 *  Created on: 19.04.2013
 *      Author: goltzsch
 */

#include "storage/SQLiteBundleSet.h"

namespace dtn
{
	namespace storage
	{
		//TODO nummer für verschiedene BundleSets
		// spezielle nummern, und generiert
		// wenn neu generiert -> alte eintr. mit neuer nummer löschen

		SQLiteBundleSet::SQLiteBundleSet(dtn::data::BundleSet::Listener *listener, dtn::data::Size bf_size, dtn::storage::SQLiteDatabase* database)
		 : _bf(bf_size * 8), _listener(listener), _consistent(true),_database(database)
		{
		}

		SQLiteBundleSet::SQLiteBundleSet(dtn::storage::SQLiteDatabase* database)
		 : _bf(1024 * 8), _listener(NULL), _consistent(true), _database(database)
		{
		}
		SQLiteBundleSet::~SQLiteBundleSet()
		{
		}

		void SQLiteBundleSet::add(const dtn::data::MetaBundle &bundle) throw()
		{
			// insert bundle id into database
			_database->add_seen_bundle(bundle);

			// add bundle to the bloomfilter
			_bf.insert(bundle.toString());
		}

		void SQLiteBundleSet::clear() throw()
		{
			_consistent = true;
			_database->clear_seen_bundles();
			_bf.clear();
		}

		bool SQLiteBundleSet::has(const dtn::data::BundleID &bundle) const throw()
		{
			// check bloom-filter first
			if (_bf.contains(bundle.toString())) {
				// Return true if the bloom-filter is not consistent with
				// the bundles set. This happen if the MemoryBundleSet gets deserialized.
				if (!_consistent) return true;

				return _database->contains_seen_bundle(bundle);
			}

			return false;
		}

		void SQLiteBundleSet::expire(const dtn::data::Timestamp timestamp) throw ()
		{
			bool commit = false;

			// we can not expire bundles if we have no idea of time
			if (timestamp == 0) return;

			// expire in database
			_database->expire(timestamp);

			// raise expired event
			if (_listener != NULL)
				_listener->eventBundleExpired(dtn::data::MetaBundle()); //TODO was sinnvolles übergeben

			//wenn kl. expiretime überschritten > commit
			if (commit)
			{
				//TODO Idee: strings mit SQL aufbauen -> performanter

				//kleinste expiretime merken, aufpassen bei add!
				// rebuild the bloom-filter
				_bf.clear();
				std::set<dtn::data::MetaBundle> bundles = _database->get_all_seen_bundles();
				for (std::set<dtn::data::MetaBundle>::const_iterator iter = bundles.begin(); iter != bundles.end(); iter++)
				{
					_bf.insert( (*iter).toString() );
				}
			}
		}

		dtn::data::Size SQLiteBundleSet::size() const throw()
		{
			return _database->count_seen_bundles();
		}

		dtn::data::Size SQLiteBundleSet::getLength() const throw()
		{
			return dtn::data::Number(_bf.size()).getLength() + _bf.size();
		}


		const ibrcommon::BloomFilter& SQLiteBundleSet::getBloomFilter() const throw()
		{
			return _bf;
		}

		std::set<dtn::data::MetaBundle> SQLiteBundleSet::getNotIn(ibrcommon::BloomFilter &filter) const throw()
		{
			std::set<dtn::data::MetaBundle> ret;

			//get bundles from db
			std::set<dtn::data::MetaBundle> bundles = _database->get_all_seen_bundles();

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
	} /* namespace data */
} /* namespace dtn */
