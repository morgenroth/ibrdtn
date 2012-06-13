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

#include "net/TCPConvergenceLayer.h"
#include "net/ConnectionEvent.h"
#include "routing/RequeueBundleEvent.h"
#include "core/BundleCore.h"

#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/net/tcpclient.h>
#include <ibrcommon/Logger.h>

#include <streambuf>
#include <functional>
#include <list>
#include <algorithm>

using namespace ibrcommon;

namespace dtn
{
	namespace net
	{
		/*
		 * class TCPConvergenceLayer
		 */
		const int TCPConvergenceLayer::DEFAULT_PORT = 4556;

		TCPConvergenceLayer::TCPConvergenceLayer()
		{
		}

		TCPConvergenceLayer::~TCPConvergenceLayer()
		{
			join();
		}

		void TCPConvergenceLayer::bind(const ibrcommon::vinterface &net, int port)
		{
			_interfaces.push_back(net);
			_tcpsrv.bind(net, port);
			_portmap[net] = port;
		}

		dtn::core::Node::Protocol TCPConvergenceLayer::getDiscoveryProtocol() const
		{
			return dtn::core::Node::CONN_TCPIP;
		}

		void TCPConvergenceLayer::update(const ibrcommon::vinterface &iface, std::string &name, std::string &params) throw(dtn::net::DiscoveryServiceProvider::NoServiceHereException)
		{
			name = "tcpcl";
			stringstream service;

			// TODO: get the main address of this host, if no interface is specified

			// search for the matching interface
			for (std::list<ibrcommon::vinterface>::const_iterator it = _interfaces.begin(); it != _interfaces.end(); it++)
			{
				const ibrcommon::vinterface &interface = *it;
				if (interface == iface)
				{
					try {
						// get all addresses of this interface
						std::list<vaddress> list = interface.getAddresses(ibrcommon::vaddress::VADDRESS_INET);

						// if no address is returned... (goto catch block)
						if (list.empty()) throw ibrcommon::Exception("no address found");

						// fill in the ip address
						service << "ip=" << list.front().get(false) << ";port=" << _portmap[iface] << ";";
					} catch (const ibrcommon::Exception&) {
						// ... set the port only
						service << "port=" << _portmap[iface] << ";";
					};

					params = service.str();
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
				TCPConnection *conn = new TCPConnection(*this, n, dtn::core::BundleCore::local, 10);

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
				TCPConnection *conn = new TCPConnection(*this, n, dtn::core::BundleCore::local, 10);

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
					// wait for incoming connections
					tcpstream *stream = _tcpsrv.accept();

					// create a new TCPConnection and return the pointer
					TCPConnection *obj = new TCPConnection(*this, stream, dtn::core::BundleCore::local, 10);

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
			_tcpsrv.shutdown();
			_tcpsrv.close();
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
			_tcpsrv.listen(5);
		}

		void TCPConvergenceLayer::componentDown()
		{
			// shutdown the TCP server
			_tcpsrv.shutdown();
			_tcpsrv.close();

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
