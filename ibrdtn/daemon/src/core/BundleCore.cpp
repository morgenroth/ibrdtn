/*
 * BundleCore.cpp
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

#include "config.h"
#include "core/BundleCore.h"
#include "core/GlobalEvent.h"
#include "core/BundleEvent.h"
#include "core/BundlePurgeEvent.h"
#include "net/TransferAbortedEvent.h"
#include "routing/RequeueBundleEvent.h"
#include "routing/QueueBundleEvent.h"
#include "routing/StaticRouteChangeEvent.h"
#include "core/TimeAdjustmentEvent.h"

#include <ibrcommon/data/BLOB.h>
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/Exceptions.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>

#include "limits.h"
#include <iostream>
#include <typeinfo>
#include <stdint.h>

#ifdef WITH_BUNDLE_SECURITY
#include "security/SecurityManager.h"
#include <ibrdtn/security/PayloadConfidentialBlock.h>
#endif

#ifdef WITH_COMPRESSION
#include <ibrdtn/data/CompressedPayloadBlock.h>
#endif

using namespace dtn::data;
using namespace dtn::utils;
using namespace std;

namespace dtn
{
	namespace core
	{
		dtn::data::EID BundleCore::local;

		size_t BundleCore::blocksizelimit = 0;
		size_t BundleCore::max_lifetime = 0;
		size_t BundleCore::max_timestamp_future = 0;
		size_t BundleCore::max_bundles_in_transit = 5;

		bool BundleCore::forwarding = true;

		BundleCore& BundleCore::getInstance()
		{
			static BundleCore instance;
			return instance;
		}

		BundleCore::BundleCore()
		 : _clock(1), _storage(NULL), _router(NULL)
		{
			/**
			 * evaluate the current local time
			 */
			if (dtn::utils::Clock::getTime() > 0)
			{
				dtn::utils::Clock::rating = 1;
			}
			else
			{
				IBRCOMMON_LOGGER(warning) << "The local clock seems to be wrong. Expiration disabled." << IBRCOMMON_LOGGER_ENDL;
			}

			bindEvent(dtn::routing::QueueBundleEvent::className);
			bindEvent(dtn::core::BundlePurgeEvent::className);
			bindEvent(dtn::core::TimeAdjustmentEvent::className);
		}

		BundleCore::~BundleCore()
		{
			unbindEvent(dtn::routing::QueueBundleEvent::className);
			unbindEvent(dtn::core::BundlePurgeEvent::className);
			unbindEvent(dtn::core::TimeAdjustmentEvent::className);
		}

		void BundleCore::componentUp()
		{
			_connectionmanager.initialize();
			_clock.initialize();

			// start a clock
			_clock.startup();
		}

		void BundleCore::componentDown()
		{
			_connectionmanager.terminate();
			_clock.terminate();
		}

		void BundleCore::setStorage(dtn::storage::BundleStorage *storage)
		{
			_storage = storage;
		}

		void BundleCore::setRouter(dtn::routing::BaseRouter *router)
		{
			_router = router;
		}

		dtn::routing::BaseRouter& BundleCore::getRouter() const
		{
			if (_router == NULL)
			{
				throw ibrcommon::Exception("router not set");
			}

			return *_router;
		}

		dtn::storage::BundleStorage& BundleCore::getStorage()
		{
			if (_storage == NULL)
			{
				throw ibrcommon::Exception("No bundle storage is set! Use BundleCore::setStorage() to set a storage.");
			}
			return *_storage;
		}

		WallClock& BundleCore::getClock()
		{
			return _clock;
		}

		void BundleCore::transferTo(const dtn::data::EID &destination, const dtn::data::BundleID &bundle)
		{
			try {
				_connectionmanager.queue(destination, bundle);
			} catch (const dtn::net::NeighborNotAvailableException &ex) {
				// signal interruption of the transfer
				dtn::net::TransferAbortedEvent::raise(destination, bundle, dtn::net::TransferAbortedEvent::REASON_CONNECTION_DOWN);
			} catch (const dtn::net::ConnectionNotAvailableException &ex) {
				// signal interruption of the transfer
				dtn::routing::RequeueBundleEvent::raise(destination, bundle);
			} catch (const ibrcommon::Exception&) {
				dtn::routing::RequeueBundleEvent::raise(destination, bundle);
			}
		}

		void BundleCore::addConvergenceLayer(dtn::net::ConvergenceLayer *cl)
		{
			_connectionmanager.addConvergenceLayer(cl);
		}

		void BundleCore::addConnection(const dtn::core::Node &n)
		{
			_connectionmanager.addConnection(n);
		}

		void BundleCore::removeConnection(const dtn::core::Node &n)
		{
			_connectionmanager.removeConnection(n);
		}

		void BundleCore::addRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop, size_t timeout)
		{
			dtn::routing::StaticRouteChangeEvent::raiseEvent(dtn::routing::StaticRouteChangeEvent::ROUTE_ADD, nexthop, destination, timeout);
		}

		void BundleCore::removeRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop)
		{
			dtn::routing::StaticRouteChangeEvent::raiseEvent(dtn::routing::StaticRouteChangeEvent::ROUTE_DEL, nexthop, destination);
		}

		const std::set<dtn::core::Node> BundleCore::getNeighbors()
		{
			return _connectionmanager.getNeighbors();
		}

		bool BundleCore::isNeighbor(const dtn::core::Node &node)
		{
			return _connectionmanager.isNeighbor(node);
		}

		const dtn::core::Node BundleCore::getNeighbor(const dtn::data::EID &eid)
		{
			return _connectionmanager.getNeighbor(eid);
		}

		void BundleCore::raiseEvent(const dtn::core::Event *evt)
		{
			try {
				const dtn::routing::QueueBundleEvent &queued = dynamic_cast<const dtn::routing::QueueBundleEvent&>(*evt);

				// get reference to the meta data of the bundle
				const dtn::data::MetaBundle &meta = queued.bundle;

				// ignore fragments
				if (meta.fragment) return;

				// if the destination is equal this node...
				if (meta.destination == local)
				{
					// if the delivered variable is still false at the end of this block,
					// we discard the bundle
					bool delivered = false;

					// process this bundle locally
					dtn::data::Bundle bundle = getStorage().get(meta);

					try {
						// check for a custody signal
						dtn::data::CustodySignalBlock custody = bundle.getBlock<dtn::data::CustodySignalBlock>();
						dtn::data::BundleID id(custody._source, custody._bundle_timestamp.getValue(), custody._bundle_sequence.getValue(), (custody._fragment_length.getValue() > 0), custody._fragment_offset.getValue());
						getStorage().releaseCustody(bundle._source, id);

						IBRCOMMON_LOGGER_DEBUG(5) << "custody released for " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

						delivered = true;
					} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
						// no custody signal available
					}

					if (delivered)
					{
						// gen a report
						dtn::core::BundleEvent::raise(meta, BUNDLE_DELIVERED);
					}
					else
					{
						// gen a report
						dtn::core::BundleEvent::raise(meta, BUNDLE_DELETED, StatusReportBlock::DESTINATION_ENDPOINT_ID_UNINTELLIGIBLE);
					}

					// delete the bundle
					dtn::core::BundlePurgeEvent::raise(meta);
				}

				return;
			} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
			} catch (const std::bad_cast&) {}

			try {
				const dtn::core::BundlePurgeEvent &purge = dynamic_cast<const dtn::core::BundlePurgeEvent&>(*evt);

				// get the global storage
				dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();

				// delete the bundle
				storage.remove(purge.bundle);

				return;
			} catch (const std::bad_cast&) { }

			try {
				const dtn::core::TimeAdjustmentEvent &timeadj = dynamic_cast<const dtn::core::TimeAdjustmentEvent&>(*evt);

				// set the local clock to the new timestamp
				dtn::utils::Clock::setOffset(timeadj.offset);

				IBRCOMMON_LOGGER(info) << "time adjusted by " << timeadj.offset.tv_sec << "." << timeadj.offset.tv_usec << "; new rating: " << timeadj.rating << IBRCOMMON_LOGGER_ENDL;
			} catch (const std::bad_cast&) { }
		}

		void BundleCore::validate(const dtn::data::PrimaryBlock &p) const throw (dtn::data::Validator::RejectedException)
		{
			/*
			 *
			 * TODO: reject a bundle if...
			 * ... it is expired (moved to validate(Bundle) to support AgeBlock for expiration)
			 * ... already in the storage
			 * ... a fragment of an already received bundle in the storage
			 *
			 * throw dtn::data::DefaultDeserializer::RejectedException();
			 *
			 */

			// if we do not forward bundles
			if (!BundleCore::forwarding)
			{
				if (!p._destination.sameHost(BundleCore::local))
				{
					// ... we reject all non-local bundles.
					IBRCOMMON_LOGGER(warning) << "non-local bundle rejected: " << p.toString() << IBRCOMMON_LOGGER_ENDL;
					throw dtn::data::Validator::RejectedException("bundle is not local");
				}
			}

			// check if the lifetime of the bundle is too long
			if (BundleCore::max_lifetime > 0)
			{
				if (p._lifetime > BundleCore::max_lifetime)
				{
					// ... we reject bundles with such a long lifetime
					IBRCOMMON_LOGGER(warning) << "lifetime of bundle rejected: " << p.toString() << IBRCOMMON_LOGGER_ENDL;
					throw dtn::data::Validator::RejectedException("lifetime of the bundle is too long");
				}
			}

			// check if the timestamp is in the future
			if (BundleCore::max_timestamp_future > 0)
			{
				// first check if the local clock is reliable
				if (dtn::utils::Clock::rating > 0)
					// then check the timestamp
					if ((dtn::utils::Clock::getTime() + BundleCore::max_timestamp_future) < p._timestamp)
					{
						// ... we reject bundles with a timestamp so far in the future
						IBRCOMMON_LOGGER(warning) << "timestamp of bundle rejected: " << p.toString() << IBRCOMMON_LOGGER_ENDL;
						throw dtn::data::Validator::RejectedException("timestamp is too far in the future");
					}
			}
		}

		void BundleCore::validate(const dtn::data::Block&, const size_t size) const throw (dtn::data::Validator::RejectedException)
		{
			/*
			 *
			 * reject a block if
			 * ... it exceeds the payload limit
			 *
			 * throw dtn::data::DefaultDeserializer::RejectedException();
			 *
			 */

			// check for the size of the block
			if ((BundleCore::blocksizelimit > 0) && (size > BundleCore::blocksizelimit))
			{
				IBRCOMMON_LOGGER(warning) << "bundle rejected: block size of " << size << " is too big" << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("block size is too big");
			}
		}

		void BundleCore::validate(const dtn::data::Bundle &b) const throw (dtn::data::Validator::RejectedException)
		{
			/*
			 *
			 * TODO: reject a bundle if
			 * ... the security checks (DTNSEC) failed
			 * ... a checksum mismatch is detected (CRC)
			 *
			 * throw dtn::data::DefaultDeserializer::RejectedException();
			 *
			 */

			// check if the bundle is expired
			if (dtn::utils::Clock::isExpired(b))
			{
				IBRCOMMON_LOGGER(warning) << "bundle rejected: bundle has expired (" << b.toString() << ")" << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("bundle is expired");
			}

#ifdef WITH_BUNDLE_SECURITY
			// do a fast security check
			try {
				dtn::security::SecurityManager::getInstance().fastverify(b);
			} catch (const dtn::security::SecurityManager::VerificationFailedException &ex) {
				IBRCOMMON_LOGGER_DEBUG(5) << "[bundle rejected] security checks failed, reason: " << ex.what() << ", bundle: " << b.toString() << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("security checks failed");
			}
#endif

			// check for invalid blocks
			const std::list<const dtn::data::Block*> bl = b.getBlocks();
			for (std::list<const dtn::data::Block*>::const_iterator iter = bl.begin(); iter != bl.end(); iter++)
			{
				try {
					const dtn::data::ExtensionBlock &e = dynamic_cast<const dtn::data::ExtensionBlock&>(**iter);

					if (e.get(dtn::data::Block::DELETE_BUNDLE_IF_NOT_PROCESSED))
					{
						// reject the hole bundle
						throw dtn::data::Validator::RejectedException("bundle contains unintelligible blocks");
					}

					if (e.get(dtn::data::Block::TRANSMIT_STATUSREPORT_IF_NOT_PROCESSED))
					{
						// transmit status report, because we can not process this block
						dtn::core::BundleEvent::raise(b, BUNDLE_RECEIVED, dtn::data::StatusReportBlock::BLOCK_UNINTELLIGIBLE);
					}
				} catch (const std::bad_cast&) { }
			}
		}

		const std::string BundleCore::getName() const
		{
			return "BundleCore";
		}

		void BundleCore::processBlocks(dtn::data::Bundle &b)
		{
			// walk through the block and process them when needed
			const std::list<const dtn::data::Block*> blist = b.getBlocks();

			for (std::list<const dtn::data::Block*>::const_iterator iter = blist.begin(); iter != blist.end(); iter++)
			{
				const dtn::data::Block &block = (**iter);
				switch (block.getType())
				{
#ifdef WITH_BUNDLE_SECURITY
					case dtn::security::PayloadConfidentialBlock::BLOCK_TYPE:
					{
						// try to decrypt the bundle
						try {
							dtn::security::SecurityManager::getInstance().decrypt(b);
						} catch (const dtn::security::SecurityManager::KeyMissingException&) {
							// decrypt needed, but no key is available
							IBRCOMMON_LOGGER(warning) << "No key available for decrypt bundle." << IBRCOMMON_LOGGER_ENDL;
						} catch (const dtn::security::SecurityManager::DecryptException &ex) {
							// decrypt failed
							IBRCOMMON_LOGGER(warning) << "Decryption of bundle failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						}
						break;
					}
#endif

#ifdef WITH_COMPRESSION
					case dtn::data::CompressedPayloadBlock::BLOCK_TYPE:
					{
						// try to decompress the bundle
						try {
							dtn::data::CompressedPayloadBlock::extract(b);
						} catch (const ibrcommon::Exception&) { };
						break;
					}
#endif
				}
			}
		}
	}
}
