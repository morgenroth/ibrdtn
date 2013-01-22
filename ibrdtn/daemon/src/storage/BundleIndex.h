/*
 * BundleIndex.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#ifndef BUNDLEINDEX_H_
#define BUNDLEINDEX_H_

#include "storage/BundleSeeker.h"
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/BundleID.h>

namespace dtn
{
	namespace storage
	{
		class BundleIndex : public dtn::storage::BundleSeeker {
		public:
			BundleIndex();
			virtual ~BundleIndex() = 0;

			virtual void add(const dtn::data::MetaBundle &b) = 0;
			virtual void remove(const dtn::data::BundleID &id) = 0;
		};
	} /* namespace storage */
} /* namespace dtn */
#endif /* BUNDLEINDEX_H_ */
