/*
 * SchedulingBundleIndex.cpp
 *
 *  Created on: 21.01.2013
 *      Author: morgenro
 */

#include "routing/SchedulingBundleIndex.h"

namespace dtn
{
	namespace routing
	{
		SchedulingBundleIndex::SchedulingBundleIndex()
		{
		}

		SchedulingBundleIndex::~SchedulingBundleIndex()
		{
		}

		void SchedulingBundleIndex::add(const dtn::data::MetaBundle &b)
		{
			_priority_index.insert(b);
		}

		void SchedulingBundleIndex::remove(const dtn::data::BundleID &id)
		{
			for (priority_index::const_iterator iter = _priority_index.begin(); iter != _priority_index.end(); ++iter)
			{
				const dtn::data::MetaBundle &b = (*iter);
				if (id == (const dtn::data::BundleID&)b) {
					_priority_index.erase(iter);
					return;
				}
			}
		}

		void SchedulingBundleIndex::get(dtn::storage::BundleSelector &cb, dtn::storage::BundleResult &result) throw (dtn::storage::NoBundleFoundException, dtn::storage::BundleSelectorException)
		{
			bool unlimited = (cb.limit() <= 0);
			size_t added = 0;

			for (priority_index::const_iterator iter = _priority_index.begin(); iter != _priority_index.end(); ++iter)
			{
				const dtn::data::MetaBundle &b = (*iter);
				if (cb.shouldAdd(b)) {
					result.put(b);
					added++;
				}

				if (!unlimited && (added >= cb.limit())) break;
			}

			if (added == 0)
				throw dtn::storage::NoBundleFoundException();
		}

		const std::set<dtn::data::EID> SchedulingBundleIndex::getDistinctDestinations()
		{
			std::set<dtn::data::EID> ret;
			return ret;
		}
	} /* namespace routing */
} /* namespace dtn */
