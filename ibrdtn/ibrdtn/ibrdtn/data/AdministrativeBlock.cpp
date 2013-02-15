/*
 * AdministrativeBlock.cpp
 *
 *  Created on: 10.01.2013
 *      Author: morgenro
 */

#include "ibrdtn/data/AdministrativeBlock.h"

namespace dtn
{
	namespace data
	{
		AdministrativeBlock::AdministrativeBlock(char admfield) : _admfield(admfield) {
		}

		AdministrativeBlock::~AdministrativeBlock() {
		}

		bool AdministrativeBlock::refsFragment() const {
			return (_admfield & 0x01);
		}
	} /* namespace data */
} /* namespace dtn */
