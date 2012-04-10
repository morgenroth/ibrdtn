/*
 * BundleList.cpp
 *
 *  Created on: 26.07.2010
 *      Author: morgenro
 */

#include "routing/BundleSummary.h"

namespace dtn
{
	namespace routing
	{
		BundleSummary::BundleSummary()
		{
		}

		BundleSummary::~BundleSummary()
		{
		}

		void BundleSummary::add(const dtn::data::MetaBundle bundle)
		{
			_vector.add(bundle);
			dtn::data::BundleList::add(bundle);
		}

		void BundleSummary::remove(const dtn::data::MetaBundle bundle)
		{
			_vector.remove(bundle);
			dtn::data::BundleList::remove(bundle);
		}

		void BundleSummary::clear()
		{
			_vector.clear();
			dtn::data::BundleList::clear();
		}

		void BundleSummary::eventBundleExpired(const ExpiringBundle &bundle)
		{
			_vector.remove(bundle.bundle);
		}

		void BundleSummary::eventCommitExpired()
		{
			_vector.commit();
		}

		bool BundleSummary::contains(const dtn::data::BundleID &bundle) const
		{
			// quick check
			if (_vector.contains(bundle))
			{
				// do a deeper inspection
				return dtn::data::BundleList::contains(bundle);
			}

			return false;
		}

		const SummaryVector& BundleSummary::getSummaryVector() const
		{
			return _vector;
		}
	}
}
