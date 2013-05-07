/*
 * BundleSet.cpp
 *
 *  Created on: 18.12.2012
 *      Author: morgenro
 *
 *  recreated on: 04.04.2013
 *  as interface
 *  	Author: goltzsch
 */

#include "ibrdtn/data/BundleSet.h"
#include "ibrdtn/data/BundleSetFactory.h"

namespace dtn
{
	namespace data
	{

		BundleSet::Listener::~Listener()
		{
		}

		BundleSet::BundleSet(BundleSet::Listener *listener, Length bf_size) : _set_impl(BundleSetFactory::create(listener,bf_size))
		{
		}
		BundleSet::BundleSet(BundleSetImpl* ptr) : _set_impl(ptr)
		{

		}

		BundleSet::~BundleSet()
		{
		}

		void BundleSet::add(const dtn::data::MetaBundle &bundle) throw ()
		{
			_set_impl->add(bundle);
		}

		void BundleSet::clear() throw ()
		{
			_set_impl->clear();
		}

		bool BundleSet::has(const dtn::data::BundleID &bundle) const throw ()
		{
			return _set_impl->has(bundle);
		}

		Size BundleSet::size() const throw ()
		{
			return _set_impl->size();
		}

		void BundleSet::expire(const Timestamp timestamp) throw ()
		{
			_set_impl->expire(timestamp);
		}

		const ibrcommon::BloomFilter& BundleSet::getBloomFilter() const throw ()
		{
			return _set_impl->getBloomFilter();
		}

		std::set<dtn::data::MetaBundle> BundleSet::getNotIn(ibrcommon::BloomFilter &filter) const throw ()
		{
			return _set_impl->getNotIn(filter);
		}

		Length BundleSet::getLength() const throw ()
		{
			return _set_impl->getLength();
		}

		std::ostream& BundleSet::serialize(std::ostream &stream) const
		{
			return _set_impl->serialize(stream);
		}

		std::istream& BundleSet::deserialize(std::istream &stream)
		{
			return _set_impl->deserialize(stream);
		}

		std::ostream &operator<<(std::ostream &stream, const BundleSet &obj)
		{
			return obj.serialize(stream);
		}

		std::istream &operator>>(std::istream &stream, BundleSet &obj)
		{
			return obj.deserialize(stream);
		}
	} /* namespace data */
} /* namespace dtn */
