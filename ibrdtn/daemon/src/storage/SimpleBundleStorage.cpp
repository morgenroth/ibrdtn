/*
 * SimpleBundleStorage.cpp
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

#include "storage/SimpleBundleStorage.h"
#include "core/EventDispatcher.h"
#include "core/TimeEvent.h"
#include "core/GlobalEvent.h"
#include "core/BundleExpiredEvent.h"
#include "core/BundleEvent.h"

#include <ibrdtn/data/AgeBlock.h>
#include <ibrdtn/utils/Utils.h>
#include <ibrcommon/thread/RWLock.h>
#include <ibrcommon/Logger.h>

#include <memory>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>

namespace dtn
{
	namespace storage
	{
		SimpleBundleStorage::SimpleBundleStorage(const ibrcommon::File &workdir, size_t maxsize, size_t buffer_limit)
		 : BundleStorage(maxsize), _list(this), _datastore(*this, workdir, buffer_limit)
		{
		}

		SimpleBundleStorage::~SimpleBundleStorage()
		{
		}

		void SimpleBundleStorage::eventDataStorageStored(const dtn::storage::DataStorage::Hash &hash)
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);
			dtn::data::MetaBundle meta(_pending_bundles[hash]);
			_pending_bundles.erase(hash);
			_stored_bundles[meta] = hash;
		}

		void SimpleBundleStorage::eventDataStorageStoreFailed(const dtn::storage::DataStorage::Hash &hash, const ibrcommon::Exception &ex)
		{
			IBRCOMMON_LOGGER(error) << "store failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;

			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);

			// get the reference to the bundle
			const dtn::data::Bundle &b = _pending_bundles[hash];

			// decrement the storage size
			freeSpace(_bundle_lengths[b]);
			
			// cleanup bundle sizes
			_bundle_lengths.erase(b);

			// raise bundle removed event
			eventBundleRemoved(b);

			// delete the pending bundle
			_pending_bundles.erase(hash);
		}

		void SimpleBundleStorage::eventDataStorageRemoved(const dtn::storage::DataStorage::Hash &hash)
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);

			for (std::map<dtn::data::MetaBundle, DataStorage::Hash>::iterator iter = _stored_bundles.begin();
					iter != _stored_bundles.end(); iter++)
			{
				const DataStorage::Hash &it_hash = iter->second;
				const dtn::data::MetaBundle it_bundle = iter->first;

				if (it_hash == hash)
				{
					// decrement the storage size
					freeSpace(_bundle_lengths[it_bundle]);

					_bundle_lengths.erase(it_bundle);

					// raise bundle removed event
					eventBundleRemoved(it_bundle);

					_stored_bundles.erase(iter);

					return;
				}
			}
		}

		void SimpleBundleStorage::eventDataStorageRemoveFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception &ex)
		{
			IBRCOMMON_LOGGER(error) << "remove failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
		}

		void SimpleBundleStorage::iterateDataStorage(const dtn::storage::DataStorage::Hash &hash, dtn::storage::DataStorage::istream &stream)
		{
			try {
				dtn::data::Bundle bundle;
				dtn::data::DefaultDeserializer ds(*stream);

				// load a bundle into the storage
				ds >> bundle;
				
				// allocate space for the bundle
				size_t bundle_size = (*stream).tellg();
				allocSpace(bundle_size);

				// extract meta data
				dtn::data::MetaBundle meta(bundle);

				// lock the bundle lists
				ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);

				// add the bundle to the stored bundles
				_stored_bundles[meta] = hash;

				// increment the storage size
				_bundle_lengths[meta] = bundle_size;

				// add it to the bundle list
				_list.add(meta);
				_priority_index.insert(meta);

				// raise bundle added event
				eventBundleAdded(bundle);
			} catch (const std::exception&) {
				// report this error to the console
				IBRCOMMON_LOGGER(error) << "Error: Unable to restore bundle in file " << hash.value << IBRCOMMON_LOGGER_ENDL;

				// error while reading file
				_datastore.remove(hash);
			}
		}

		dtn::data::Bundle SimpleBundleStorage::__get(const dtn::data::MetaBundle &meta)
		{
			DataStorage::Hash hash(meta.toString());
			std::map<DataStorage::Hash, dtn::data::Bundle>::iterator it = _pending_bundles.find(hash);

			if (_pending_bundles.end() != it)
			{
				return it->second;
			}

			try {
				DataStorage::istream stream = _datastore.retrieve(hash);

				// load the bundle from the storage
				dtn::data::Bundle bundle;

				// load the bundle from file
				try {
					dtn::data::DefaultDeserializer(*stream) >> bundle;
				} catch (const std::exception &ex) {
					throw dtn::SerializationFailedException("bundle get failed: " + std::string(ex.what()));
				}

				try {
					dtn::data::AgeBlock &agebl = bundle.getBlock<dtn::data::AgeBlock>();

					// modify the AgeBlock with the age of the file
					time_t age = stream.lastaccess() - stream.lastmodify();

					agebl.addSeconds(age);
				} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

				return bundle;
			} catch (const DataStorage::DataNotAvailableException&) { }

			throw NoBundleFoundException();
		}

		void SimpleBundleStorage::componentUp() throw ()
		{
			// routine checked for throw() on 15.02.2013

			// load persistent bundles
			_datastore.iterateAll();

			// some output
			IBRCOMMON_LOGGER(info) << _list.size() << " Bundles restored." << IBRCOMMON_LOGGER_ENDL;

			dtn::core::EventDispatcher<dtn::core::TimeEvent>::add(this);

			try {
				_datastore.start();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_TAG("SimpleBundleStorage", error) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void SimpleBundleStorage::componentDown() throw ()
		{
			// routine checked for throw() on 15.02.2013

			dtn::core::EventDispatcher<dtn::core::TimeEvent>::remove(this);
			try {
				_datastore.wait();
				_datastore.stop();
				_datastore.join();
			} catch (const ibrcommon::Exception &ex) {
				IBRCOMMON_LOGGER_TAG("SimpleBundleStorage", error) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void SimpleBundleStorage::raiseEvent(const dtn::core::Event *evt) throw ()
		{
			try {
				const dtn::core::TimeEvent &time = dynamic_cast<const dtn::core::TimeEvent&>(*evt);
				if (time.getAction() == dtn::core::TIME_SECOND_TICK)
				{
					ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);
					_list.expire(time.getTimestamp());
				}
			} catch (const std::bad_cast&) { }
		}

		const std::string SimpleBundleStorage::getName() const
		{
			return "SimpleBundleStorage";
		}

		bool SimpleBundleStorage::empty()
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READONLY);
			return _list.empty();
		}

		void SimpleBundleStorage::releaseCustody(const dtn::data::EID&, const dtn::data::BundleID&)
		{
			// custody is successful transferred to another node.
			// it is safe to delete this bundle now. (depending on the routing algorithm.)
		}

		unsigned int SimpleBundleStorage::count()
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READONLY);
			return _list.size();
		}

		void SimpleBundleStorage::get(BundleSelector &cb, BundleResult &result) throw (NoBundleFoundException, BundleSelectorException)
		{
			size_t items_added = 0;

			// we have to iterate through all bundles
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READONLY);

			for (std::set<dtn::data::MetaBundle, CMP_BUNDLE_PRIORITY>::const_iterator iter = _priority_index.begin(); (iter != _priority_index.end()) && ((cb.limit() == 0) || (items_added < cb.limit())); iter++)
			{
				const dtn::data::MetaBundle &meta = (*iter);

				if ( cb.shouldAdd(meta) )
				{
					result.put(meta);
					items_added++;
				}
			}

			if (items_added == 0) throw NoBundleFoundException();
		}

		dtn::data::Bundle SimpleBundleStorage::get(const dtn::data::BundleID &id)
		{
			try {
				ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READONLY);

				for (std::set<dtn::data::MetaBundle>::const_iterator iter = _list.begin(); iter != _list.end(); iter++)
				{
					const dtn::data::MetaBundle &meta = (*iter);
					if (id == meta)
					{
						return __get(meta);
					}
				}
			} catch (const dtn::SerializationFailedException &ex) {
				// bundle loading failed
				IBRCOMMON_LOGGER(error) << "Error while loading bundle data: " << ex.what() << IBRCOMMON_LOGGER_ENDL;

				// the bundle is broken, delete it
				remove(id);

				throw BundleStorage::BundleLoadException();
			}

			throw NoBundleFoundException();
		}

		const SimpleBundleStorage::eid_set SimpleBundleStorage::getDistinctDestinations()
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READONLY);
			std::set<dtn::data::EID> ret;
			return ret;
		}

		void SimpleBundleStorage::store(const dtn::data::Bundle &bundle)
		{
			// get the bundle size
			dtn::data::DefaultSerializer s(std::cout);
			size_t bundle_size = s.getLength(bundle);
			
			// allocate space for the bundle
			allocSpace(bundle_size);

			// store the bundle
			std::auto_ptr<BundleContainer> bc(new BundleContainer(bundle));
			DataStorage::Hash hash(*bc);

			// check if this container is too big for us.
			{
				ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);

				// create meta data object
				dtn::data::MetaBundle meta(bundle);

				// accept custody if requested
				try {
					dtn::data::EID custodian = BundleStorage::acceptCustody(bundle);

					// container for the custody accepted bundle
					dtn::data::Bundle ca_bundle = bundle;

					// set the new custodian
					ca_bundle._custodian = custodian;

					// create meta data object
					meta = ca_bundle;

					// add the bundle to the stored bundles
					_pending_bundles[hash] = ca_bundle;
				} catch (const ibrcommon::Exception&) {
					// no custody requested
					// add the bundle to the stored bundles
					_pending_bundles[hash] = bundle;
				}

				// increment the storage size
				_bundle_lengths[meta] = bundle_size;

				// add it to the bundle list
				_list.add(meta);
				_priority_index.insert(meta);
			}

			// raise bundle added event
			eventBundleAdded(bundle);

			// put the bundle into the data store
			_datastore.store(hash, bc.get());
			bc.release();
		}

		void SimpleBundleStorage::remove(const dtn::data::BundleID &id)
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);

			for (std::set<dtn::data::MetaBundle>::const_iterator iter = _list.begin(); iter != _list.end(); iter++)
			{
				if ((*iter) == id)
				{
					// remove item in the bundlelist
					dtn::data::MetaBundle meta = (*iter);

					// remove it from the bundle list
					_list.remove(meta);
					_priority_index.erase(meta);

					DataStorage::Hash hash(meta.toString());

					// create a background task for removing the bundle
					_datastore.remove(hash);

					return;
				}
			}

			throw NoBundleFoundException();
		}

		dtn::data::MetaBundle SimpleBundleStorage::remove(const ibrcommon::BloomFilter &filter)
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);

			for (std::set<dtn::data::MetaBundle>::const_iterator iter = _list.begin(); iter != _list.end(); iter++)
			{
				// remove item in the bundlelist
				const dtn::data::MetaBundle meta = (*iter);

				if ( filter.contains(meta.toString()) )
				{
					// remove it from the bundle list
					_list.remove(meta);
					_priority_index.erase(meta);

					DataStorage::Hash hash(meta.toString());

					// create a background task for removing the bundle
					_datastore.remove(hash);

					return meta;
				}
			}

			throw NoBundleFoundException();
		}

		void SimpleBundleStorage::clear()
		{
			ibrcommon::RWLock l(_bundleslock, ibrcommon::RWMutex::LOCK_READWRITE);

			// mark all bundles for deletion
			for (std::set<dtn::data::MetaBundle>::const_iterator iter = _list.begin(); iter != _list.end(); iter++)
			{
				// remove item in the bundlelist
				const dtn::data::MetaBundle &meta = (*iter);

				DataStorage::Hash hash(meta.toString());

				// create a background task for removing the bundle
				_datastore.remove(hash);

				// raise bundle removed event
				eventBundleRemoved(meta);
			}

			_priority_index.clear();
			_list.clear();

			// set the storage size to zero
			clearSpace();
		}

		void SimpleBundleStorage::eventBundleExpired(const dtn::data::MetaBundle &b) throw ()
		{
			for (std::set<dtn::data::MetaBundle>::const_iterator iter = _list.begin(); iter != _list.end(); iter++)
			{
				if ((*iter) == b)
				{
					// remove the bundle
					const dtn::data::MetaBundle &meta = (*iter);

					DataStorage::Hash hash(meta.toString());

					// create a background task for removing the bundle
					_datastore.remove(hash);

					// remove the bundle off the index
					_priority_index.erase(meta);

					break;
				}
			}

			// raise bundle event
			dtn::core::BundleEvent::raise( b, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::LIFETIME_EXPIRED);

			// raise an event
			dtn::core::BundleExpiredEvent::raise( b );
		}

		SimpleBundleStorage::BundleContainer::BundleContainer(const dtn::data::Bundle &b)
		 : _bundle(b)
		{ }

		SimpleBundleStorage::BundleContainer::~BundleContainer()
		{ }

		std::string SimpleBundleStorage::BundleContainer::getKey() const
		{
			return dtn::data::BundleID(_bundle).toString();
		}

		std::ostream& SimpleBundleStorage::BundleContainer::serialize(std::ostream &stream)
		{
			// get an serializer for bundles
			dtn::data::DefaultSerializer s(stream);

			// length of the bundle
			unsigned int size = s.getLength(_bundle);

			// serialize the bundle
			s << _bundle; stream.flush();

			// check the streams health
			if (!stream.good())
			{
				std::stringstream ss; ss << "Output stream went bad [" << std::strerror(errno) << "]";
				throw dtn::SerializationFailedException(ss.str());
			}

			// get the write position
			if (size > stream.tellp())
			{
				std::stringstream ss; ss << "Not all data were written [" << stream.tellp() << " of " << size << " bytes]";
				throw dtn::SerializationFailedException(ss.str());
			}

			// return the stream, this allows stacking
			return stream;
		}
	}
}
