/*
 * BundleSet.cpp
 *
 *  Created on: 18.12.2012
 *      Author: morgenro
 */

#include "ibrdtn/data/BundleSet.h"
#include <vector>

namespace dtn
{
	namespace data
	{
		BundleSet::Listener::~Listener()
		{ }

		BundleSet::BundleSet(BundleSet::Listener *listener, size_t bf_size)
		 : _bf(bf_size * 8), _listener(listener), _consistent(true)
		{
		}

		BundleSet::~BundleSet()
		{
		}

		void BundleSet::add(const dtn::data::MetaBundle &bundle) throw ()
		{
			// insert bundle id to the private list
			pair<std::set<dtn::data::MetaBundle>::iterator,bool> ret = _bundles.insert(bundle);

			ExpiringBundle exb(*ret.first);
			_expire.insert(exb);

			// add bundle to the bloomfilter
			_bf.insert(bundle.toString());
		}

		void BundleSet::clear() throw ()
		{
			_consistent = true;
			_bundles.clear();
			_expire.clear();
			_bf.clear();
		}

		bool BundleSet::has(const dtn::data::BundleID &bundle) const throw ()
		{
			// check bloom-filter first
			if (_bf.contains(bundle.toString())) {
				// Return true if the bloom-filter is not consistent with
				// the bundles set. This happen if the BundleSet gets deserialized.
				if (!_consistent) return true;

				std::set<dtn::data::MetaBundle>::iterator iter = _bundles.find(bundle);
				return (iter != _bundles.end());
			}

			return false;
		}

		size_t BundleSet::size() const throw ()
		{
			return _bundles.size();
		}

		void BundleSet::expire(const size_t timestamp) throw ()
		{
			bool commit = false;

			// we can not expire bundles if we have no idea of time
			if (timestamp == 0) return;

			std::set<ExpiringBundle>::iterator iter = _expire.begin();

			while (iter != _expire.end())
			{
				const ExpiringBundle &b = (*iter);

				if ( b.bundle.expiretime >= timestamp ) break;

				// raise expired event
				if (_listener != NULL)
					_listener->eventBundleExpired( b.bundle );

				// remove this item in public list
				_bundles.erase( b.bundle );

				// remove this item in private list
				_expire.erase( iter++ );

				// set commit to true (triggers bloom-filter rebuild)
				commit = true;
			}

			if (commit)
			{
				// rebuild the bloom-filter
				_bf.clear();
				for (std::set<dtn::data::MetaBundle>::const_iterator iter = _bundles.begin(); iter != _bundles.end(); iter++)
				{
					_bf.insert( (*iter).toString() );
				}
			}
		}

		const ibrcommon::BloomFilter& BundleSet::getBloomFilter() const throw ()
		{
			return _bf;
		}

		std::set<dtn::data::MetaBundle> BundleSet::getNotIn(ibrcommon::BloomFilter &filter) const throw ()
		{
			std::set<dtn::data::MetaBundle> ret;

//			// if the lists are equal return an empty list
//			if (filter == _bf) return ret;

			// iterate through all items to find the differences
			for (std::set<dtn::data::MetaBundle>::const_iterator iter = _bundles.begin(); iter != _bundles.end(); iter++)
			{
				if (!filter.contains( (*iter).toString() ) )
				{
					ret.insert( (*iter) );
				}
			}

			return ret;
		}

		BundleSet::ExpiringBundle::ExpiringBundle(const MetaBundle &b)
		 : bundle(b)
		{ }

		BundleSet::ExpiringBundle::~ExpiringBundle()
		{ }

		bool BundleSet::ExpiringBundle::operator!=(const ExpiringBundle& other) const
		{
			return !(other == *this);
		}

		bool BundleSet::ExpiringBundle::operator==(const ExpiringBundle& other) const
		{
			return (other.bundle == this->bundle);
		}

		bool BundleSet::ExpiringBundle::operator<(const ExpiringBundle& other) const
		{
			if (bundle.expiretime < other.bundle.expiretime) return true;
			if (bundle.expiretime != other.bundle.expiretime) return false;

			if (bundle < other.bundle) return true;

			return false;
		}

		bool BundleSet::ExpiringBundle::operator>(const ExpiringBundle& other) const
		{
			return !(((*this) < other) || ((*this) == other));
		}

		size_t BundleSet::getLength() const throw ()
		{
			return dtn::data::SDNV(_bf.size()).getLength() + _bf.size();
		}

		std::ostream &operator<<(std::ostream &stream, const BundleSet &obj)
		{
			dtn::data::SDNV size(obj._bf.size());
			stream << size;

			const char *data = reinterpret_cast<const char*>(obj._bf.table());
			stream.write(data, obj._bf.size());

			return stream;
		}

		std::istream &operator>>(std::istream &stream, BundleSet &obj)
		{
			dtn::data::SDNV count;
			stream >> count;

			std::vector<char> buffer(count.getValue());

			stream.read(&buffer[0], buffer.size());

			obj.clear();
			obj._bf.load((unsigned char*)&buffer[0], buffer.size());

			// set the set to in-consistent mode
			obj._consistent = false;

			return stream;
		}
	} /* namespace data */
} /* namespace dtn */
