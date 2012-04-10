/*
 * FileBundle.cpp
 *
 *  Created on: 24.07.2009
 *      Author: morgenro
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
