/*
 * BundleFragment.cpp
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

#include "ibrdtn/data/BundleFragment.h"
#include "ibrdtn/data/Bundle.h"

namespace dtn
{
	namespace data
	{
		BundleFragment::BundleFragment(const dtn::data::Bundle &bundle, const dtn::data::Length &payload_length)
		 : _bundle(bundle), _offset(0), _length(payload_length)
		{
		}

		BundleFragment::BundleFragment(const dtn::data::Bundle &bundle, const dtn::data::Length &offset, const dtn::data::Length &payload_length)
		 : _bundle(bundle), _offset(offset), _length(payload_length)
		{
		}

		BundleFragment::~BundleFragment()
		{
		}
	}
}
