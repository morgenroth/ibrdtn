/*
 * BLOBBundle.cpp
 *
 *  Created on: 25.01.2010
 *      Author: morgenro
 */

#include "ibrdtn/config.h"
#include "ibrdtn/api/BLOBBundle.h"

namespace dtn
{
	namespace api
	{
		BLOBBundle::BLOBBundle(dtn::data::EID destination, ibrcommon::BLOB::Reference &ref) : Bundle(destination)
		{
			_b.push_back(ref);
		}

		BLOBBundle::~BLOBBundle()
		{
		}
	}
}
