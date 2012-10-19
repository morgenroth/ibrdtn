/*
 * TCPConvergenceLayer.h
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

#ifndef TCPCONVERGENCELAYER_H_
#define TCPCONVERGENCELAYER_H_

#include "config.h"
#include "Component.h"

#include "core/Node.h"
#include "net/ConvergenceLayer.h"
#include "net/TCPConnection.h"
#include "net/DiscoveryService.h"
#include "net/DiscoveryServiceProvider.h"

#include <ibrcommon/net/vsocket.h>
#include <ibrcommon/net/vinterface.h>

namespace dtn
{
	namespace net
	{
		/**
		 * This class implement a ConvergenceLayer for TCP/IP.
		 * http://tools.ietf.org/html/draft-irtf-dtnrg-tcp-clayer-02
		 */
		class TCPConvergenceLayer : public dtn::daemon::IndependentComponent, public ConvergenceLayer, public DiscoveryServiceProvider
		{
			friend class TCPConnection;
		public:
			/**
			 * Constructor
			 * @param[in] bind_addr The address to bind.
			 * @param[in] port The port to use.
			 */
			TCPConvergenceLayer();

			/**
			 * Destructor
			 */
			virtual ~TCPConvergenceLayer();

			/**
			 * Bind on a interface
			 * @param net
			 * @param port
			 */
			void bind(const ibrcommon::vinterface &net, int port);

			/**
			 * Queue a new transmission job for this convergence layer.
			 * @param job
			 */
			void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job);

			/**
			 * Open a connection to the given node.
			 * @param n
			 * @return
			 */
			void open(const dtn::core::Node &n);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			/**
			 * Returns the discovery protocol type
			 * @return
			 */
			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			/**
			 * this method updates the given values
			 */
			void update(const ibrcommon::vinterface &iface, DiscoveryAnnouncement &announcement)
				throw(dtn::net::DiscoveryServiceProvider::NoServiceHereException);

		protected:
			void __cancellation() throw ();

			void componentUp();
			void componentRun();
			void componentDown();

		private:
			/**
			 * closes all active tcp connections
			 */
			void closeAll();

			/**
			 * Add a connection to the connection list.
			 * @param conn
			 */
			void connectionUp(TCPConnection *conn);

			/**
			 * Removes a connection from the connection list. This method is called by the
			 * connection object itself to cleanup on a disconnect.
			 * @param conn
			 */
			void connectionDown(TCPConnection *conn);

			static const int DEFAULT_PORT;

			ibrcommon::vsocket _vsocket;

			ibrcommon::Conditional _connections_cond;
			std::list<TCPConnection*> _connections;
			std::list<ibrcommon::vinterface> _interfaces;
			std::map<ibrcommon::vinterface, unsigned int> _portmap;
			unsigned int _any_port;
		};
	}
}

#endif /* TCPCONVERGENCELAYER_H_ */
