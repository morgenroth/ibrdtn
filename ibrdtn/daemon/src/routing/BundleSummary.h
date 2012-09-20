/*
 * BundleList.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef BUNDLESUMMARY_H_
#define BUNDLESUMMARY_H_

#include "routing/SummaryVector.h"
#include <ibrdtn/data/BundleList.h>


namespace dtn
{
	namespace routing
	{
		class BundleSummary : public dtn::data::BundleList::Listener
		{
		public:
			BundleSummary();
			virtual ~BundleSummary();

			void add(const dtn::data::MetaBundle bundle);
			void remove(const dtn::data::MetaBundle bundle);
			void clear();

			bool contains(const dtn::data::BundleID &bundle) const;
			void expire(const size_t timestamp);
			size_t count() const;

			const SummaryVector& getSummaryVector() const;

		protected:
			void eventBundleExpired(const dtn::data::BundleList::ExpiringBundle&);
			void eventCommitExpired();

		private:
			SummaryVector _vector;
			dtn::data::BundleList _list;
		};
	}
}

#endif /* BUNDLESUMMARY_H_ */
