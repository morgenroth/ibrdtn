/*
 * BundleFragment.cpp
 *
 *  Created on: 14.04.2011
 *      Author: morgenro
 */

#include "ibrdtn/data/BundleFragment.h"
#include "ibrdtn/data/Bundle.h"

namespace dtn
{
	namespace data
	{
		BundleFragment::BundleFragment(const dtn::data::Bundle &bundle, size_t payload_length)
		 : _bundle(bundle), _offset(0), _length(payload_length)
		{
		}

		BundleFragment::BundleFragment(const dtn::data::Bundle &bundle, size_t offset, size_t payload_length)
		 : _bundle(bundle), _offset(offset), _length(payload_length)
		{
		}

		BundleFragment::~BundleFragment()
		{
		}
	}
}
