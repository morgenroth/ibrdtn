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

#ifndef BUNDLELIST_H_
#define BUNDLELIST_H_

#include <ibrdtn/data/MetaBundle.h>
#include <set>

namespace dtn
{
	namespace data
	{
		class BundleList : public std::set<dtn::data::MetaBundle>
		{
		public:
			class Listener {
			public:
				virtual ~Listener() = 0;
				virtual void eventBundleExpired(const dtn::data::MetaBundle&) = 0;
			};

			BundleList(BundleList::Listener *listener = NULL);
			virtual ~BundleList();

			virtual void add(const dtn::data::MetaBundle &bundle);
			virtual void remove(const dtn::data::MetaBundle &bundle);
			virtual void clear();

			virtual void expire(const size_t timestamp);

		private:
			class ExpiringBundle
			{
			public:
				ExpiringBundle(const MetaBundle &b);
				virtual ~ExpiringBundle();

				bool operator!=(const ExpiringBundle& other) const;
				bool operator==(const ExpiringBundle& other) const;
				bool operator<(const ExpiringBundle& other) const;
				bool operator>(const ExpiringBundle& other) const;

				const MetaBundle &bundle;
			};

			std::set<ExpiringBundle> _bundles;

			BundleList::Listener *_listener;
		};

	}
}

#endif /* BUNDLELIST_H_ */
