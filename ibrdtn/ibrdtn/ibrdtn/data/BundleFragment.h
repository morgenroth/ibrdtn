/*
 * BundleFragment.h
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

#ifndef BUNDLEFRAGMENT_H_
#define BUNDLEFRAGMENT_H_

#include <ibrdtn/data/Number.h>
#include <stdlib.h>

namespace dtn
{
	namespace data
	{
		class Bundle;
		class BundleFragment
		{
			public:
				BundleFragment(const dtn::data::Bundle &bundle, const dtn::data::Length &payload_length);
				BundleFragment(const dtn::data::Bundle &bundle, const dtn::data::Length &offset, const dtn::data::Length &payload_length);
				virtual ~BundleFragment();

				const dtn::data::Bundle &_bundle;
				const dtn::data::Length _offset;
				const dtn::data::Length _length;
		};
	}
}

#endif /* BUNDLEFRAGMENT_H_ */
