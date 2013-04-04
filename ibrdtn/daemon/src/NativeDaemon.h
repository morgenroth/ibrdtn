/*
 * NativeDaemon.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "Component.h"
#include "StandByManager.h"
#include "net/IPNDAgent.h"
#include "storage/BundleSeeker.h"
#include <ibrcommon/Exceptions.h>
#include <list>
#include <set>

#ifndef NATIVEDAEMON_H_
#define NATIVEDAEMON_H_

namespace dtn
{
	namespace daemon
	{
		class NativeDaemonCallback {
		public:
			enum States {
				UNINITIALIZED,
				COMPONENTS_CREATED,
				COMPONENTS_INITIALIZED,
				STARTUP_COMPLETED,
				SHUTDOWN_INITIATED,
				COMPONENTS_TERMINATED,
				COMPONENTS_REMOVED
			};

			virtual ~NativeDaemonCallback() = 0;

			virtual void stateChanged(States state) throw () = 0;
		};

		class NativeDaemonException : public ibrcommon::Exception
		{
		public:
			NativeDaemonException(std::string what = "An error happened.") throw() : ibrcommon::Exception(what)
			{
			};
		};

		class NativeDaemon {
		public:
			NativeDaemon(NativeDaemonCallback *statecb = NULL);
			virtual ~NativeDaemon();

			/**
			 * Initialize the daemon modules and components
			 */
			void initialize() throw (NativeDaemonException);

			/**
			 * Execute the daemon main loop. This starts all the
			 * components and blocks until the daemon is shut-down.
			 */
			void main_loop() throw (NativeDaemonException);

			/**
			 * Shut-down the daemon. The call returns immediately.
			 */
			void shutdown() throw ();

			/**
			 * Generate a reload signal
			 */
			void reload() throw ();

			/**
			 * Enable / disable debugging at runtime
			 */
			void setDebug(bool enable) throw ();

			/**
			 * Set the config file and enable default logging capabilities
			 */
			void enableLogging(std::string configPath, std::string defaultTag, int logLevel, int debugVerbosity) throw ();

			/** NATIVE DAEMON METHODS **/
			std::string getLocalUri() throw ();
			std::set<std::string> getNeighbors() throw ();
			void addConnection(std::string eid, std::string protocol, std::string address, std::string service) throw ();
			void removeConnection(std::string eid, std::string protocol, std::string address, std::string service) throw ();

		private:
			void setState(NativeDaemonCallback::States state) throw ();

			void init_blob_storage() throw (NativeDaemonException);
			void init_bundle_storage() throw (NativeDaemonException);
			void init_convergencelayers() throw (NativeDaemonException);
			void init_security() throw (NativeDaemonException);
			void init_global_variables() throw (NativeDaemonException);
			void init_components() throw (NativeDaemonException);
			void init_discovery() throw (NativeDaemonException);
			void init_routing() throw (NativeDaemonException);
			void init_api() throw (NativeDaemonException);

			NativeDaemonCallback *_statecb;

			// list of components
			std::list< dtn::daemon::Component* > _components;

			// stand-by manager
			dtn::daemon::StandByManager *_standby_manager;

			// IP neighbor discovery process
			dtn::net::IPNDAgent *_ipnd;

			// bundle index used to search for new bundles in the routing modules
			dtn::storage::BundleIndex *_bundle_index;
			dtn::storage::BundleSeeker *_bundle_seeker;
		};
	} /* namespace daemon */
} /* namespace dtn */
#endif /* NATIVEDAEMON_H_ */
