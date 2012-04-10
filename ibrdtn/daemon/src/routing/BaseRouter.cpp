/*
 * BaseRouter.cpp
 *
 *  Created on: 15.02.2010
 *      Author: morgenro
 */

#include "config.h"
#include "routing/BaseRouter.h"
#include "core/BundleCore.h"

#include "net/TransferAbortedEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/BundleReceivedEvent.h"
#include "net/ConnectionEvent.h"
#include "routing/QueueBundleEvent.h"
#include "routing/RequeueBundleEvent.h"
#include "storage/BundleStorage.h"
#include "core/BundleGeneratedEvent.h"
#include "core/BundleExpiredEvent.h"
#include "core/BundleEvent.h"
#include "core/NodeEvent.h"
#include "core/TimeEvent.h"
#include "routing/NodeHandshakeEvent.h"
#include "routing/StaticRouteChangeEvent.h"

#include "routing/StaticRoutingExtension.h"
#include "routing/NeighborRoutingExtension.h"
#include "routing/NodeHandshakeExtension.h"
#include "routing/RetransmissionExtension.h"

#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/MutexLock.h>

#ifdef WITH_BUNDLE_SECURITY
#include "security/SecurityManager.h"
#endif

namespace dtn
{
	namespace routing
	{
		BaseRouter *BaseRouter::Extension::_router = NULL;

		/**
		 * base implementation of the ThreadedExtension class
		 */
		BaseRouter::ThreadedExtension::ThreadedExtension()
		{ }

		BaseRouter::ThreadedExtension::~ThreadedExtension()
		{
			join();
		}

		/**
		 * base implementation of the Extension class
		 */
		BaseRouter::Extension::Extension()
		{ }

		BaseRouter::Extension::~Extension()
		{ }

		BaseRouter& BaseRouter::Extension::operator*()
		{
			return *_router;
		}

		/**
		 * Transfer one bundle to another node.
		 * @param destination The EID of the other node.
		 * @param id The ID of the bundle to transfer. This bundle must be stored in the storage.
		 */
		void BaseRouter::Extension::transferTo(const dtn::data::EID &destination, const dtn::data::BundleID &id)
		{
			// lock the list of neighbors
			ibrcommon::MutexLock l(_router->getNeighborDB());

			// get the neighbor entry for the next hop
			NeighborDatabase::NeighborEntry &entry = _router->getNeighborDB().get(destination);

			// transfer bundle to the neighbor
			transferTo(entry, id);
		}

		void BaseRouter::Extension::transferTo(NeighborDatabase::NeighborEntry &entry, const dtn::data::BundleID &id)
		{
			// acquire the transfer of this bundle, could throw already in transit or no resource left exception
			entry.acquireTransfer(id);

			// transfer the bundle to the next hop
			dtn::core::BundleCore::getInstance().transferTo(entry.eid, id);
		}

		bool BaseRouter::Extension::isRouting(const dtn::data::EID &eid)
		{
			if (eid.getApplication() == "routing")
			{
				return true;
			}
			else if ((eid.getScheme() == dtn::data::EID::CBHE_SCHEME) && (eid.getApplication() == "50"))
			{
				return true;
			}

			return false;
		}

		/**
		 * base implementation of the Endpoint class
		 */
		BaseRouter::Endpoint::Endpoint()
		{ }

		BaseRouter::Endpoint::~Endpoint()
		{ }

		/**
		 * implementation of the VirtualEndpoint class
		 */
		BaseRouter::VirtualEndpoint::VirtualEndpoint(dtn::data::EID name)
		 : _client(NULL), _name(name)
		{ }

		BaseRouter::VirtualEndpoint::~VirtualEndpoint()
		{ }

		/**
		 * implementation of the BaseRouter class
		 */
		BaseRouter::BaseRouter(dtn::storage::BundleStorage &storage)
		 : _storage(storage), _nh_extension(NULL)
		{
			// register myself for all extensions
			Extension::_router = this;

			// add standard routing modules
			_nh_extension = new dtn::routing::NodeHandshakeExtension();
			addExtension( _nh_extension );
			addExtension( new dtn::routing::StaticRoutingExtension() );
			addExtension( new dtn::routing::NeighborRoutingExtension() );
			addExtension( new dtn::routing::RetransmissionExtension() );
		}

