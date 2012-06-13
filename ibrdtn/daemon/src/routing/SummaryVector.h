/*
 * SummaryVector.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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
