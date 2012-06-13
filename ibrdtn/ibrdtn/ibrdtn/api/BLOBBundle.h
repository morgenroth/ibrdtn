/*
 * BLOBBundle.h
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

#ifndef BLOBBUNDLE_H_
#define BLOBBUNDLE_H_

#include "ibrdtn/api/Bundle.h"
#include "ibrdtn/data/PayloadBlock.h"
#include <ibrcommon/data/BLOB.h>
#include <iostream>

namespace dtn
{
	namespace api
	{
		/**
		 * This type of bundle contains a predefined BLOB object, which is part of the
		 * ibrcommon library. The data in the BLOB is transmitted as payload of the bundle.
		 */
		class BLOBBundle : public Bundle
		{
		public:
			/**
			 * Construct a new BLOBBundle with an destination and a reference to a BLOB object.
			 */
			BLOBBundle(dtn::data::EID destination, ibrcommon::BLOB::Reference &ref);

			/**
			 * Destructor of the BLOBBundle.
			 */
			virtual ~BLOBBundle();
		};
	}
}

#endif /* BLOBBUNDLE_H_ */
