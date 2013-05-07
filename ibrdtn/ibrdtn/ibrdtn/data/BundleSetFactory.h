/*
 * BundleSetFactory.h
 *
 *  Created on: May 7, 2013
 *      Author: goltzsch
 */

#ifndef BUNDLESETFACTORY_H_
#define BUNDLESETFACTORY_H_

#include "ibrdtn/data/BundleSet.h"
#include "ibrdtn/data/BundleSetImpl.h"
#include "ibrdtn/data/Number.h"

namespace dtn
{
	namespace data
	{
		class BundleSetFactory {
			public:
				virtual ~BundleSetFactory();

				/*
				 * create new BundleSet, based on inner BundleSetFactory
				 */
				static BundleSetImpl* create(BundleSet::Listener* listener, Size bf_size);

				/*
				 * the BundleSetFactory, which creates the bundles
				 */
				static dtn::data::BundleSetFactory* bundleSetFactory;

			protected:
				virtual BundleSetImpl* createBundleSet(BundleSet::Listener* listener, Size bf_size) = 0;
		};

	} /* namespace data */
} /* namespace dtn */

#endif /* BUNDLESETFACTORY_H_ */
