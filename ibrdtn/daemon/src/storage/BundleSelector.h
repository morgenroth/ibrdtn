/*
 * BundleSelector.h
 *
 *  Created on: 22.01.2013
 *      Author: morgenro
 */

#ifndef BUNDLESELECTOR_H_
#define BUNDLESELECTOR_H_

#include "storage/BundleResult.h"
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/MetaBundle.h>
#include <ibrcommon/Exceptions.h>

namespace dtn
{
	namespace storage
	{
		class BundleSelectorException : public ibrcommon::Exception
		{
		public:
			BundleSelectorException(std::string what = "Bundle query aborted.") throw() : ibrcommon::Exception(what)
			{
			}
		};

		class NoBundleFoundException : public dtn::MissingObjectException
		{
		public:
			NoBundleFoundException(std::string what = "No bundle match the specified criteria.") throw() : dtn::MissingObjectException(what)
			{
			};
		};

		class BundleSelector
		{
		public:
			virtual ~BundleSelector() {};

			/**
			 * Limit the number of selected items.
			 * @return The limit as number of items.
			 */
			virtual size_t limit() const { return 1; };

			/**
			 * This method is called by the storage to determine if one bundle
			 * should added to a set or not.
			 * @param id The bundle id of the bundle to select.
			 * @return True, if the bundle should be added to the set.
			 */
			virtual bool shouldAdd(const dtn::data::MetaBundle&) const throw (BundleSelectorException) { return false; };
		};
	}
}

#endif /* BUNDLESELECTOR_H_ */
