/*
 * BundleStorage.h
 *
 *  Created on: 24.03.2009
 *      Author: morgenro
 */

#ifndef BUNDLESTORAGE_H_
#define BUNDLESTORAGE_H_

#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/BundleID.h>
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/CustodySignalBlock.h>
#include <ibrcommon/data/BloomFilter.h>

#include <stdexcept>
#include <iterator>
#include <set>

namespace dtn
{
	namespace storage
	{
		class BundleStorage
		{
		public:
			class NoBundleFoundException : public dtn::MissingObjectException
			{
			public:
				NoBundleFoundException(string what = "No bundle match the specified criteria.") throw() : dtn::MissingObjectException(what)
				{
				};
			};

			class BundleLoadException : public NoBundleFoundException
			{
			public:
				BundleLoadException(string what = "Error while loading bundle data.") throw() : NoBundleFoundException(what)
				{
				};
			};

			class StorageSizeExeededException : public ibrcommon::Exception
			{
			public:
				StorageSizeExeededException(string what = "No space left in the storage.") throw() : ibrcommon::Exception(what)
				{
				};
			};

			class BundleFilterCallback
			{
			public:
				virtual ~BundleFilterCallback() {};

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
				virtual bool shouldAdd(const dtn::data::MetaBundle&) const { return false; };
			};

			/**
			 * destructor
			 */
			virtual ~BundleStorage() = 0;

			/**
			 * Stores a bundle in the storage.
			 * @param bundle The bundle to store.
			 */
			virtual void store(const dtn::data::Bundle &bundle) = 0;

			/**
			 * This method returns a specific bundle which is identified by its id.
			 * @param id The ID of the bundle to return.
			 * @return A bundle object.
			 */
			virtual dtn::data::Bundle get(const dtn::data::BundleID &id) = 0;

			/**
			 * Query the database for a number of bundles. The bundles are selected with the BundleFilterCallback
			 * class which is to implement by the user of this method.
			 * @param cb The instance of the callback filter class.
			 * @return A list of bundles.
			 */
			virtual const std::list<dtn::data::MetaBundle> get(BundleFilterCallback &cb) = 0;

			/**
			 * Return a set of distinct destinations for all bundles in the storage.
			 * @return
			 */
			virtual const std::set<dtn::data::EID> getDistinctDestinations() = 0;

			/**
			 * This method deletes a specific bundle in the storage.
			 * No reports will be generated here.
			 * @param id The ID of the bundle to remove.
			 */
			virtual void remove(const dtn::data::BundleID &id) = 0;

			/**
			 * This method deletes a specific bundle in the storage.
			 * No reports will be generated here.
			 * @param b The bundle to remove.
			 */
			void remove(const dtn::data::Bundle &b);

			/**
			 * Remove one bundles which match this filter
			 * @param filter
			 * @return The bundle meta data of the removed bundle.
			 */
			virtual dtn::data::MetaBundle remove(const ibrcommon::BloomFilter &filter);

			/**
			 * Clears all bundles and fragments in the storage.
			 */
			virtual void clear() {};

			/**
			 * @return True, if no bundles in the storage.
			 */
			virtual bool empty() { return true; };

			/**
			 * @return the count of bundles in the storage
			 */
			virtual unsigned int count() { return 0; };

			/**
			 * This method is called if another node accepts custody for a
			 * bundle of us.
			 * @param bundle
			 */
			virtual void releaseCustody(const dtn::data::EID &custodian, const dtn::data::BundleID &id) = 0;

			/**
			 * Accept custody for a given bundle. The previous custodian gets notified
			 * with a custody accept message.
			 * @param bundle
			 * @return The new custodian.
			 */
			const dtn::data::EID acceptCustody(const dtn::data::MetaBundle &meta);

			/**
			 * Reject custody for a given bundle. The custodian of this bundle gets notified
			 * with a custody reject message.
			 * @param bundle
			 */
			void rejectCustody(const dtn::data::MetaBundle &meta, dtn::data::CustodySignalBlock::REASON_CODE reason = dtn::data::CustodySignalBlock::NO_ADDITIONAL_INFORMATION);

		protected:
			/**
			 * constructor
			 */
			BundleStorage();
		};
	}
}

#endif /* BUNDLESTORAGE_H_ */
