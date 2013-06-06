/*
 * BundleSetImpl.h
 *
 *  Created on: 04.04.2013
 *      Author: goltzsch
 */

#ifndef BUNDLESETIMPL_H_
#define BUNDLESETIMPL_H_

#include "ibrdtn/data/MetaBundle.h"
#include <ibrcommon/data/BloomFilter.h>
#include <set>

namespace dtn
{
	namespace data
	{
		class BundleSetImpl
		{
		public:
			/**
			 * @param bf_size Initial size fo the bloom-filter.
			 */
			virtual ~BundleSetImpl() = 0;

			virtual void add(const dtn::data::MetaBundle &bundle) throw () = 0;
			virtual void clear() throw () = 0;
			virtual bool has(const dtn::data::BundleID &bundle) const throw () = 0;

			virtual void expire(const Timestamp timestamp) = 0;

			/**
			 * Returns the number of elements in this set
			 */
			virtual Size size() const throw() = 0;

			/**
			 * Returns the data length of the serialized BundleSet
			 */
			virtual Length getLength() const throw () = 0;

			virtual const ibrcommon::BloomFilter& getBloomFilter() const throw() = 0;

			virtual std::set<dtn::data::MetaBundle> getNotIn(ibrcommon::BloomFilter &filter) const throw () = 0;

			virtual std::ostream& serialize(std::ostream &stream) const = 0;
			virtual std::istream& deserialize(std::istream &stream) = 0;

			virtual std::string getType() = 0;
			virtual bool isPersistent() = 0;
			virtual std::string getName() = 0;
		public:
			class ExpiringBundle
			{
			public:
				ExpiringBundle(const MetaBundle &b);
				virtual ~ExpiringBundle();

				bool operator!=(const ExpiringBundle& other) const;
				bool operator==(const ExpiringBundle& other) const;
				bool operator<(const ExpiringBundle& other) const;
				bool operator>(const ExpiringBundle& other) const;

				const dtn::data::MetaBundle &bundle;
			};

		};
	} /* namespace data */
} /* namespace dtn */
#endif /* BUNDLESETIMPL_H_ */
