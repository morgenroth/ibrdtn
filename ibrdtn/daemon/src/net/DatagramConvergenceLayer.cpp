/*
 * DatagramConvergenceLayer.cpp
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

#include "net/DatagramConvergenceLayer.h"
#include "net/DatagramConnection.h"

#include "core/BundleCore.h"
#include "core/EventDispatcher.h"
#include "core/TimeEvent.h"
#include "core/NodeEvent.h"

#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/thread/RWLock.h>

#include <string.h>
#include <vector>

namespace dtn
{
	namespace net
	{
		const std::string DatagramConvergenceLayer::TAG = "DatagramConvergenceLayer";

		DatagramConvergenceLayer::DatagramConvergenceLayer(DatagramService *ds)
		 : _service(ds), _running(false), _discovery_sn(0)
		{
			// initialize stats
			addStats("out", 0);
			addStats("in", 0);
		}

		DatagramConvergenceLayer::~DatagramConvergenceLayer()
		{
			// wait until all connections are down
			{
				ibrcommon::MutexLock l(_cond_connections);
				while (_connections.size() != 0) _cond_connections.wait();
			}

			join();
			delete _service;
		}

		dtn::core::Node::Protocol DatagramConvergenceLayer::getDiscoveryProtocol() const
		{
			return _service->getProtocol();
		}

		void DatagramConvergenceLayer::callback_send(DatagramConnection&, const char &flags, const unsigned int &seqno, const std::string &destination, const char *buf, const dtn::data::Length &len) throw (DatagramException)
		{
			// only on sender at once
			ibrcommon::MutexLock l(_send_lock);

			// forward the send request to DatagramService
			_service->send(HEADER_SEGMENT, flags, seqno, destination, buf, len);

			// traffic monitoring
			addStats("out", len);
		}

		void DatagramConvergenceLayer::callback_ack(DatagramConnection&, const unsigned int &seqno, const std::string &destination) throw (DatagramException)
		{
			// only on sender at once
			ibrcommon::MutexLock l(_send_lock);

			// forward the send request to DatagramService
			_service->send(HEADER_ACK, 0, seqno, destination, NULL, 0);
		}

		void DatagramConvergenceLayer::queue(const dtn::core::Node &node, const dtn::net::BundleTransfer &job)
		{
			const std::list<dtn::core::Node::URI> uri_list = node.get(_service->getProtocol());
			if (uri_list.empty()) return;

			// get the first element of the result
			const dtn::core::Node::URI &uri = uri_list.front();

			// some debugging
			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "job queued for " << node.getEID().getString() << IBRCOMMON_LOGGER_ENDL;

			// lock the connection list while working with it
			ibrcommon::RWLock lc(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);

			// get a new or the existing connection for this address
			DatagramConnection &conn = getConnection( uri.value );

			// queue the job to the connection
			conn.queue(job);
		}

		DatagramConnection& DatagramConvergenceLayer::getConnection(const std::string &identifier)
		{
			DatagramConnection *connection = NULL;

			{
				// lock the list of connections while iterating or adding new connections
				ibrcommon::MutexLock l(_cond_connections);

				// Test if connection for this address already exist
				for(connection_list::const_iterator i = _connections.begin(); i != _connections.end(); ++i)
				{
					if ((*i)->getIdentifier() == identifier)
						return *(*i);
				}

				// Connection does not exist, create one and put it into the list
				connection = new DatagramConnection(identifier, _service->getParameter(), (*this));

				// add a new connection to the list of connections
				_connections.push_back(connection);
				_cond_connections.signal(true);
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "Selected identifier: " << connection->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
			connection->start();
			return *connection;
		}

		void DatagramConvergenceLayer::connectionUp(const DatagramConnection *conn)
		{
			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "Up: " << conn->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
		}

		void DatagramConvergenceLayer::connectionDown(const DatagramConnection *conn)
		{
			ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READWRITE);
			ibrcommon::MutexLock lc(_cond_connections);

			const connection_list::iterator i = std::find(_connections.begin(), _connections.end(), conn);

			if (i != _connections.end())
			{
				IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "Down: " << conn->getIdentifier() << IBRCOMMON_LOGGER_ENDL;

				_connections.erase(i);
				_cond_connections.signal(true);
			}
			else
			{
				IBRCOMMON_LOGGER_TAG(DatagramConvergenceLayer::TAG, error) << "Error in " << conn->getIdentifier() << " not found!" << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void DatagramConvergenceLayer::componentUp() throw ()
		{
			// routine checked for throw() on 15.02.2013
			dtn::core::EventDispatcher<dtn::core::TimeEvent>::add(this);
			try {
				_service->bind();
			} catch (const std::exception &e) {
				IBRCOMMON_LOGGER_TAG(DatagramConvergenceLayer::TAG, error) << "bind to " << _service->getInterface().toString() << " failed (" << e.what() << ")" << IBRCOMMON_LOGGER_ENDL;
			}

			// set the running variable
			_running = true;
		}

		void DatagramConvergenceLayer::componentDown() throw ()
		{
			dtn::core::EventDispatcher<dtn::core::TimeEvent>::remove(this);

			// shutdown all connections
			ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);
			ibrcommon::MutexLock l(_cond_connections);
			for(connection_list::const_iterator i = _connections.begin(); i != _connections.end(); ++i)
			{
				(*i)->shutdown();
			}
		}

		void DatagramConvergenceLayer::sendAnnoucement()
		{
			DiscoveryAnnouncement announcement(DiscoveryAnnouncement::DISCO_VERSION_01, dtn::core::BundleCore::local);

			// set sequencenumber
			announcement.setSequencenumber(_discovery_sn);
			_discovery_sn++;

			// clear all services
			announcement.clearServices();

			// serialize announcement
			stringstream ss;
			ss << announcement;

			std::streamsize len = ss.str().size();

			try {
				// only on sender at once
				ibrcommon::MutexLock l(_send_lock);

				// forward the send request to DatagramService
				_service->send(HEADER_BROADCAST, 0, 0, ss.str().c_str(), static_cast<dtn::data::Length>(len));
			} catch (const DatagramException&) {
				// ignore any send failure
			};
		}

		void DatagramConvergenceLayer::componentRun() throw ()
		{
			size_t maxlen = _service->getParameter().max_msg_length;
			std::string address;
			unsigned int seqno = 0;
			char flags = 0;
			char type = 0;
			std::vector<char> data(maxlen);
			size_t len = 0;

			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "componentRun() entered" << IBRCOMMON_LOGGER_ENDL;

			while (_running)
			{
				try {
					// Receive full frame from socket
					len = _service->recvfrom(&data[0], maxlen, type, flags, seqno, address);

					// traffic monitoring
					addStats("in", len);
				} catch (const DatagramException&) {
					_running = false;
					break;
				}

				IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "componentRun() Address: " << address << IBRCOMMON_LOGGER_ENDL;

				// Check for extended header and retrieve if available
				if (type == HEADER_BROADCAST)
				{
					try {
						IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "componentRun() Announcement received" << IBRCOMMON_LOGGER_ENDL;
						DiscoveryAnnouncement announce;
						stringstream ss;
						ss.write(&data[0], len);
						ss >> announce;

						// ignore own beacons
						if (announce.getEID() == dtn::core::BundleCore::local) continue;

						// convert the announcement into NodeEvents
						Node n(announce.getEID());

						// timeout value
						size_t to_value = 30;

						// add
						n.add(Node::URI(Node::NODE_DISCOVERED, _service->getProtocol(), address, to_value, 20));

						const std::list<DiscoveryService> services = announce.getServices();
						for (std::list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); ++iter)
						{
							const DiscoveryService &s = (*iter);
							n.add(Node::Attribute(Node::NODE_DISCOVERED, s.getName(), s.getParameters(), to_value, 20));
						}

						{
							// lock the connection list while working with it
							ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);

							// Connection instance for this address
							DatagramConnection& connection = getConnection(address);
							connection.setPeerEID(announce.getEID());
						}

						// announce NodeInfo to ConnectionManager
						dtn::core::BundleCore::getInstance().getConnectionManager().updateNeighbor(n);
					} catch (const ibrcommon::Exception&) {
						// catch wrong formats
					}

					continue;
				}
				else if ( type == HEADER_SEGMENT )
				{
					ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);

					// Connection instance for this address
					DatagramConnection& connection = getConnection(address);

					try {
						// Decide in which queue to write based on the src address
						connection.queue(flags, seqno, &data[0], len);
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_TAG(DatagramConvergenceLayer::TAG, error) << ex.what() << IBRCOMMON_LOGGER_ENDL;
						connection.shutdown();
					};
				}
				else if ( type == HEADER_ACK )
				{
					ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);

					// Connection instance for this address
					DatagramConnection& connection = getConnection(address);

					// Decide in which queue to write based on the src address
					connection.ack(seqno);
				}

				yield();
			}
		}

		void DatagramConvergenceLayer::raiseEvent(const Event *evt) throw ()
		{
			try {
				const TimeEvent &time=dynamic_cast<const TimeEvent&>(*evt);
				if (time.getAction() == TIME_SECOND_TICK)
					if (time.getTimestamp().get<size_t>() % 5 == 0)
						sendAnnoucement();
			} catch (const std::bad_cast&)
			{}
		}

		void DatagramConvergenceLayer::__cancellation() throw ()
		{
			_running = false;
			_service->shutdown();
		}

		const std::string DatagramConvergenceLayer::getName() const
		{
			return DatagramConvergenceLayer::TAG;
		}
	} /* namespace data */
} /* namespace dtn */
