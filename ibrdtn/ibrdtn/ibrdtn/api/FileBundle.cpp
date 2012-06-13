/*
 * FileBundle.cpp
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

#include "ibrdtn/config.h"
#include "ibrdtn/api/FileBundle.h"
#include "ibrdtn/data/PayloadBlock.h"
#include <ibrcommon/data/BLOB.h>

namespace dtn
{
	namespace api
	{
		FileBundle::FileBundle(const dtn::data::EID &destination, const ibrcommon::File &file)
		 : Bundle(destination)
		{
			// create a reference out of the given file
			ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::open(file);

			// create a new payload block with this reference.
			_b.push_back(ref);
		}

		FileBundle::~FileBundle()
		{
		}
	}
}
