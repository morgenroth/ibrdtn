/*
 * SummaryVector.h
 *
 *  Created on: 02.03.2010
 *      Author: morgenro
 */

#ifndef SUMMARYVECTOR_H_
#define SUMMARYVECTOR_H_

#include <ibrdtn/data/BundleID.h>
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/BundleString.h>
#include <ibrcommon/data/BloomFilter.h>
#include <iostream>
#include <set>

namespace dtn
{
	namespace routing
	{
		class SummaryVector
		{
		public:
			SummaryVector(const std::set<dtn::data::MetaBundle> &list);
			SummaryVector();
			virtual ~SummaryVector();

			virtual void commit();
			virtual bool contains(const dtn::data::BundleID &id) const;
			virtual void add(const dtn::data::BundleID &id);
			virtual void remove(const dtn::data::BundleID &id);

			virtual void clear();
			virtual void add(const std::set<dtn::data::MetaBundle> &list);

			size_t getLength() const;

			const ibrcommon::BloomFilter& getBloomFilter() const;

			std::set<dtn::data::BundleID> getNotIn(ibrcommon::BloomFilter &filter) const;

			friend std::ostream &operator<<(std::ostream &stream, const SummaryVector &obj);
			friend std::istream &operator>>(std::istream &stream, SummaryVector &obj);

		private:
			std::set<dtn::data::BundleID> _ids;
			ibrcommon::BloomFilter _bf;
		};
	}
}


#endif /* SUMMARYVECTOR_H_ */
