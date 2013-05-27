/*
 * ConnectionManager.h
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

#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include "Component.h"
#include "net/ConvergenceLayer.h"
#include "net/P2PDialupExtension.h"
#include "net/BundleReceiver.h"
#include "core/EventReceiver.h"
#include <ibrdtn/data/EID.h>
#include "core/Node.h"
#include <ibrcommon/Exceptions.h>

#include <set>

namespace dtn
{
	namespace net
	{
		class NeighborNotAvailableException : public ibrcommon::Exception
		{
		public:
			NeighborNotAvailableException(string what = "The requested connection is not available.") throw() : ibrcommon::Exception(what)
			{
			};
		};

		class ConnectionNotAvailableException : public ibrcommon::Exception
		{
		public:
			ConnectionNotAvailableException(string what = "The requested connection is not available.") throw() : ibrcommon::Exception(what)
			{
			};
		};

		class ConnectionManager : public dtn::core::EventReceiver, public dtn::daemon::IntegratedComponent
		{
		public:
			ConnectionManager();
			virtual ~ConnectionManager();

			void add(const dtn::core::Node &n);
			void remove(const dtn::core::Node &n);

			/**
			 * Add a convergence layer
			 */
			void add(ConvergenceLayer *cl);

			/**
			 * Remove a convergence layer
			 */
			void remove(ConvergenceLayer *cl);

			/**
			 * Add a p2p dial-up extension
			 */
			void add(P2PDialupExtension *ext);

			/**
			 * Remove a p2p dial-up extension
			 */
			void remove(P2PDialupExtension *ext);

			/**
			 * queue a bundle for transmission
			 */
			void queue(const dtn::net::BundleTransfer &job);

			/**
			 * method to receive new events from the EventSwitch
			 */
			void raiseEvent(const dtn::core::Event *evt) throw ();

			class ShutdownException : public ibrcommon::Exception
			{
			public:
				ShutdownException(string what = "System shutdown") throw() : ibrcommon::Exception(what)
				{
				};
			};

			void open(const dtn::core::Node &node);

			/**
			 * get a set with all neighbors
			 * @return
			 */
			const std::set<dtn::core::Node> getNeighbors();

			/**
			 * Checks if a node is already known as neighbor.
			 * @param
			 * @return
			 */
			bool isNeighbor(const dtn::core::Node&);

			/**
			 * Get the neighbor with the given EID.
			 * @throw dtn::net::NeighborNotAvailableException if the neighbor is not available.
			 * @param eid The EID of the neighbor.
			 * @return A node object with all neighbor data.
			 */
			const dtn::core::Node getNeighbor(const dtn::data::EID &eid) throw (NeighborNotAvailableException);

			/**
			 * Add collected data about a neighbor to the neighbor database.
			 * @param n The node object of the neighbor
			 */
			void updateNeighbor(const dtn::core::Node &n);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

		protected:
			/**
			 * trigger for periodical discovery of nodes
			 * @param node
			 */
			void discovered(const dtn::core::Node &node);

			virtual void componentUp() throw ();
			virtual void componentDown() throw ();

		private:
			/**
			 * establish a dial-up connection to the given node
			 */
			void dialup(const dtn::core::Node &n);

			/**
			 *  queue a bundle for delivery
			 */
			void queue(const dtn::core::Node &node, const dtn::net::BundleTransfer &job);

			/**
			 * checks for timed out nodes
			 */
			void check_unavailable();

			/**
			 * check for available nodes and announce them
			 */
			void check_available();

			/**
			 * auto connect to available nodes
			 */
			void check_autoconnect();

			/**
			 * get node
			 */
			dtn::core::Node& getNode(const dtn::data::EID &eid) throw (NeighborNotAvailableException);

			// mutex for the list of convergence layers
			ibrcommon::Mutex _cl_lock;

			// contains all configured convergence layers
			std::set<ConvergenceLayer*> _cl;

			// dial-up extensions
			ibrcommon::Mutex _dialup_lock;
			std::set<P2PDialupExtension*> _dialups;

			// mutex for the lists of nodes
			ibrcommon::Mutex _node_lock;

			// contains all nodes
			typedef std::map<dtn::data::EID, dtn::core::Node> nodemap;
			nodemap _nodes;

			// next timestamp for autoconnect check
			dtn::data::Timestamp _next_autoconnect;
		};
	}
}

#endif /* CONNECTIONMANAGER_H_ */
