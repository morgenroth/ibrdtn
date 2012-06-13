/*
 * ConnectionManager.cpp
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

#include "Configuration.h"
#include "net/ConnectionManager.h"
#include "net/UDPConvergenceLayer.h"
#include "net/TCPConvergenceLayer.h"
#include "net/BundleReceivedEvent.h"
#include "net/ConnectionEvent.h"
#include "core/NodeEvent.h"
#include "core/BundleEvent.h"
#include "core/BundleCore.h"
#include "core/NodeEvent.h"
#include "core/TimeEvent.h"
#include "routing/RequeueBundleEvent.h"

#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/net/tcpserver.h>
#include <ibrcommon/Logger.h>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <typeinfo>

using namespace dtn::core;

namespace dtn
{
	namespace net
	{
		struct CompareNodeDestination:
		public std::binary_function< dtn::core::Node, dtn::data::EID, bool > {
			bool operator() ( const dtn::core::Node &node, const dtn::data::EID &destination ) const {
				return node.getEID() == destination;
			}
		};

		ConnectionManager::ConnectionManager()
		 : _shutdown(false), _next_autoconnect(0)
		{
		}

		ConnectionManager::~ConnectionManager()
		{
		}

		void ConnectionManager::componentUp()
		{
			bindEvent(TimeEvent::className);
			bindEvent(NodeEvent::className);
			bindEvent(ConnectionEvent::className);

			// set next auto connect
			const dtn::daemon::Configuration::Network &nc = dtn::daemon::Configuration::getInstance().getNetwork();
			if (nc.getAutoConnect() != 0)
			{
				_next_autoconnect = dtn::utils::Clock::getTime() + nc.getAutoConnect();
			}
		}

		void ConnectionManager::componentDown()
		{
			{
				ibrcommon::MutexLock l(_cl_lock);
				// clear the list of convergence layers
				_cl.clear();
			}

			unbindEvent(NodeEvent::className);
			unbindEvent(TimeEvent::className);
			unbindEvent(ConnectionEvent::className);
		}

		void ConnectionManager::raiseEvent(const dtn::core::Event *evt)
		{
			try {
				const NodeEvent &nodeevent = dynamic_cast<const NodeEvent&>(*evt);
				const Node &n = nodeevent.getNode();

				switch (nodeevent.getAction())
				{
					case NODE_INFO_UPDATED:
						discovered(n);
						break;

					case NODE_AVAILABLE:
						if (n.doConnectImmediately())
						{
							// open the connection immediately
							open(n);
						}
						break;

					default:
						break;
				}
			} catch (const std::bad_cast&) { }

			try {
				const TimeEvent &timeevent = dynamic_cast<const TimeEvent&>(*evt);

				if (timeevent.getAction() == TIME_SECOND_TICK)
				{
					check_unavailable();
					check_autoconnect();
				}
			} catch (const std::bad_cast&) { }

			try {
				const ConnectionEvent &connection = dynamic_cast<const ConnectionEvent&>(*evt);

				switch (connection.state)
				{
					case ConnectionEvent::CONNECTION_UP:
					{
						ibrcommon::MutexLock l(_node_lock);

						try {
							dtn::core::Node &n = getNode(connection.peer);
							n += connection.node;

							IBRCOMMON_LOGGER_DEBUG(56) << "Node attributes added: " << n << IBRCOMMON_LOGGER_ENDL;
						} catch (const ibrcommon::Exception&) {
							_nodes.push_back(connection.node);

							IBRCOMMON_LOGGER_DEBUG(56) << "New node available: " << connection.node << IBRCOMMON_LOGGER_ENDL;

							// announce the new node
							dtn::core::NodeEvent::raise(connection.node, dtn::core::NODE_AVAILABLE);

						}

						break;
					}

					case ConnectionEvent::CONNECTION_DOWN:
					{
						ibrcommon::MutexLock l(_node_lock);

						try {
							// remove the node from the connected list
							dtn::core::Node &n = getNode(connection.peer);
							n -= connection.node;

							IBRCOMMON_LOGGER_DEBUG(56) << "Node attributes removed: " << n << IBRCOMMON_LOGGER_ENDL;
						} catch (const ibrcommon::Exception&) { };
						break;
					}

					default:
						break;
				}

			} catch (const std::bad_cast&) {

			}
		}

		void ConnectionManager::addConnection(const dtn::core::Node &n)
		{
			ibrcommon::MutexLock l(_node_lock);
			try {
				dtn::core::Node &db = getNode(n.getEID());

				// add all attributes to the node in the database
				db += n;

				IBRCOMMON_LOGGER_DEBUG(56) << "Node attributes added: " << db << IBRCOMMON_LOGGER_ENDL;

			} catch (const ibrcommon::Exception&) {
				_nodes.push_back(n);

				// announce the new node
				dtn::core::NodeEvent::raise(n, dtn::core::NODE_AVAILABLE);
				IBRCOMMON_LOGGER_DEBUG(56) << "New node available: " << n << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void ConnectionManager::removeConnection(const dtn::core::Node &n)
		{
			ibrcommon::MutexLock l(_node_lock);
			try {
				dtn::core::Node &db = getNode(n.getEID());

				// erase all attributes to the node in the database
				db -= n;

				IBRCOMMON_LOGGER_DEBUG(56) << "Node attributes removed: " << db << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::Exception&) { };
		}

		void ConnectionManager::addConvergenceLayer(ConvergenceLayer *cl)
		{
			ibrcommon::MutexLock l(_cl_lock);
			_cl.insert( cl );
		}

		void ConnectionManager::discovered(const dtn::core::Node &node)
		{
			// ignore messages of ourself
			if (node.getEID() == dtn::core::BundleCore::local) return;

			ibrcommon::MutexLock l(_node_lock);

			try {
				dtn::core::Node &db = getNode(node.getEID());

				// add all attributes to the node in the database
				db += node;

				IBRCOMMON_LOGGER_DEBUG(56) << "Node attributes added: " << db << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::Exception&) {
				_nodes.push_back(node);

				// announce the new node
				dtn::core::NodeEvent::raise(node, dtn::core::NODE_AVAILABLE);
				IBRCOMMON_LOGGER_DEBUG(56) << "New node available: " << node << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void ConnectionManager::check_unavailable()
		{
			ibrcommon::MutexLock l(_node_lock);

			// search for outdated nodes
			std::list<dtn::core::Node>::iterator iter = _nodes.begin();
			while ( iter != _nodes.end() )
			{
				dtn::core::Node &n = (*iter);

				if ( n.expire() )
				{
					// announce the unavailable event
					dtn::core::NodeEvent::raise(n, dtn::core::NODE_UNAVAILABLE);

					// remove the element
					_nodes.erase( iter++ );
				}
				else
				{
					iter++;
				}
			}
		}

		void ConnectionManager::check_autoconnect()
		{
			std::queue<dtn::core::Node> _connect_nodes;

			size_t interval = dtn::daemon::Configuration::getInstance().getNetwork().getAutoConnect();
			if (interval == 0) return;

			if (_next_autoconnect < dtn::utils::Clock::getTime())
			{
				// search for non-connected but available nodes
				ibrcommon::MutexLock l(_cl_lock);
				for (std::list<dtn::core::Node>::const_iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
				{
					const Node &n = (*iter);
					std::list<Node::URI> ul = n.get(Node::NODE_CONNECTED, Node::CONN_TCPIP);

					if (ul.empty() && n.isAvailable())
					{
						_connect_nodes.push(n);
					}
				}

				// set the next check time
				_next_autoconnect = dtn::utils::Clock::getTime() + interval;
			}

			while (!_connect_nodes.empty())
			{
				open(_connect_nodes.front());
				_connect_nodes.pop();
			}
		}

		void ConnectionManager::open(const dtn::core::Node &node)
		{
			ibrcommon::MutexLock l(_cl_lock);

			// search for the right cl
			for (std::set<ConvergenceLayer*>::iterator iter = _cl.begin(); iter != _cl.end(); iter++)
			{
				ConvergenceLayer *cl = (*iter);
				if (node.has(cl->getDiscoveryProtocol()))
				{
					cl->open(node);

					// stop here, we queued the bundle already
					return;
				}
			}

			throw ConnectionNotAvailableException();
		}

		void ConnectionManager::queue(const dtn::core::Node &node, const ConvergenceLayer::Job &job)
		{
			ibrcommon::MutexLock l(_cl_lock);

			// search for the right cl
			for (std::set<ConvergenceLayer*>::iterator iter = _cl.begin(); iter != _cl.end(); iter++)
			{
				ConvergenceLayer *cl = (*iter);
				if (node.has(cl->getDiscoveryProtocol()))
				{
					cl->queue(node, job);

					// stop here, we queued the bundle already
					return;
				}
			}

			throw ConnectionNotAvailableException();
		}

		void ConnectionManager::queue(const ConvergenceLayer::Job &job)
		{
			ibrcommon::MutexLock l(_node_lock);

			if (IBRCOMMON_LOGGER_LEVEL >= 50)
			{
				IBRCOMMON_LOGGER_DEBUG(50) << "## node list ##" << IBRCOMMON_LOGGER_ENDL;
				for (std::list<dtn::core::Node>::const_iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
				{
					const dtn::core::Node &n = (*iter);
					IBRCOMMON_LOGGER_DEBUG(2) << n << IBRCOMMON_LOGGER_ENDL;
				}
			}

			IBRCOMMON_LOGGER_DEBUG(50) << "search for node " << job._destination.getString() << IBRCOMMON_LOGGER_ENDL;

			// queue to a node
			for (std::list<dtn::core::Node>::const_iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
			{
				const Node &n = (*iter);
				if (n == job._destination)
				{
					IBRCOMMON_LOGGER_DEBUG(2) << "next hop: " << n << IBRCOMMON_LOGGER_ENDL;
					queue(n, job);
					return;
				}
			}

			throw NeighborNotAvailableException("No active connection to this neighbor available!");
		}

		void ConnectionManager::queue(const dtn::data::EID &eid, const dtn::data::BundleID &b)
		{
			queue( ConvergenceLayer::Job(eid, b) );
		}

		const std::set<dtn::core::Node> ConnectionManager::getNeighbors()
		{
			ibrcommon::MutexLock l(_node_lock);

			std::set<dtn::core::Node> ret;

			for (std::list<dtn::core::Node>::const_iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
			{
				const Node &n = (*iter);
				if (n.isAvailable()) ret.insert( *iter );
			}

			return ret;
		}

		const dtn::core::Node ConnectionManager::getNeighbor(const dtn::data::EID &eid)
		{
			ibrcommon::MutexLock l(_node_lock);

			// search for the node in the node list
			for (std::list<dtn::core::Node>::const_iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
			{
				const Node &n = (*iter);
				if ((n.getEID() == eid) && (n.isAvailable())) return n;
			}

			throw dtn::net::NeighborNotAvailableException();
		}

		bool ConnectionManager::isNeighbor(const dtn::core::Node &node)
		{
			ibrcommon::MutexLock l(_node_lock);

			// search for the node in the node list
			for (std::list<dtn::core::Node>::const_iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
			{
				const Node &n = (*iter);
				if ((n == node) && (n.isAvailable())) return true;
			}

			return false;
		}

		const std::string ConnectionManager::getName() const
		{
			return "ConnectionManager";
		}

		dtn::core::Node& ConnectionManager::getNode(const dtn::data::EID &eid)
		{
			for (std::list<dtn::core::Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
			{
				dtn::core::Node &n = (*iter);
				if (n == eid) return n;
			}

			throw ibrcommon::Exception("neighbor not found");
		}
	}
}
