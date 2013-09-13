/*
 * BundleID.h
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

#ifndef BUNDLEID_H_
#define BUNDLEID_H_

#include <ibrdtn/data/Number.h>
#include "ibrdtn/data/EID.h"
#include "ibrdtn/data/Bundle.h"

namespace dtn
{
	namespace data
	{
		class BundleID
		{
		public:
			BundleID(const dtn::data::EID source = dtn::data::EID(), const dtn::data::Timestamp &timestamp = 0, const dtn::data::Number &sequencenumber = 0, const bool fragment = false, const dtn::data::Number &offset = 0);
			BundleID(const dtn::data::PrimaryBlock &b);
			virtual ~BundleID();

			bool operator!=(const BundleID& other) const;
			bool operator==(const BundleID& other) const;
			bool operator<(const BundleID& other) const;
			bool operator>(const BundleID& other) const;

			bool operator<(const PrimaryBlock& other) const;
			bool operator>(const PrimaryBlock& other) const;
			bool operator!=(const PrimaryBlock& other) const;
			bool operator==(const PrimaryBlock& other) const;

			std::string toString() const;

			friend std::ostream &operator<<(std::ostream &stream, const BundleID &obj);
			friend std::istream &operator>>(std::istream &stream, BundleID &obj);

			dtn::data::EID source;
			dtn::data::Timestamp timestamp;
			dtn::data::Number sequencenumber;

			bool fragment;
			dtn::data::Number offset;
		};
	}
}

#endif /* BUNDLEID_H_ */
