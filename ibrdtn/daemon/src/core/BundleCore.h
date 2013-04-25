/*
 * BundleCore.h
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

#ifndef BUNDLECORE_H_
#define BUNDLECORE_H_

#include "Component.h"
#include "Configuration.h"

#include "core/EventReceiver.h"
#include "core/StatusReportGenerator.h"
#include "storage/BundleStorage.h"
#include "core/WallClock.h"
#include "routing/BaseRouter.h"

#include "net/ConnectionManager.h"
#include "net/ConvergenceLayer.h"

#include <ibrdtn/data/Serializer.h>
#include <ibrdtn/data/EID.h>

#include <ibrcommon/link/LinkManager.h>

#include <vector>
#include <set>
#include <map>

using namespace dtn::data;

namespace dtn
{
	namespace core
	{
		class P2PDialupException : public ibrcommon::Exception
		{
			public:
				P2PDialupException(string what = "No path known except of dial-up connections.") throw() : Exception(what)
				{
				};
		};

		/**
		 * The BundleCore manage the Bundle Protocol basics
		 */
		class BundleCore : public dtn::daemon::IntegratedComponent, public dtn::core::EventReceiver, public dtn::data::Validator, public ibrcommon::LinkManager::EventCallback, public dtn::daemon::Configuration::OnChangeListener
		{
		public:
			static const std::string TAG;

			static dtn::data::EID local;

			static BundleCore& getInstance();

			WallClock& getClock();

			virtual void onConfigurationChanged(const dtn::daemon::Configuration &conf) throw ();

			void setStorage(dtn::storage::BundleStorage *storage);
			dtn::storage::BundleStorage& getStorage();

			void setSeeker(dtn::storage::BundleSeeker *seeker);
			dtn::storage::BundleSeeker& getSeeker();

			void setRouter(dtn::routing::BaseRouter *router);
			dtn::routing::BaseRouter& getRouter() const;

			void transferTo(const dtn::data::EID &destination, const dtn::data::BundleID &bundle) throw (P2PDialupException);

			/**
			 * Make the connection manager available to other modules.
			 * @return The connection manager reference
			 */
			dtn::net::ConnectionManager& getConnectionManager();

			/**
			 * Add a static route to the static routing module.
			 * @param destination
			 * @param nexthop
			 * @param timeout
			 */
			void addRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop, size_t timeout = 0);

			/**
			 * Remove a static route from the static routing module.
			 * @param destination
			 * @param nexthop
			 */
			void removeRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop);

			/**
			 * Determine if this daemon is globally connected.
			 * @return True, if the daemon is globally connected.
			 */
			bool isGloballyConnected() const;

			void raiseEvent(const dtn::core::Event *evt) throw ();

			virtual void validate(const dtn::data::PrimaryBlock &obj) const throw (RejectedException);
			virtual void validate(const dtn::data::Block &obj, const size_t length) const throw (RejectedException);
			virtual void validate(const dtn::data::PrimaryBlock &bundle, const dtn::data::Block &obj, const size_t length) const throw (RejectedException);
			virtual void validate(const dtn::data::Bundle &obj) const throw (RejectedException);
			virtual void validate(const dtn::data::MetaBundle &obj) const throw (RejectedException);

			/**
			 * Define a global block size limit. This is used in the validator to reject bundles while receiving.
			 */
			static size_t blocksizelimit;

			/**
			 * Define the maximum lifetime for accepted bundles
			 */
			static size_t max_lifetime;

			/**
			 * Define the maximum offset for the timestamp of pre-dated bundles
			 */
			static size_t max_timestamp_future;

			/**
			 * Define if forwarding is allowed. If set to false, this daemon only accepts bundles for local applications.
			 */
			static bool forwarding;


			/**
			 * Defines how many bundles should be in transit at once
			 */
			static size_t max_bundles_in_transit;

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			static void processBlocks(dtn::data::Bundle &b);

			void setGloballyConnected(bool val);

			void eventNotify(const ibrcommon::LinkEvent &evt);

		protected:
			virtual void componentUp() throw ();
			virtual void componentDown() throw ();

		private:
			/**
			 * default constructor
			 */
			BundleCore();

			/**
			 * destructor
			 */
			virtual ~BundleCore();

			/**
			 * Check if we are connected to the internet.
			 */
			void check_connection_state() throw ();

			/**
			 * Forbidden copy constructor
			 */
			BundleCore operator=(const BundleCore &k) { return k; };

			/**
			 * This is a clock object. It can be used to synchronize methods to the local clock.
			 */
			WallClock _clock;

			dtn::storage::BundleStorage *_storage;
			dtn::storage::BundleSeeker *_seeker;
			dtn::routing::BaseRouter *_router;

			// generator for statusreports
			StatusReportGenerator _statusreportgen;

			// manager class for connections
			dtn::net::ConnectionManager _connectionmanager;

			/**
			 * In this boolean we store the connection state.
			 * The variable is true if we are connected via a global address.
			 */
			bool _globally_connected;
		};
	}
}

#endif /*BUNDLECORE_H_*/