		BaseRouter::~BaseRouter()
		{
			// delete all extensions
			for (std::list<BaseRouter::Extension*>::iterator iter = _extensions.begin(); iter != _extensions.end(); iter++)
			{
				delete (*iter);
			}
		}

		/**
		 * Add a routing extension to the routing core.
		 * @param extension
		 */
		void BaseRouter::addExtension(BaseRouter::Extension *extension)
		{
			_extensions.push_back(extension);
		}

		const std::list<BaseRouter::Extension*>& BaseRouter::getExtensions() const
		{
			return _extensions;
		}

		void BaseRouter::componentUp()
		{
			bindEvent(dtn::net::TransferAbortedEvent::className);
			bindEvent(dtn::net::TransferCompletedEvent::className);
			bindEvent(dtn::net::BundleReceivedEvent::className);
			bindEvent(dtn::routing::QueueBundleEvent::className);
			bindEvent(dtn::routing::RequeueBundleEvent::className);
			bindEvent(dtn::routing::NodeHandshakeEvent::className);
			bindEvent(dtn::routing::StaticRouteChangeEvent::className);
			bindEvent(dtn::core::NodeEvent::className);
			bindEvent(dtn::core::BundleExpiredEvent::className);
			bindEvent(dtn::core::TimeEvent::className);
			bindEvent(dtn::core::BundleGeneratedEvent::className);
			bindEvent(dtn::net::ConnectionEvent::className);

			for (std::list<BaseRouter::Extension*>::iterator iter = _extensions.begin(); iter != _extensions.end(); iter++)
			{
				ThreadedExtension *thread = dynamic_cast<ThreadedExtension*>(*iter);

				if (thread != NULL)
				{
					try {
						// run the thread
						thread->start();
					} catch (const ibrcommon::ThreadException &ex) {
						IBRCOMMON_LOGGER(error) << "failed to start component in BaseRouter\n" << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				}
			}
		}

		void BaseRouter::componentDown()
		{
			unbindEvent(dtn::net::TransferAbortedEvent::className);
			unbindEvent(dtn::net::TransferCompletedEvent::className);
			unbindEvent(dtn::net::BundleReceivedEvent::className);
			unbindEvent(dtn::routing::QueueBundleEvent::className);
			unbindEvent(dtn::routing::RequeueBundleEvent::className);
			unbindEvent(dtn::routing::NodeHandshakeEvent::className);
			unbindEvent(dtn::routing::StaticRouteChangeEvent::className);
			unbindEvent(dtn::core::NodeEvent::className);
			unbindEvent(dtn::core::BundleExpiredEvent::className);
			unbindEvent(dtn::core::TimeEvent::className);
			unbindEvent(dtn::core::BundleGeneratedEvent::className);
			unbindEvent(dtn::net::ConnectionEvent::className);

			// stop all extensions
			for (std::list<BaseRouter::Extension*>::iterator iter = _extensions.begin(); iter != _extensions.end(); iter++)
			{
				ThreadedExtension *thread = dynamic_cast<ThreadedExtension*>(*iter);

				if (thread != NULL)
				{
					try {
						// run the thread
						thread->stop();
						thread->join();
					} catch (const ibrcommon::ThreadException &ex) {
						IBRCOMMON_LOGGER(error) << "failed to stop component in BaseRouter\n" << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				}
			}
		}

		/**
		 * method to receive new events from the EventSwitch
		 */
		void BaseRouter::raiseEvent(const dtn::core::Event *evt)
		{
			// If a new neighbor comes available, send him a request for the summary vector
			// If a neighbor went away we can free the stored database
			try {
				const dtn::core::NodeEvent &event = dynamic_cast<const dtn::core::NodeEvent&>(*evt);

				if (event.getAction() == NODE_AVAILABLE)
				{
					ibrcommon::MutexLock l(_neighbor_database);
					_neighbor_database.create( event.getNode().getEID() );
				}
				else if (event.getAction() == NODE_UNAVAILABLE)
				{
					ibrcommon::MutexLock l(_neighbor_database);
					_neighbor_database.reset( event.getNode().getEID() );
				}
			} catch (const std::bad_cast&) { };

			try {
				const dtn::net::TransferCompletedEvent &event = dynamic_cast<const dtn::net::TransferCompletedEvent&>(*evt);

				// if a transfer is completed, then release the transfer resource of the peer
				try {
					try {
						// add this bundle to the purge vector if it is delivered to its destination
						if (( event.getPeer().getNode() == event.getBundle().destination.getNode() ) && (event.getBundle().procflags & dtn::data::Bundle::DESTINATION_IS_SINGLETON))
						{
							IBRCOMMON_LOGGER(notice) << "singleton bundle added to purge vector: " << event.getBundle().toString() << IBRCOMMON_LOGGER_ENDL;

							// add it to the purge vector
							ibrcommon::MutexLock l(_purged_bundles_lock);
							_purged_bundles.add(event.getBundle());
						}
					} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) { };

					// lock the list of neighbors
					ibrcommon::MutexLock l(_neighbor_database);
					NeighborDatabase::NeighborEntry &entry = _neighbor_database.get(event.getPeer());
					entry.releaseTransfer(event.getBundle());

					// add the bundle to the summary vector of the neighbor
					_neighbor_database.addBundle(event.getPeer(), event.getBundle());
				} catch (const NeighborDatabase::NeighborNotAvailableException&) { };

			} catch (const std::bad_cast&) { };

			try {
				const dtn::net::TransferAbortedEvent &event = dynamic_cast<const dtn::net::TransferAbortedEvent&>(*evt);

				// if a tranfer is aborted, then release the transfer resource of the peer
				try {
					// lock the list of neighbors
					ibrcommon::MutexLock l(_neighbor_database);
					NeighborDatabase::NeighborEntry &entry = _neighbor_database.get(event.getPeer());
					entry.releaseTransfer(event.getBundleID());

					if (event.reason == dtn::net::TransferAbortedEvent::REASON_REFUSED)
					{
						const dtn::data::MetaBundle meta = _storage.get(event.getBundleID());

						// add the transferred bundle to the bloomfilter of the receiver
						_neighbor_database.addBundle(event.getPeer(), meta);
					}
				} catch (const NeighborDatabase::NeighborNotAvailableException&) { };
			} catch (const std::bad_cast&) { };

			try {
				const dtn::net::BundleReceivedEvent &received = dynamic_cast<const dtn::net::BundleReceivedEvent&>(*evt);

				// drop bundles to the NULL-destination
				if (received.bundle._destination == EID("dtn:null")) return;

				// Store incoming bundles into the storage
				try {
					if (received.fromlocal)
					{
						// store the bundle into a storage module
						_storage.store(received.bundle);

						// set the bundle as known
						setKnown(received.bundle);

						// raise the queued event to notify all receivers about the new bundle
						QueueBundleEvent::raise(received.bundle, received.peer);
					}
					// if the bundle is not known
					else if (!filterKnown(received.bundle))
					{
#ifdef WITH_BUNDLE_SECURITY
						// security methods modifies the bundle, thus we need a copy of it
						dtn::data::Bundle bundle = received.bundle;

						// lets see if signatures and hashes are correct and remove them if possible
						dtn::security::SecurityManager::getInstance().verify(bundle);

						// prevent loops
						{
							ibrcommon::MutexLock l(_neighbor_database);

							// add the bundle to the summary vector of the neighbor
							_neighbor_database.addBundle(received.peer, received.bundle);
						}

						// store the bundle into a storage module
						_storage.store(bundle);
#else
						// store the bundle into a storage module
						_storage.store(received.bundle);
#endif

						// raise the queued event to notify all receivers about the new bundle
						QueueBundleEvent::raise(received.bundle, received.peer);
					}

					// finally create a bundle received event
					dtn::core::BundleEvent::raise(received.bundle, dtn::core::BUNDLE_RECEIVED);
#ifdef WITH_BUNDLE_SECURITY
				} catch (const dtn::security::SecurityManager::VerificationFailedException &ex) {
					IBRCOMMON_LOGGER(notice) << "Security checks failed, bundle will be dropped: " << received.bundle.toString() << IBRCOMMON_LOGGER_ENDL;
#endif
				} catch (const ibrcommon::IOException &ex) {
					IBRCOMMON_LOGGER(notice) << "Unable to store bundle " << received.bundle.toString() << IBRCOMMON_LOGGER_ENDL;

					// raise BundleEvent because we have to drop the bundle
					dtn::core::BundleEvent::raise(received.bundle, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::DEPLETED_STORAGE);
				} catch (const dtn::storage::BundleStorage::StorageSizeExeededException &ex) {
					IBRCOMMON_LOGGER(notice) << "No space left for bundle " << received.bundle.toString() << IBRCOMMON_LOGGER_ENDL;

					// raise BundleEvent because we have to drop the bundle
					dtn::core::BundleEvent::raise(received.bundle, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::DEPLETED_STORAGE);
				}

				return;
			} catch (const std::bad_cast&) {
			}

			try {
				const dtn::core::BundleGeneratedEvent &generated = dynamic_cast<const dtn::core::BundleGeneratedEvent&>(*evt);
				
				// set the bundle as known
				setKnown(generated.bundle);

				// Store incoming bundles into the storage
				try {
					// store the bundle into a storage module
					_storage.store(generated.bundle);

					// raise the queued event to notify all receivers about the new bundle
 					QueueBundleEvent::raise(generated.bundle, dtn::core::BundleCore::local);
				} catch (const ibrcommon::IOException &ex) {
					IBRCOMMON_LOGGER(notice) << "Unable to store bundle " << generated.bundle.toString() << IBRCOMMON_LOGGER_ENDL;
				} catch (const dtn::storage::BundleStorage::StorageSizeExeededException &ex) {
					IBRCOMMON_LOGGER(notice) << "No space left for bundle " << generated.bundle.toString() << IBRCOMMON_LOGGER_ENDL;
				}

				return;
			} catch (const std::bad_cast&) {
			}

			try {
				const dtn::core::TimeEvent &time = dynamic_cast<const dtn::core::TimeEvent&>(*evt);
				{
					ibrcommon::MutexLock l(_known_bundles_lock);
					_known_bundles.expire(time.getTimestamp());
				}

				{
					ibrcommon::MutexLock l(_purged_bundles_lock);
					_purged_bundles.expire(time.getTimestamp());
				}

				{
					ibrcommon::MutexLock l(_neighbor_database);
					_neighbor_database.expire(time.getTimestamp());
				}
			} catch (const std::bad_cast&) {
			}

			// notify all underlying extensions
			for (std::list<BaseRouter::Extension*>::iterator iter = _extensions.begin(); iter != _extensions.end(); iter++)
			{
				(*iter)->notify(evt);
			}
		}

		/**
		 * Get a bundle out of the storage.
		 * @param id The ID of the bundle.
		 * @return The requested bundle.
		 */
		dtn::data::Bundle BaseRouter::getBundle(const dtn::data::BundleID &id)
		{
			return _storage.get(id);
		}

		void BaseRouter::doHandshake(const dtn::data::EID &eid)
		{
			_nh_extension->doHandshake(eid);
		}

		dtn::storage::BundleStorage& BaseRouter::getStorage()
		{
			return _storage;
		}

		void BaseRouter::setKnown(const dtn::data::MetaBundle &meta)
		{
			ibrcommon::MutexLock l(_known_bundles_lock);
			return _known_bundles.add(meta);
		}

		// set the bundle as known
		bool BaseRouter::isKnown(const dtn::data::BundleID &id)
		{
			ibrcommon::MutexLock l(_known_bundles_lock);
			return _known_bundles.contains(id);
		}

		// check if a bundle is known
		// if the bundle is unkown add it to the known list and return
		// false
		bool BaseRouter::filterKnown(const dtn::data::MetaBundle &meta)
		{
			ibrcommon::MutexLock l(_known_bundles_lock);
			bool ret = _known_bundles.contains(meta);
			if (!ret) _known_bundles.add(meta);

			return ret;
		}

		const SummaryVector BaseRouter::getSummaryVector()
		{
			ibrcommon::MutexLock l(_known_bundles_lock);
			return _known_bundles.getSummaryVector();
		}

		void BaseRouter::addPurgedBundle(const dtn::data::MetaBundle &meta)
		{
			ibrcommon::MutexLock l(_purged_bundles_lock);
			return _purged_bundles.add(meta);
		}

		const SummaryVector BaseRouter::getPurgedBundles()
		{
			ibrcommon::MutexLock l(_purged_bundles_lock);
			return _purged_bundles.getSummaryVector();
		}

		const std::string BaseRouter::getName() const
		{
			return "BaseRouter";
		}

		NeighborDatabase& BaseRouter::getNeighborDB()
		{
			return _neighbor_database;
		}
	}
}
