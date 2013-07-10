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
#include "Configuration.h"
#include "core/EventDispatcher.h"
#include "core/BundleCore.h"
#include "core/GlobalEvent.h"
#include "core/BundleEvent.h"
#include "core/BundlePurgeEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "routing/RequeueBundleEvent.h"
#include "routing/QueueBundleEvent.h"
#include "routing/StaticRouteChangeEvent.h"
#include "core/TimeAdjustmentEvent.h"

#include <ibrcommon/data/BLOB.h>
#include <ibrdtn/data/CustodySignalBlock.h>
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
		const std::string BundleCore::TAG = "BundleCore";
		dtn::data::EID BundleCore::local;

		dtn::data::Length BundleCore::blocksizelimit = 0;
		dtn::data::Length BundleCore::foreign_blocksizelimit = 0;
		dtn::data::Length BundleCore::max_lifetime = 0;
		dtn::data::Length BundleCore::max_timestamp_future = 0;
		dtn::data::Size BundleCore::max_bundles_in_transit = 5;

		bool BundleCore::forwarding = true;

		BundleCore& BundleCore::getInstance()
		{
			static BundleCore instance;
			return instance;
		}

		BundleCore::BundleCore()
		 : _clock(1), _storage(NULL), _seeker(NULL), _router(NULL), _globally_connected(true)
		{
			dtn::core::EventDispatcher<dtn::routing::QueueBundleEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::BundlePurgeEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::TimeAdjustmentEvent>::add(this);
			dtn::core::EventDispatcher<dtn::net::TransferCompletedEvent>::add(this);
			dtn::core::EventDispatcher<dtn::net::TransferAbortedEvent>::add(this);
		}

		BundleCore::~BundleCore()
		{
			dtn::core::EventDispatcher<dtn::routing::QueueBundleEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::BundlePurgeEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::TimeAdjustmentEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::net::TransferCompletedEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::net::TransferAbortedEvent>::remove(this);
		}

		void BundleCore::componentUp() throw ()
		{
			// routine checked for throw() on 15.02.2013
			onConfigurationChanged(dtn::daemon::Configuration::getInstance());

			_connectionmanager.initialize();
			_clock.initialize();

			// start a clock
			_clock.startup();
		}

		void BundleCore::componentDown() throw ()
		{
			ibrcommon::LinkManager::getInstance().removeEventListener(this);

			_connectionmanager.terminate();
			_clock.terminate();
		}

		void BundleCore::onConfigurationChanged(const dtn::daemon::Configuration &config) throw ()
		{
			// set the timezone
			dtn::utils::Clock::setTimezone(config.getTimezone());

			// set local eid
			dtn::core::BundleCore::local = config.getNodename();
			IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Local node name: " << config.getNodename() << IBRCOMMON_LOGGER_ENDL;

			// set block size limit
			dtn::core::BundleCore::blocksizelimit = config.getLimit("blocksize");
			if (dtn::core::BundleCore::blocksizelimit > 0)
			{
				IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Block size limited to " << dtn::core::BundleCore::blocksizelimit << " bytes" << IBRCOMMON_LOGGER_ENDL;
			}

			// set block size limit
			dtn::core::BundleCore::foreign_blocksizelimit = config.getLimit("foreign_blocksize");
			if (dtn::core::BundleCore::foreign_blocksizelimit > 0)
			{
				IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Block size of foreign bundles limited to " << dtn::core::BundleCore::foreign_blocksizelimit << " bytes" << IBRCOMMON_LOGGER_ENDL;
			}

			// set the lifetime limit
			dtn::core::BundleCore::max_lifetime = config.getLimit("lifetime");
			if (dtn::core::BundleCore::max_lifetime > 0)
			{
				IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Lifetime limited to " << dtn::core::BundleCore::max_lifetime << " seconds" << IBRCOMMON_LOGGER_ENDL;
			}

			// set the timestamp limit
			dtn::core::BundleCore::max_timestamp_future = config.getLimit("predated_timestamp");
			if (dtn::core::BundleCore::max_timestamp_future > 0)
			{
				IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Pre-dated timestamp limited to " << dtn::core::BundleCore::max_timestamp_future << " seconds in the future" << IBRCOMMON_LOGGER_ENDL;
			}

			// set the maximum count of bundles in transit (bundles to send to the CL queue)
			dtn::data::Size transit_limit = config.getLimit("bundles_in_transit");
			if (transit_limit > 0)
			{
				dtn::core::BundleCore::max_bundles_in_transit = transit_limit;
				IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Limit the number of bundles in transit to " << dtn::core::BundleCore::max_bundles_in_transit << IBRCOMMON_LOGGER_ENDL;
			}

			// enable or disable forwarding of bundles
			if (config.getNetwork().doForwarding())
			{
				IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Forwarding of bundles enabled." << IBRCOMMON_LOGGER_ENDL;
				BundleCore::forwarding = true;
			}
			else
			{
				IBRCOMMON_LOGGER_TAG(BundleCore::TAG, info) << "Forwarding of bundles disabled." << IBRCOMMON_LOGGER_ENDL;
				BundleCore::forwarding = false;
			}

			const std::set<ibrcommon::vinterface> &global_nets = config.getNetwork().getInternetDevices();

			// remove myself from all listeners
			ibrcommon::LinkManager::getInstance().removeEventListener(this);

			// add all listener in the configuration
			for (std::set<ibrcommon::vinterface>::const_iterator iter = global_nets.begin(); iter != global_nets.end(); ++iter)
			{
				ibrcommon::LinkManager::getInstance().addEventListener(*iter, this);
			}

			// check connection state
			check_connection_state();
		}

		void BundleCore::setStorage(dtn::storage::BundleStorage *storage)
		{
			_storage = storage;
		}

		void BundleCore::setSeeker(dtn::storage::BundleSeeker *seeker)
		{
			_seeker = seeker;
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

		dtn::storage::BundleSeeker& BundleCore::getSeeker()
		{
			if (_seeker == NULL)
			{
				throw ibrcommon::Exception("No bundle seeker is set! Use BundleCore::setSeeker() to set a seeker.");
			}
			return *_seeker;
		}

		WallClock& BundleCore::getClock()
		{
			return _clock;
		}

		void BundleCore::transferTo(dtn::net::BundleTransfer &transfer) throw (P2PDialupException)
		{
			try {
				_connectionmanager.queue(transfer);
			} catch (const dtn::net::NeighborNotAvailableException &ex) {
				// signal interruption of the transfer
				transfer.abort(dtn::net::TransferAbortedEvent::REASON_CONNECTION_DOWN);
			} catch (const dtn::net::ConnectionNotAvailableException &ex) {
			} catch (const P2PDialupException&) {
				// re-throw the P2PDialupException
				throw;
			} catch (const ibrcommon::Exception&) {
			}
		}

		dtn::net::ConnectionManager& BundleCore::getConnectionManager()
		{
			return _connectionmanager;
		}

		void BundleCore::addRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop, const dtn::data::Timeout timeout)
		{
			dtn::routing::StaticRouteChangeEvent::raiseEvent(dtn::routing::StaticRouteChangeEvent::ROUTE_ADD, nexthop, destination, timeout);
		}

		void BundleCore::removeRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop)
		{
			dtn::routing::StaticRouteChangeEvent::raiseEvent(dtn::routing::StaticRouteChangeEvent::ROUTE_DEL, nexthop, destination);
		}

		bool BundleCore::isGloballyConnected() const
		{
			return _globally_connected;
		}

		void BundleCore::raiseEvent(const dtn::core::Event *evt) throw ()
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

					if (bundle.get(dtn::data::Bundle::APPDATA_IS_ADMRECORD))
					try {
						// check for a custody signal
						dtn::data::PayloadBlock &payload = bundle.find<dtn::data::PayloadBlock>();

						CustodySignalBlock custody;
						custody.read(payload);

						getStorage().releaseCustody(bundle.source, custody.bundleid);

						IBRCOMMON_LOGGER_DEBUG_TAG("BundleCore", 5) << "custody released for " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;

						delivered = true;
					} catch (const AdministrativeBlock::WrongRecordException&) {
						// no custody signal available
					} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
						// no payload block available
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

					if (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
					{
						// delete the bundle
						dtn::core::BundlePurgeEvent::raise(meta);
					}
				}

				return;
			} catch (const dtn::storage::NoBundleFoundException&) {
				return;
			} catch (const std::bad_cast&) {}

			try {
				const dtn::net::TransferCompletedEvent &completed = dynamic_cast<const dtn::net::TransferCompletedEvent&>(*evt);
				const dtn::data::MetaBundle &meta = completed.getBundle();
				const dtn::data::EID &peer = completed.getPeer();

				if ((meta.destination.getNode() == peer.getNode())
						&& (meta.procflags & dtn::data::Bundle::DESTINATION_IS_SINGLETON))
				{
					// bundle has been delivered to its destination
					// delete it from our storage
					dtn::core::BundlePurgeEvent::raise(meta);

					IBRCOMMON_LOGGER_TAG("BundleCore", notice) << "singleton bundle delivered: " << meta.toString() << IBRCOMMON_LOGGER_ENDL;

					// gen a report
					dtn::core::BundleEvent::raise(meta, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);
				}

				return;
			} catch (const std::bad_cast&) { }

			try {
				const dtn::net::TransferAbortedEvent &aborted = dynamic_cast<const dtn::net::TransferAbortedEvent&>(*evt);

				if (aborted.reason != dtn::net::TransferAbortedEvent::REASON_REFUSED) return;

				const dtn::data::EID &peer = aborted.getPeer();
				const dtn::data::BundleID &id = aborted.getBundleID();

				try {
					const dtn::data::MetaBundle meta = this->getStorage().get(id);

					if (!(meta.procflags & dtn::data::Bundle::DESTINATION_IS_SINGLETON)) return;

					// if the bundle has been sent by this module delete it
					if (meta.destination.getNode() == peer.getNode())
					{
						// bundle is not deliverable
						dtn::core::BundlePurgeEvent::raise(meta, dtn::data::StatusReportBlock::NO_KNOWN_ROUTE_TO_DESTINATION_FROM_HERE);
					}
				} catch (const dtn::storage::NoBundleFoundException&) { };

				return;
			} catch (const std::bad_cast&) { };

			try {
				const dtn::core::BundlePurgeEvent &purge = dynamic_cast<const dtn::core::BundlePurgeEvent&>(*evt);

				// get the global storage
				dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();

				try {
					// delete the bundle
					storage.remove(purge.bundle);
				} catch (const dtn::storage::NoBundleFoundException&) { };

				return;
			} catch (const std::bad_cast&) { }

			try {
				const dtn::core::TimeAdjustmentEvent &timeadj = dynamic_cast<const dtn::core::TimeAdjustmentEvent&>(*evt);

				// set the local clock to the new timestamp
				dtn::utils::Clock::setOffset(timeadj.offset);
			} catch (const std::bad_cast&) { }
		}

		void BundleCore::validate(const dtn::data::MetaBundle &obj) const throw (dtn::data::Validator::RejectedException)
		{
			if (dtn::utils::Clock::isExpired(obj.expiretime)) {
				// ... bundle is expired
				IBRCOMMON_LOGGER_DEBUG_TAG("BundleCore", 35) << "bundle rejected: bundle has expired (" << obj.toString() << ")" << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("bundle is expired");
			}

			// check if the lifetime of the bundle is too long
			if (BundleCore::max_lifetime > 0)
			{
				if (obj.lifetime > BundleCore::max_lifetime)
				{
					// ... we reject bundles with such a long lifetime
					IBRCOMMON_LOGGER_DEBUG_TAG("BundleCore", 35) << "lifetime of bundle rejected: " << obj.toString() << IBRCOMMON_LOGGER_ENDL;
					throw dtn::data::Validator::RejectedException("lifetime of the bundle is too long");
				}
			}

			// check if the timestamp is in the future
			if (BundleCore::max_timestamp_future > 0)
			{
				// first check if the local clock is reliable
				if (dtn::utils::Clock::getRating() > 0)
					// then check the timestamp
					if ((dtn::utils::Clock::getTime() + BundleCore::max_timestamp_future) < obj.timestamp)
					{
						// ... we reject bundles with a timestamp so far in the future
						IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "timestamp of bundle rejected: " << obj.toString() << IBRCOMMON_LOGGER_ENDL;
						throw dtn::data::Validator::RejectedException("timestamp is too far in the future");
					}
			}
		}

		void BundleCore::validate(const dtn::data::PrimaryBlock &p) const throw (dtn::data::Validator::RejectedException)
		{
			// if we do not forward bundles
			if (!BundleCore::forwarding)
			{
				if (!p.destination.sameHost(BundleCore::local))
				{
					// ... we reject all non-local bundles.
					IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "non-local bundle rejected: " << p.toString() << IBRCOMMON_LOGGER_ENDL;
					throw dtn::data::Validator::RejectedException("bundle is not local");
				}
			}

			// check if the lifetime of the bundle is too long
			if (BundleCore::max_lifetime > 0)
			{
				if (p.lifetime > BundleCore::max_lifetime)
				{
					// ... we reject bundles with such a long lifetime
					IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "lifetime of bundle rejected: " << p.toString() << IBRCOMMON_LOGGER_ENDL;
					throw dtn::data::Validator::RejectedException("lifetime of the bundle is too long");
				}
			}

			// check if the timestamp is in the future
			if (BundleCore::max_timestamp_future > 0)
			{
				// first check if the local clock is reliable
				if (dtn::utils::Clock::getRating() > 0)
					// then check the timestamp
					if ((dtn::utils::Clock::getTime() + BundleCore::max_timestamp_future) < p.timestamp)
					{
						// ... we reject bundles with a timestamp so far in the future
						IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "timestamp of bundle rejected: " << p.toString() << IBRCOMMON_LOGGER_ENDL;
						throw dtn::data::Validator::RejectedException("timestamp is too far in the future");
					}
			}
		}

		void BundleCore::validate(const dtn::data::Block&, const dtn::data::Number& size) const throw (dtn::data::Validator::RejectedException)
		{
			// check for the size of the block
			// reject a block if it exceeds the payload limit
			if ((BundleCore::blocksizelimit > 0) && (size > BundleCore::blocksizelimit))
			{
				IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "bundle rejected: block size of " << size.toString() << " is too big" << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("block size is too big");
			}
		}

		void BundleCore::validate(const dtn::data::PrimaryBlock &bundle, const dtn::data::Block&, const dtn::data::Number& size) const throw (RejectedException)
		{
			// check for the size of the foreign block
			// reject a block if it exceeds the payload limit
			if (BundleCore::foreign_blocksizelimit > 0) {
				if (size > BundleCore::foreign_blocksizelimit) {
					if (bundle.source.getNode() != dtn::core::BundleCore::local) {
						if (bundle.destination.getNode() != dtn::core::BundleCore::local) {
							IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "foreign bundle " << bundle.toString() << " rejected: block size of " << size.toString() << " is too big" << IBRCOMMON_LOGGER_ENDL;
							throw dtn::data::Validator::RejectedException("foreign block size is too big");
						}
					}
				}
			}

			// check for the size of the block
			// reject a block if it exceeds the payload limit
			if (BundleCore::blocksizelimit > 0)
			{
				if (size > BundleCore::blocksizelimit)
				{
					IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "bundle " << bundle.toString() << " rejected: block size of " << size.toString() << " is too big" << IBRCOMMON_LOGGER_ENDL;
					throw dtn::data::Validator::RejectedException("block size is too big");
				}
			}
		}

		void BundleCore::validate(const dtn::data::Bundle &b) const throw (dtn::data::Validator::RejectedException)
		{
			// reject bundles without destination
			if (b.destination.isNone())
			{
				IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "bundle rejected: the destination is null" << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("bundle destination is none");
			}

			// check if the bundle is expired
			if (dtn::utils::Clock::isExpired(b))
			{
				IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "bundle rejected: bundle has expired (" << b.toString() << ")" << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("bundle is expired");
			}

#ifdef WITH_BUNDLE_SECURITY
			// do a fast security check
			try {
				dtn::security::SecurityManager::getInstance().fastverify(b);
			} catch (const dtn::security::SecurityManager::VerificationFailedException &ex) {
				IBRCOMMON_LOGGER_DEBUG_TAG("BundleCore", 5) << "[bundle rejected] security checks failed, reason: " << ex.what() << ", bundle: " << b.toString() << IBRCOMMON_LOGGER_ENDL;
				throw dtn::data::Validator::RejectedException("security checks failed");
			}
#endif

			// check for invalid blocks
			for (dtn::data::Bundle::const_iterator iter = b.begin(); iter != b.end(); ++iter)
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
			for (dtn::data::Bundle::iterator iter = b.begin(); iter != b.end(); ++iter)
			{
				const dtn::data::Block &block = (**iter);
#ifdef WITH_BUNDLE_SECURITY
				if (block.getType() == dtn::security::PayloadConfidentialBlock::BLOCK_TYPE)
				{
					// try to decrypt the bundle
					try {
						dtn::security::SecurityManager::getInstance().decrypt(b);
					} catch (const dtn::security::SecurityManager::KeyMissingException&) {
						// decrypt needed, but no key is available
						IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "No key available for decrypt bundle." << IBRCOMMON_LOGGER_ENDL;
					} catch (const dtn::security::SecurityManager::DecryptException &ex) {
						// decrypt failed
						IBRCOMMON_LOGGER_TAG("BundleCore", warning) << "Decryption of bundle failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
					break;
				}
#endif

#ifdef WITH_COMPRESSION
				if (block.getType() == dtn::data::CompressedPayloadBlock::BLOCK_TYPE)
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

		void BundleCore::eventNotify(const ibrcommon::LinkEvent &evt)
		{
			const std::set<ibrcommon::vinterface> &global_nets = dtn::daemon::Configuration::getInstance().getNetwork().getInternetDevices();
			if (global_nets.find(evt.getInterface()) != global_nets.end()) {
				check_connection_state();
			}
		}

		void BundleCore::setGloballyConnected(bool val)
		{
			if (val == _globally_connected) return;

			if (val) {
				dtn::core::GlobalEvent::raise(dtn::core::GlobalEvent::GLOBAL_INTERNET_AVAILABLE);
			} else {
				dtn::core::GlobalEvent::raise(dtn::core::GlobalEvent::GLOBAL_INTERNET_UNAVAILABLE);
			}

			_globally_connected = val;
		}

		void BundleCore::check_connection_state() throw ()
		{
			const std::set<ibrcommon::vinterface> &global_nets = dtn::daemon::Configuration::getInstance().getNetwork().getInternetDevices();

			if (global_nets.empty()) {
				// no configured internet devices
				// assume we are connected globally
				setGloballyConnected(true);
			} else {
				bool found = false;
				for (std::set<ibrcommon::vinterface>::const_iterator iter = global_nets.begin(); iter != global_nets.end(); ++iter)
				{
					const ibrcommon::vinterface &iface = (*iter);
					if (!iface.getAddresses(ibrcommon::vaddress::SCOPE_GLOBAL).empty()) {
						found = true;
					}
				}
				setGloballyConnected(found);
			}
		}
	}
}
