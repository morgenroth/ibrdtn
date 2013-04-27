/*
 * BundleMerger.h
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

#ifndef BUNDLEMERGER_H_
#define BUNDLEMERGER_H_

#include "ibrcommon/data/BLOB.h"
#include "ibrdtn/data/Bundle.h"
#include <set>

namespace dtn
{
	namespace data
	{
		class BundleMerger
		{
		public:
			class Chunk
			{
			public:
				Chunk(uint64_t, uint64_t);
				virtual ~Chunk();

				bool operator<(const Chunk &other) const;

				uint64_t offset;
				uint64_t length;

				static bool isComplete(uint64_t length, const std::set<Chunk> &chunks);
			};

			class Container
			{
			public:
				Container(ibrcommon::BLOB::Reference &ref);
				virtual ~Container();
				bool isComplete();

				Bundle& getBundle();

				friend Container &operator<<(Container &c, dtn::data::Bundle &obj);

			private:
				bool contains(uint64_t offset, uint64_t length) const;
				void add(uint64_t offset, uint64_t length);

				dtn::data::Bundle _bundle;
				ibrcommon::BLOB::Reference _blob;
				bool _initialized;
				bool _hasFirstFragBlocksAdded;
				bool _hasLastFragBlocksAdded;

				std::set<Chunk> _chunks;
				uint64_t _appdatalength;
			};

			static Container getContainer();
			static Container getContainer(ibrcommon::BLOB::Reference &ref);
		};
	}
}


#endif /* BUNDLEMERGER_H_ */
