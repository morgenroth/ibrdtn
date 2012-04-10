/*
 * BundleList.h
 *
 *  Created on: 26.07.2010
 *      Author: morgenro
 */

#ifndef BUNDLESUMMARY_H_
#define BUNDLESUMMARY_H_

#include "routing/SummaryVector.h"
#include <ibrdtn/data/BundleList.h>


namespace dtn
{
	namespace routing
	{
		class BundleSummary : public dtn::data::BundleList
		{
		public:
			BundleSummary();
			virtual ~BundleSummary();

			void add(const dtn::data::MetaBundle bundle);
			void remove(const dtn::data::MetaBundle bundle);
			void clear();

			bool contains(const dtn::data::BundleID &bundle) const;

			const SummaryVector& getSummaryVector() const;

		protected:
			void eventBundleExpired(const ExpiringBundle&);
			void eventCommitExpired();

		private:
			SummaryVector _vector;
		};
	}
}

#endif /* BUNDLESUMMARY_H_ */
