/*
 * MemoryBundleSet.h
 * renamed from BundleSet.h 04.04.2013
 * goltzsch
 *
 *  Created on: 18.12.2012
 *      Author: morgenro
 */

#ifndef MEMORYBUNDLESET_H_
#define MEMORYBUNDLESET_H_

#include "ibrdtn/data/BundleSetImpl.h"
#include "ibrdtn/data/BundleSet.h"
#include "ibrdtn/data/MetaBundle.h"
#include <ibrcommon/data/BloomFilter.h>
#include <set>

namespace dtn
{
	namespace data
	{
		class MemoryBundleSet : public BundleSetImpl
		{
		public:
			/**
			 * @param bf_size Initial size fo the bloom-filter.
			 */
			MemoryBundleSet(BundleSet::Listener *listener = NULL, Length bf_size = 1024);
			virtual ~MemoryBundleSet();

			virtual void add(const dtn::data::MetaBundle &bundle) throw ();
			virtual void clear() throw ();
			virtual bool has(const dtn::data::BundleID &bundle) const throw ();

			virtual void expire(const Timestamp timestamp) throw ();

			/**
			 * Returns the number of elements in this set
			 */
			virtual Size size() const throw ();

			/**
			 * Returns the data length of the serialized BundleSet
			 */
			Length getLength() const throw ();

			const ibrcommon::BloomFilter& getBloomFilter() const throw ();

			std::set<dtn::data::MetaBundle> getNotIn(ibrcommon::BloomFilter &filter) const throw ();

			virtual std::ostream &serialize(std::ostream &stream) const;
            virtual std::istream &deserialize(std::istream &stream);

			virtual std::string getType();
			virtual bool isPersistent();
			virtual std::string getName();


		private:

			std::set<dtn::data::MetaBundle> _bundles;
			std::set<BundleSetImpl::ExpiringBundle> _expire;

			ibrcommon::BloomFilter _bf;

			BundleSet::Listener *_listener;

			bool _consistent;
		};
	} /* namespace data */
} /* namespace dtn */
#endif /* MEMORYBUNDLESET_H_ */
