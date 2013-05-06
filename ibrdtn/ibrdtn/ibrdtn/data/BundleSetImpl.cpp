/*
 * BundleSetImpl.cpp
 *
 *  Created on: Apr 5, 2013
 *      Author: goltzsch
 *
 *  only implements destructor
 */

#include "ibrdtn/data/BundleSetImpl.h"

namespace dtn
{
	namespace data
	{
		BundleSetImpl::~BundleSetImpl()
		{

		}
		BundleSetImpl::ExpiringBundle::ExpiringBundle(const MetaBundle &b) : bundle(b)
		{ }

		BundleSetImpl::ExpiringBundle::~ExpiringBundle()
		{ }

		bool BundleSetImpl::ExpiringBundle::operator!=(const ExpiringBundle& other) const
		{
			return !(other == *this);
		}

		bool BundleSetImpl::ExpiringBundle::operator==(const ExpiringBundle& other) const
		{
			return (other.bundle == this->bundle);
		}

		bool BundleSetImpl::ExpiringBundle::operator<(const ExpiringBundle& other) const
		{
			if (bundle.expiretime < other.bundle.expiretime) return true;
			if (bundle.expiretime != other.bundle.expiretime) return false;

			if (bundle < other.bundle) return true;

			return false;
		}

		bool BundleSetImpl::ExpiringBundle::operator>(const ExpiringBundle& other) const
		{
			return !(((*this) < other) || ((*this) == other));
		}
}
}



