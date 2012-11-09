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
#include "core/TimeEvent.h"
#include "core/NodeEvent.h"

#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/MutexLock.h>

#include <string.h>

namespace dtn
{
	namespace net
	{
		DatagramConvergenceLayer::DatagramConvergenceLayer(DatagramService *ds)
		 : _service(ds), _running(false), _discovery_sn(0)
		{
		}

		DatagramConvergenceLayer::~DatagramConvergenceLayer()
		{
			// wait until all connections are down
			{
				ibrcommon::MutexLock l(_connection_cond);
				while (_connections.size() != 0) _connection_cond.wait();
			}

			join();
			delete _service;
		}

		dtn::core::Node::Protocol DatagramConvergenceLayer::getDiscoveryProtocol() const
		{
			return _service->getProtocol();
		}

		void DatagramConvergenceLayer::callback_send(DatagramConnection&, const char &flags, const unsigned int &seqno, const std::string &destination, const char *buf, int len) throw (DatagramException)
		{
			// only on sender at once
			ibrcommon::MutexLock l(_send_lock);

			// forward the send request to DatagramService
			_service->send(HEADER_SEGMENT, flags, seqno, destination, buf, len);
		}

		void DatagramConvergenceLayer::callback_ack(DatagramConnection&, const unsigned int &seqno, const std::string &destination) throw (DatagramException)
		{
			// only on sender at once
			ibrcommon::MutexLock l(_send_lock);

			// forward the send request to DatagramService
			_service->send(HEADER_ACK, 0, seqno, destination, NULL, 0);
		}

		void DatagramConvergenceLayer::queue(const dtn::core::Node &node, const ConvergenceLayer::Job &job)
		{
			const std::list<dtn::core::Node::URI> uri_list = node.get(_service->getProtocol());
			if (uri_list.empty()) return;

			// get the first element of the result
			const dtn::core::Node::URI &uri = uri_list.front();

			// some debugging
			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConvergenceLayer::queue"<< IBRCOMMON_LOGGER_ENDL;

			// lock the connection list while working with it
			ibrcommon::MutexLock lc(_connection_cond);

			// get a new or the existing connection for this address
			DatagramConnection &conn = getConnection( uri.value );

			// queue the job to the connection
			conn.queue(job);
		}

		DatagramConnection& DatagramConvergenceLayer::getConnection(const std::string &identifier)
		{
			// Test if connection for this address already exist
			for(std::list<DatagramConnection*>::iterator i = _connections.begin(); i != _connections.end(); ++i)
			{
				IBRCOMMON_LOGGER_DEBUG(10) << "Connection identifier: " << (*i)->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
				if ((*i)->getIdentifier() == identifier)
					return *(*i);
			}

			// Connection does not exist, create one and put it into the list
			DatagramConnection *connection = new DatagramConnection(identifier, _service->getParameter(), (*this));

			_connections.push_back(connection);
			_connection_cond.signal(true);

			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConvergenceLayer::getConnection "<< connection->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
			connection->start();
			return *connection;
		}

		void DatagramConvergenceLayer::connectionUp(const DatagramConnection *conn)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConvergenceLayer::connectionUp: " << conn->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
		}

		void DatagramConvergenceLayer::connectionDown(const DatagramConnection *conn)
		{
			ibrcommon::MutexLock lc(_connection_cond);

			std::list<DatagramConnection*>::iterator i;
			for(i = _connections.begin(); i != _connections.end(); ++i)
			{
				if ((*i) == conn)
				{
					IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConvergenceLayer::connectionDown: " << conn->getIdentifier() << IBRCOMMON_LOGGER_ENDL;

					_connections.erase(i);
					_connection_cond.signal(true);
					return;
				}
			}

			IBRCOMMON_LOGGER(error) << "DatagramConvergenceLayer::connectionDown: " << conn->getIdentifier() << " not found!" << IBRCOMMON_LOGGER_ENDL;
		}

