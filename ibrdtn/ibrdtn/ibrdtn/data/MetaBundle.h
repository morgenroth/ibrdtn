/*
 * MetaBundle.h
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

#ifndef METABUNDLE_H_
#define METABUNDLE_H_

#include "ibrdtn/data/BundleID.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/DTNTime.h"
#include "ibrdtn/data/EID.h"

namespace dtn
{
	namespace data
	{
		class MetaBundle : public dtn::data::BundleID
		{
		public:
			MetaBundle();
			MetaBundle(const dtn::data::BundleID &id);
			MetaBundle(const dtn::data::Bundle &b);
			virtual ~MetaBundle();

			int getPriority() const;
			bool get(dtn::data::PrimaryBlock::FLAGS flag) const;

			dtn::data::DTNTime received;
			size_t lifetime;
			dtn::data::EID destination;
			dtn::data::EID reportto;
			dtn::data::EID custodian;
			size_t appdatalength;
			size_t procflags;
			size_t expiretime;
			size_t hopcount;
			size_t payloadlength;
			int8_t net_priority;
		};
	}
}

#endif /* METABUNDLE_H_ */
