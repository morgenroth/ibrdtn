/*
 * SimpleBundleStorage.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SIMPLEBUNDLESTORAGE_H_
#define SIMPLEBUNDLESTORAGE_H_

#include "Component.h"
#include "core/BundleCore.h"
#include "storage/BundleStorage.h"
#include "core/Node.h"
#include "core/EventReceiver.h"

#include "storage/DataStorage.h"

#include <ibrcommon/thread/Conditional.h>
#include <ibrcommon/thread/AtomicCounter.h>
#include <ibrcommon/thread/RWMutex.h>

#include <ibrcommon/data/File.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/BundleList.h>
#include <ibrcommon/thread/Queue.h>

#include <set>
#include <map>

using namespace dtn::data;

namespace dtn
{
	namespace storage
	{
		/**
		 * This storage holds all bundles and fragments in the system memory.
		 */
		class SimpleBundleStorage : public DataStorage::Callback, public BundleStorage, public dtn::core::EventReceiver, public dtn::daemon::IntegratedComponent, public BundleList::Listener
		{
		public:
			/**
			 * Constructor
			 */
			SimpleBundleStorage(const ibrcommon::File &workdir, size_t maxsize = 0, size_t buffer_limit = 0);

			/**
			 * Destructor
			 */
			virtual ~SimpleBundleStorage();

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
			 * @see BundleSeeker::get(BundleSelector &cb, BundleResult &result)
			 */
			virtual void get(BundleSelector &cb, BundleResult &result) throw (NoBundleFoundException, BundleSelectorException);

			/**
			 * @see BundleSeeker::getDistinctDestinations()
			 */
			virtual const eid_set getDistinctDestinations();

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
			 * @sa BundleStorage::releaseCustody();
			 */
			void releaseCustody(const dtn::data::EID &custodian, const dtn::data::BundleID &id);

			/**
			 * This method is used to receive events.
			 * @param evt
			 */
			void raiseEvent(const dtn::core::Event *evt) throw ();

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			virtual void eventDataStorageStored(const dtn::storage::DataStorage::Hash &hash);
			virtual void eventDataStorageStoreFailed(const dtn::storage::DataStorage::Hash &hash, const ibrcommon::Exception&);
			virtual void eventDataStorageRemoved(const dtn::storage::DataStorage::Hash &hash);
			virtual void eventDataStorageRemoveFailed(const dtn::storage::DataStorage::Hash &hash, const ibrcommon::Exception&);
			virtual void iterateDataStorage(const dtn::storage::DataStorage::Hash &hash, dtn::storage::DataStorage::istream &stream);

		protected:
			virtual void componentUp() throw ();
			virtual void componentDown() throw ();
			virtual void eventBundleExpired(const dtn::data::MetaBundle &b);

		private:
			class BundleContainer : public DataStorage::Container
			{
			public:
				BundleContainer(const dtn::data::Bundle &b);
				virtual ~BundleContainer();

				std::string getKey() const;
				std::ostream& serialize(std::ostream &stream);

			private:
				const dtn::data::Bundle _bundle;
			};

			dtn::data::Bundle __get(const dtn::data::MetaBundle&);

			// bundle list
			dtn::data::BundleList _list;

			// This object manage data stored on disk
			DataStorage _datastore;

			ibrcommon::RWMutex _bundleslock;
			std::map<DataStorage::Hash, dtn::data::Bundle> _pending_bundles;
			std::map<dtn::data::MetaBundle, DataStorage::Hash> _stored_bundles;

			typedef std::map<dtn::data::BundleID, size_t> size_map;
			size_map _bundle_lengths;

			struct CMP_BUNDLE_PRIORITY
			{
				bool operator() (const dtn::data::MetaBundle& lhs, const dtn::data::MetaBundle& rhs) const
				{
					if (lhs.getPriority() > rhs.getPriority())
						return true;

					if (lhs.getPriority() != rhs.getPriority())
						return false;

					if (lhs.timestamp < rhs.timestamp)
						return true;

					if (lhs.timestamp != rhs.timestamp)
						return false;

					if (lhs.sequencenumber < rhs.sequencenumber)
						return true;

					if (lhs.sequencenumber != rhs.sequencenumber)
						return false;

					if (rhs.fragment)
					{
						if (!lhs.fragment) return true;
						return (lhs.offset < rhs.offset);
					}

					return false;
				}
			};

			std::set<dtn::data::MetaBundle, CMP_BUNDLE_PRIORITY> _priority_index;
		};
	}
}

#endif /*SIMPLEBUNDLESTORAGE_H_*/
