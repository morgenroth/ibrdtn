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
		 : _service(ds), _active_conns(0), _running(false),
		   _stats_in(0), _stats_out(0), _stats_rtt(0.0), _stats_retries(0), _stats_failure(0)
		{
		}

		DatagramConvergenceLayer::~DatagramConvergenceLayer()
		{
			// wait until the component thread is terminated
			join();

			// wait until all connections are down
			{
				ibrcommon::MutexLock l(_cond_connections);
				while (_active_conns != 0) _cond_connections.wait();
			}

			// delete the associated service
			delete _service;
		}

		void DatagramConvergenceLayer::resetStats()
		{
			_stats_in = 0;
			_stats_out = 0;
			_stats_rtt = 0.0;
			_stats_retries = 0;
			_stats_failure = 0;
		}

		void DatagramConvergenceLayer::getStats(ConvergenceLayer::stats_data &data) const
		{
			std::stringstream ss_format;

			static const std::string IN_TAG = dtn::core::Node::toString(getDiscoveryProtocol()) + "|in";
			static const std::string OUT_TAG = dtn::core::Node::toString(getDiscoveryProtocol()) + "|out";

			static const std::string RTT_TAG = dtn::core::Node::toString(getDiscoveryProtocol()) + "|rtt";
			static const std::string RETRIES_TAG = dtn::core::Node::toString(getDiscoveryProtocol()) + "|retries";
			static const std::string FAIL_TAG = dtn::core::Node::toString(getDiscoveryProtocol()) + "|fail";

			ss_format << _stats_in;
			data[IN_TAG] = ss_format.str();
			ss_format.str("");

			ss_format << _stats_out;
			data[OUT_TAG] = ss_format.str();
			ss_format.str("");

			ss_format << _stats_rtt;
			data[RTT_TAG] = ss_format.str();
			ss_format.str("");

			ss_format << _stats_retries;
			data[RETRIES_TAG] = ss_format.str();
			ss_format.str("");

			ss_format << _stats_failure;
			data[FAIL_TAG] = ss_format.str();
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
			_stats_out += len;
		}

		void DatagramConvergenceLayer::callback_ack(DatagramConnection&, const unsigned int &seqno, const std::string &destination) throw (DatagramException)
		{
			// only on sender at once
			ibrcommon::MutexLock l(_send_lock);

			// forward the send request to DatagramService
			_service->send(HEADER_ACK, 0, seqno, destination, NULL, 0);
		}

		void DatagramConvergenceLayer::callback_nack(DatagramConnection&, const unsigned int &seqno, const std::string &destination) throw (DatagramException)
		{
			// only on sender at once
			ibrcommon::MutexLock l(_send_lock);

			// forward the send request to DatagramService
			_service->send(HEADER_NACK, 0, seqno, destination, NULL, 0);
		}

		void DatagramConvergenceLayer::queue(const dtn::core::Node &node, const dtn::net::BundleTransfer &job)
		{
			// do not queue any new jobs if the convergence layer goes down
			if (!_running) return;

			const std::list<dtn::core::Node::URI> uri_list = node.get(_service->getProtocol());
			if (uri_list.empty()) return;

			// get the first element of the result
			const dtn::core::Node::URI &uri = uri_list.front();

			// some debugging
			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "job queued for " << node.getEID().getString() << IBRCOMMON_LOGGER_ENDL;

			// lock the connection list while working with it
			ibrcommon::RWLock lc(_mutex_connection, ibrcommon::RWMutex::LOCK_READWRITE);

			// get a new or the existing connection for this address
			DatagramConnection &conn = getConnection( uri.value, true );

			// queue the job to the connection
			conn.queue(job);
		}

		DatagramConnection& DatagramConvergenceLayer::getConnection(const std::string &identifier, bool create) throw (ConnectionNotAvailableException)
		{
			DatagramConnection *connection = NULL;

			// Test if connection for this address already exist
			for(connection_list::const_iterator i = _connections.begin(); i != _connections.end(); ++i)
			{
				if ((*i)->getIdentifier() == identifier)
					return *(*i);
			}

			// throw exception if we should not create new connections
			if (!create) throw ConnectionNotAvailableException();

			// Connection does not exist, create one and put it into the list
			connection = new DatagramConnection(identifier, _service->getParameter(), (*this));

			// add a new connection to the list of connections
			_connections.push_back(connection);

			// increment the number of active connections
			{
				ibrcommon::MutexLock l(_cond_connections);
				++_active_conns;
				_cond_connections.signal(true);
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "Selected identifier: " << connection->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
			connection->start();
			return *connection;
		}

		void DatagramConvergenceLayer::reportSuccess(size_t retries, double rtt)
		{
			_stats_rtt = rtt;
			_stats_retries += retries;
		}

		void DatagramConvergenceLayer::reportFailure()
		{
			_stats_failure++;
		}

		void DatagramConvergenceLayer::connectionUp(const DatagramConnection *conn)
		{
			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "Up: " << conn->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
		}

		void DatagramConvergenceLayer::connectionDown(const DatagramConnection *conn)
		{
			ibrcommon::MutexLock l(_cond_connections);
			ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READWRITE);

			const connection_list::iterator i = std::find(_connections.begin(), _connections.end(), conn);

			if (i != _connections.end())
			{
				IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "Down: " << conn->getIdentifier() << IBRCOMMON_LOGGER_ENDL;
				_connections.erase(i);

				// decrement the number of connections
				--_active_conns;
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
			try {
				_service->bind();
			} catch (const std::exception &e) {
				IBRCOMMON_LOGGER_TAG(DatagramConvergenceLayer::TAG, error) << "bind to " << _service->getInterface().toString() << " failed (" << e.what() << ")" << IBRCOMMON_LOGGER_ENDL;
			}

			// register for discovery beacon handling
			dtn::core::BundleCore::getInstance().getDiscoveryAgent().registerService(_service->getInterface(), this);

			// set the running variable
			_running = true;
		}

		void DatagramConvergenceLayer::componentDown() throw ()
		{
			// un-register for discovery beacon handling
			dtn::core::BundleCore::getInstance().getDiscoveryAgent().unregisterService(_service->getInterface(), this);

			// shutdown all connections
			ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);
			for(connection_list::const_iterator i = _connections.begin(); i != _connections.end(); ++i)
			{
				(*i)->shutdown();
			}
		}

		void DatagramConvergenceLayer::onAdvertiseBeacon(const ibrcommon::vinterface &iface, const DiscoveryBeacon &beacon) throw ()
		{
			// only handler beacons for this interface
			if (iface != _service->getInterface()) return;

			// serialize announcement
			stringstream ss;
			ss << beacon;

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

			// get the reference to the discovery agent
			dtn::net::DiscoveryAgent &agent = dtn::core::BundleCore::getInstance().getDiscoveryAgent();

			IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "componentRun() entered" << IBRCOMMON_LOGGER_ENDL;

			while (_running)
			{
				try {
					// Receive full frame from socket
					len = _service->recvfrom(&data[0], maxlen, type, flags, seqno, address);

					// traffic monitoring
					_stats_in += len;
				} catch (const DatagramException &ex) {
					IBRCOMMON_LOGGER_TAG(DatagramConvergenceLayer::TAG, error) << "recvfrom() failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					_running = false;
					break;
				}

				IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "componentRun() Address: " << address << IBRCOMMON_LOGGER_ENDL;

				// Check for extended header and retrieve if available
				if (type == HEADER_BROADCAST)
				{
					try {
						IBRCOMMON_LOGGER_DEBUG_TAG(DatagramConvergenceLayer::TAG, 10) << "componentRun() Announcement received" << IBRCOMMON_LOGGER_ENDL;

						DiscoveryBeacon beacon = agent.obtainBeacon();

						stringstream ss;
						ss.write(&data[0], len);
						ss >> beacon;

						// ignore own beacons
						if (beacon.getEID() == dtn::core::BundleCore::local) continue;

						if (beacon.isShort())
						{
							// can not generate node name from short beacons
							continue;
						}

						// add discovered service entry
						beacon.addService(dtn::net::DiscoveryService(_service->getProtocol(), address));

						{
							// lock the connection list while working with it
							ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READWRITE);

							// Connection instance for this address
							DatagramConnection& connection = getConnection(address, true);
							connection.setPeerEID(beacon.getEID());
						}

						// announce the received beacon
						agent.onBeaconReceived(beacon);
					} catch (const ibrcommon::Exception&) {
						// catch wrong formats
					}

					continue;
				}
				else if ( type == HEADER_SEGMENT )
				{
					ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READWRITE);

					// Connection instance for this address
					DatagramConnection& connection = getConnection(address, true);

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
					try {
						ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);

						// Connection instance for this address
						DatagramConnection& connection = getConnection(address, false);

						IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 20) << "ack received for seqno " << seqno << IBRCOMMON_LOGGER_ENDL;

						// Decide in which queue to write based on the src address
						connection.ack(seqno);
					} catch (const ConnectionNotAvailableException &ex) {
						// connection does not exists - ignore the ACK
					}
				}
				else if ( type == HEADER_NACK )
				{
					// the peer refused the current bundle
					try {
						ibrcommon::RWLock rwl(_mutex_connection, ibrcommon::RWMutex::LOCK_READONLY);

						// Connection instance for this address
						DatagramConnection& connection = getConnection(address, false);

						IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 20) << "nack received for seqno " << seqno << IBRCOMMON_LOGGER_ENDL;

						// Decide in which queue to write based on the src address
						connection.nack(seqno, flags & DatagramService::NACK_TEMPORARY);
					} catch (const ConnectionNotAvailableException &ex) {
						// connection does not exists - ignore the NACK
					}
				}

				yield();
			}
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