		void DatagramConvergenceLayer::componentUp() throw ()
		{
			bindEvent(dtn::core::TimeEvent::className);
			try {
				_service->bind();
			} catch (const std::exception &e) {
				IBRCOMMON_LOGGER_DEBUG(10) << "Failed to add DatagramConvergenceLayer on " << _service->getInterface().toString() << IBRCOMMON_LOGGER_ENDL;
				IBRCOMMON_LOGGER_DEBUG(10) << "Exception: " << e.what() << IBRCOMMON_LOGGER_ENDL;
			}

			// set the running variable
			_running = true;
		}

		void DatagramConvergenceLayer::componentDown() throw ()
		{
			unbindEvent(dtn::core::TimeEvent::className);

			// shutdown all connections
			{
				ibrcommon::MutexLock l(_connection_cond);
				for(std::list<DatagramConnection*>::iterator i = _connections.begin(); i != _connections.end(); ++i)
				{
					(*i)->shutdown();
				}
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

			int len = ss.str().size();

			try {
				// only on sender at once
				ibrcommon::MutexLock l(_send_lock);

				// forward the send request to DatagramService
				_service->send(HEADER_BROADCAST, 0, 0, ss.str().c_str(), len);
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
			char data[maxlen];
			size_t len = 0;

			while (_running)
			{
				IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConvergenceLayer::componentRun early" << IBRCOMMON_LOGGER_ENDL;

				try {
					// Receive full frame from socket
					len = _service->recvfrom(data, maxlen, type, flags, seqno, address);
				} catch (const DatagramException&) {
					_running = false;
					break;
				}

				IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConvergenceLayer::componentRun" << IBRCOMMON_LOGGER_ENDL;
				IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConvergenceLayer::componentRun: ADDRESS " << address << IBRCOMMON_LOGGER_ENDL;

				// Check for extended header and retrieve if available
				if (type == HEADER_BROADCAST)
				{
					IBRCOMMON_LOGGER(notice) << "Received announcement for DatagramConvergenceLayer discovery" << IBRCOMMON_LOGGER_ENDL;
					DiscoveryAnnouncement announce;
					stringstream ss;
					ss.write(data, len);
					ss >> announce;

					// convert the announcement into NodeEvents
					Node n(announce.getEID());

					// timeout value
					size_t to_value = 30;

					// add
					n.add(Node::URI(Node::NODE_DISCOVERED, _service->getProtocol(), address, to_value, 20));

					const std::list<DiscoveryService> services = announce.getServices();
					for (std::list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); iter++)
					{
						const DiscoveryService &s = (*iter);
						n.add(Node::Attribute(Node::NODE_DISCOVERED, s.getName(), s.getParameters(), to_value, 20));
					}

					// announce NodeInfo to ConnectionManager
					dtn::core::BundleCore::getInstance().getConnectionManager().updateNeighbor(n);

					continue;
				}
				else if ( type == HEADER_SEGMENT )
				{
					ibrcommon::MutexLock lc(_connection_cond);

					// Connection instance for this address
					DatagramConnection& connection = getConnection(address);

					try {
						// Decide in which queue to write based on the src address
						connection.queue(flags, seqno, data, len);
					} catch (const ibrcommon::Exception&) { };
				}
				else if ( type == HEADER_ACK )
				{
					ibrcommon::MutexLock lc(_connection_cond);

					// Connection instance for this address
					DatagramConnection& connection = getConnection(address);

					// Decide in which queue to write based on the src address
					connection.ack(seqno);
				}

				yield();
			}
		}

		void DatagramConvergenceLayer::raiseEvent(const Event *evt)
		{
			try {
				const TimeEvent &time=dynamic_cast<const TimeEvent&>(*evt);
				if (time.getAction() == TIME_SECOND_TICK)
					if (time.getTimestamp() % 5 == 0)
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
			return "DatagramConvergenceLayer";
		}
	} /* namespace data */
} /* namespace dtn */
