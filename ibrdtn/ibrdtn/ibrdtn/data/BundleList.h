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
			BundleList();
			virtual ~BundleList();

			virtual void add(const dtn::data::MetaBundle &bundle);
			virtual void remove(const dtn::data::MetaBundle &bundle);
			virtual void clear();
			virtual bool contains(const dtn::data::BundleID &bundle) const;

			void expire(const size_t timestamp);

			bool operator==(const size_t version) const;
			size_t getVersion() const;

		protected:
			class ExpiringBundle
			{
			public:
				ExpiringBundle(const MetaBundle &b);
				virtual ~ExpiringBundle();

				bool operator!=(const ExpiringBundle& other) const;
				bool operator==(const ExpiringBundle& other) const;
				bool operator<(const ExpiringBundle& other) const;
				bool operator>(const ExpiringBundle& other) const;

				const MetaBundle bundle;
				const size_t expiretime;
			};

			virtual void eventBundleExpired(const ExpiringBundle&) {};
			virtual void eventCommitExpired() {};

			std::set<ExpiringBundle> _bundles;

		private:
			// the version value gets incremented on every change
			size_t _version;
		};

	}
}

#endif /* BUNDLELIST_H_ */
