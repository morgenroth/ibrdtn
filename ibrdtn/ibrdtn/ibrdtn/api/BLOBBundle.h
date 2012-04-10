/*
 * BLOBBundle.h
 *
 *  Created on: 25.01.2010
 *      Author: morgenro
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
