/*
 * TCPConvergenceLayer.cpp
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
#include "net/TCPConvergenceLayer.h"
#include "net/ConnectionEvent.h"
#include "routing/RequeueBundleEvent.h"
#include "core/BundleCore.h"

#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/net/socket.h>
#include <ibrcommon/Logger.h>

#include <streambuf>
#include <functional>
#include <list>
#include <algorithm>

#ifdef WITH_TLS
#include <ibrcommon/ssl/TLSStream.h>
#endif

namespace dtn
{
	namespace net
	{
		/*
		 * class TCPConvergenceLayer
		 */
		const int TCPConvergenceLayer::DEFAULT_PORT = 4556;

		TCPConvergenceLayer::TCPConvergenceLayer()
		 : _any_port(0)
		{
		}

		TCPConvergenceLayer::~TCPConvergenceLayer()
		{
			join();
		}

		void TCPConvergenceLayer::bind(const ibrcommon::vinterface &net, int port)
		{
			// do not allow any futher binding if we already bound to any interface
			if (_any_port > 0) return;

			if (net.empty()) {
				// bind to any interface
				_vsocket.add(new ibrcommon::tcpserversocket(port));
				_any_port = port;
			} else {
				// bind on all addresses on this interface
				_interfaces.push_back(net);

				// create sockets for all addresses on the interface
				std::list<ibrcommon::vaddress> addrs = net.getAddresses();

				// convert the port into a string
				std::stringstream ss; ss << port;

				for (std::list<ibrcommon::vaddress>::iterator iter = addrs.begin(); iter != addrs.end(); iter++) {
					ibrcommon::vaddress &addr = (*iter);
					addr.setService(ss.str());
					_vsocket.add(new ibrcommon::tcpserversocket(addr), net);
				}

				//_tcpsrv.bind(net, port);
				_portmap[net] = port;
			}
		}

		dtn::core::Node::Protocol TCPConvergenceLayer::getDiscoveryProtocol() const
		{
			return dtn::core::Node::CONN_TCPIP;
		}

		void TCPConvergenceLayer::update(const ibrcommon::vinterface &iface, DiscoveryAnnouncement &announcement) throw(dtn::net::DiscoveryServiceProvider::NoServiceHereException)
		{
			// announce port only if we are bound to any interface
			if (_interfaces.empty() && (_any_port > 0)) {
				std::stringstream service;
				// ... set the port only
				service << "port=" << _portmap[iface] << ";";
				announcement.addService( DiscoveryService("tcpcl", service.str()));
				return;
			}

			// determine if we should enable crosslayer discovery by sending out our own address
			bool crosslayer = dtn::daemon::Configuration::getInstance().getDiscovery().enableCrosslayer();

			// this marker should set to true if we added an service description
			bool announced = false;

			// search for the matching interface
			for (std::list<ibrcommon::vinterface>::const_iterator it = _interfaces.begin(); it != _interfaces.end(); it++)
			{
				const ibrcommon::vinterface &interface = *it;
				if (interface == iface)
				{
					try {
						// check if cross layer discovery is disabled
						if (!crosslayer) throw ibrcommon::Exception("crosslayer discovery disabled!");

						// get all addresses of this interface
						std::list<ibrcommon::vaddress> list = interface.getAddresses();

						// if no address is returned... (goto catch block)
						if (list.empty()) throw ibrcommon::Exception("no address found");

						for (std::list<ibrcommon::vaddress>::const_iterator addr_it = list.begin(); addr_it != list.end(); addr_it++)
						{
							if ((*addr_it).scope() != ibrcommon::vaddress::SCOPE_GLOBAL) continue;

							std::stringstream service;
							// fill in the ip address
							service << "ip=" << (*addr_it).address() << ";port=" << _portmap[iface] << ";";
							announcement.addService( DiscoveryService("tcpcl", service.str()));

							// set the announce mark
							announced = true;
						}
					} catch (const ibrcommon::Exception&) {
						// address collection process aborted
					};

					// if we still not announced anything...
					if (!announced) {
						std::stringstream service;
						// ... set the port only
						service << "port=" << _portmap[iface] << ";";
						announcement.addService( DiscoveryService("tcpcl", service.str()));
					}
					return;
				}
			}

			throw dtn::net::DiscoveryServiceProvider::NoServiceHereException();
		}

		const std::string TCPConvergenceLayer::getName() const
		{
			return "TCPConvergenceLayer";
		}

		void TCPConvergenceLayer::open(const dtn::core::Node &n)
		{
			// search for an existing connection
			ibrcommon::MutexLock l(_connections_cond);

			for (std::list<TCPConnection*>::iterator iter = _connections.begin(); iter != _connections.end(); iter++)
			{
				TCPConnection &conn = *(*iter);

				if (conn.match(n))
				{
					return;
				}
			}

			try {
				// create a connection
				TCPConnection *conn = new TCPConnection(*this, n, NULL, 10);

#ifdef WITH_TLS
				// enable TLS Support
				if ( ibrcommon::TLSStream::isInitialized() )
				{
					conn->enableTLS();
				}
#endif

				// raise setup event
				ConnectionEvent::raise(ConnectionEvent::CONNECTION_SETUP, n);

				// add connection as pending
				_connections.push_back( conn );

				// start the ClientHandler (service)
				conn->initialize();

				// signal that there is a new connection
				_connections_cond.signal(true);
			} catch (const ibrcommon::Exception&) {	};

			return;
		}

		void TCPConvergenceLayer::queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job)
		{
			// search for an existing connection
			ibrcommon::MutexLock l(_connections_cond);

			for (std::list<TCPConnection*>::iterator iter = _connections.begin(); iter != _connections.end(); iter++)
			{
				TCPConnection &conn = *(*iter);

				if (conn.match(n))
				{
					conn.queue(job._bundle);
					IBRCOMMON_LOGGER_DEBUG(15) << "queued bundle to an existing tcp connection (" << conn.getNode().toString() << ")" << IBRCOMMON_LOGGER_ENDL;

					return;
				}
			}

			try {
				// create a connection
				TCPConnection *conn = new TCPConnection(*this, n, NULL, 10);

#ifdef WITH_TLS
				// enable TLS Support
				if ( ibrcommon::TLSStream::isInitialized() )
				{
					conn->enableTLS();
				}
#endif

				// raise setup event
				ConnectionEvent::raise(ConnectionEvent::CONNECTION_SETUP, n);

				// add connection as pending
				_connections.push_back( conn );

				// start the ClientHandler (service)
				conn->initialize();

				// queue the bundle
				conn->queue(job._bundle);

				// signal that there is a new connection
				_connections_cond.signal(true);

				IBRCOMMON_LOGGER_DEBUG(15) << "queued bundle to an new tcp connection (" << conn->getNode().toString() << ")" << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::Exception&) {
				// raise transfer abort event for all bundles without an ACK
				dtn::routing::RequeueBundleEvent::raise(n.getEID(), job._bundle);
			}
		}

		void TCPConvergenceLayer::connectionUp(TCPConnection *conn)
		{
			ibrcommon::MutexLock l(_connections_cond);
			for (std::list<TCPConnection*>::iterator iter = _connections.begin(); iter != _connections.end(); iter++)
			{
				if (conn == (*iter))
				{
					// put pending connection to the active connections
					return;
				}
			}

			_connections.push_back( conn );

			// signal that there is a new connection
			_connections_cond.signal(true);

			IBRCOMMON_LOGGER_DEBUG(15) << "tcp connection added (" << conn->getNode().toString() << ")" << IBRCOMMON_LOGGER_ENDL;
		}

		void TCPConvergenceLayer::connectionDown(TCPConnection *conn)
		{
			ibrcommon::MutexLock l(_connections_cond);
			for (std::list<TCPConnection*>::iterator iter = _connections.begin(); iter != _connections.end(); iter++)
			{
				if (conn == (*iter))
				{
					_connections.erase(iter);
					IBRCOMMON_LOGGER_DEBUG(15) << "tcp connection removed (" << conn->getNode().toString() << ")" << IBRCOMMON_LOGGER_ENDL;

					// signal that there is a connection less
					_connections_cond.signal(true);
					return;
				}
			}
		}

		void TCPConvergenceLayer::componentRun()
		{
			try {
				while (true)
				{
					ibrcommon::socketset readfds;
					
					// wait for incoming connections
					_vsocket.select(&readfds, NULL, NULL, NULL);

					for (ibrcommon::socketset::iterator iter = readfds.begin(); iter != readfds.end(); iter++) {
						try {
							// assume that all sockets are serversockets
							ibrcommon::serversocket &sock = dynamic_cast<ibrcommon::serversocket&>(**iter);

							// wait for incoming connections
							ibrcommon::vaddress peeraddr;
							ibrcommon::clientsocket *client = sock.accept(peeraddr);

							// create a EID based on the peer address
							dtn::data::EID source("tcp://" + peeraddr.address() + ":" + peeraddr.service());

							// create a new node object
							dtn::core::Node node(source);

							// create a new TCPConnection and return the pointer
							TCPConnection *obj = new TCPConnection(*this, node, client, 10);

#ifdef WITH_TLS
							// enable TLS Support
							if ( ibrcommon::TLSStream::isInitialized() )
							{
								obj->enableTLS();
							}
#endif

							// add the connection to the connection list
							connectionUp(obj);

							// initialize the object
							obj->initialize();
						} catch (const std::bad_cast&) {

						}
					}

					// breakpoint
					ibrcommon::Thread::yield();
				}
			} catch (const std::exception&) {
				// ignore all errors
				return;
			}
		}

		void TCPConvergenceLayer::__cancellation()
		{
			_vsocket.down();
		}

		void TCPConvergenceLayer::closeAll()
		{
			// search for an existing connection
			ibrcommon::MutexLock l(_connections_cond);
			for (std::list<TCPConnection*>::iterator iter = _connections.begin(); iter != _connections.end(); iter++)
			{
				TCPConnection &conn = *(*iter);

				// close the connection immediately
				conn.shutdown();
			}
		}

		void TCPConvergenceLayer::componentUp()
		{
			// listen on the socket, max. 5 concurrent awaiting connections
			_vsocket.up();
		}

		void TCPConvergenceLayer::componentDown()
		{
			// shutdown all sockets
			_vsocket.down();

			// close all active connections
			closeAll();

			// wait until all tcp connections are down
			{
				ibrcommon::MutexLock l(_connections_cond);
				while (_connections.size() > 0) _connections_cond.wait();
			}
		}
	}
}
