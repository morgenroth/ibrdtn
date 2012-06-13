/*
 * MemoryBundleStorage.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef MEMORYBUNDLESTORAGE_H_
#define MEMORYBUNDLESTORAGE_H_

#include "Component.h"
#include "core/BundleCore.h"
#include "storage/BundleStorage.h"
#include "core/Node.h"
#include "core/EventReceiver.h"

#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/BundleList.h>

namespace dtn
{
	namespace storage
	{
		class MemoryBundleStorage : public BundleStorage, public dtn::core::EventReceiver, public dtn::daemon::IntegratedComponent, private dtn::data::BundleList
		{
		public:
			MemoryBundleStorage(size_t maxsize = 0);
			virtual ~MemoryBundleStorage();

			/**
			 * Stores a bundle in the storage.
			 * @param bundle The bundle to store.
			 */
			virtual void store(const dtn::data::Bundle &bundle);

			/**
			 * This method returns a specific bundle which is identified by
			 * its id.
			 * @param id The ID of the bundle to return.
			 * @return A bundle object of the
			 */
			virtual dtn::data::Bundle get(const dtn::data::BundleID &id);

			/**
			 * @see BundleStorage::get(BundleFilterCallback &cb)
			 */
			virtual const std::list<dtn::data::MetaBundle> get(BundleFilterCallback &cb);

			/**
			 * @see BundleStorage::getDistinctDestinations()
			 */
			virtual const std::set<dtn::data::EID> getDistinctDestinations();

			/**
			 * This method deletes a specific bundle in the storage.
			 * No reports will be generated here.
			 * @param id The ID of the bundle to remove.
			 */
			void remove(const dtn::data::BundleID &id);

			/**
			 * Remove one bundles which match this filter
			 * @param filter
			 * @return The bundle meta data of the removed bundle.
			 */
			dtn::data::MetaBundle remove(const ibrcommon::BloomFilter &filter);

			/**
			 * @sa BundleStorage::clear()
			 */
			void clear();

			/**
			 * @sa BundleStorage::empty()
			 */
			bool empty();

			/**
			 * @sa BundleStorage::count()
			 */
			unsigned int count();

			/**
			 * Get the current size
			 */
			size_t size() const;

			/**
			 * @sa BundleStorage::releaseCustody();
			 */
			void releaseCustody(const dtn::data::EID &custodian, const dtn::data::BundleID &id);

			/**
			 * This method is used to receive events.
			 * @param evt
			 */
			void raiseEvent(const dtn::core::Event *evt);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

		protected:
			virtual void componentUp();
			virtual void componentDown();

			virtual void eventBundleExpired(const ExpiringBundle &b);

		private:
			ibrcommon::Mutex _bundleslock;
			std::set<dtn::data::Bundle> _bundles;

			struct CMP_BUNDLE_PRIORITY
			{
				bool operator() (const dtn::data::MetaBundle& lhs, const dtn::data::MetaBundle& rhs) const
				{
					if (lhs.getPriority() == rhs.getPriority())
						return rhs > lhs;

					return (lhs.getPriority() > rhs.getPriority());
				}
			};

			std::set<dtn::data::MetaBundle, CMP_BUNDLE_PRIORITY> _priority_index;
			std::map<dtn::data::BundleID, ssize_t> _bundle_lengths;

			size_t _maxsize;
			size_t _currentsize;
		};
	}
}

#endif /* MEMORYBUNDLESTORAGE_H_ */
