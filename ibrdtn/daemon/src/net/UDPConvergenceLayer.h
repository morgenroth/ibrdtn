/*
 * UDPConvergenceLayer.h
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

#ifndef UDPCONVERGENCELAYER_H_
#define UDPCONVERGENCELAYER_H_

#include "Component.h"
#include "net/ConvergenceLayer.h"
#include <ibrcommon/Exceptions.h>
#include "net/DiscoveryServiceProvider.h"
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/net/socket.h>
#include <ibrcommon/net/vsocket.h>
#include <ibrcommon/link/LinkManager.h>


namespace dtn
{
	namespace net
	{
		/**
		 * This class implement a ConvergenceLayer for UDP/IP.
		 * Each bundle is sent in exact one UDP datagram.
		 */
		class UDPConvergenceLayer : public ConvergenceLayer, public dtn::daemon::IndependentComponent, public DiscoveryServiceProvider, public ibrcommon::LinkManager::EventCallback
		{
		public:
			/**
			 * Constructor
			 * @param[in] bind_addr The address to bind.
			 * @param[in] port The udp port to use.
			 * @param[in] broadcast If true, the broadcast feature for this socket is enabled.
			 * @param[in] mtu The maximum bundle size.
			 */
			UDPConvergenceLayer(ibrcommon::vinterface net, int port, unsigned int mtu = 1280);

			/**
			 * Desktruktor
			 */
			virtual ~UDPConvergenceLayer();

			/**
			 * this method updates the given values
			 */
			void update(const ibrcommon::vinterface &iface, DiscoveryAnnouncement &announcement)
				throw (dtn::net::DiscoveryServiceProvider::NoServiceHereException);

			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			void eventNotify(const ibrcommon::LinkEvent &evt);

		protected:
			virtual void componentUp() throw ();
			virtual void componentRun() throw ();;
			virtual void componentDown() throw ();
			void __cancellation() throw ();

		private:
			void receive(dtn::data::Bundle&, dtn::data::EID &sender) throw (ibrcommon::socket_exception, dtn::InvalidDataException);
			void send(const ibrcommon::vaddress &addr, const std::string &data) throw (ibrcommon::socket_exception, NoAddressFoundException);

			ibrcommon::vsocket _vsocket;
			ibrcommon::vinterface _net;
			int _port;

			static const int DEFAULT_PORT;

			unsigned int m_maxmsgsize;

			ibrcommon::Mutex m_writelock;
			ibrcommon::Mutex m_readlock;

			bool _running;


		};
	}
}

#endif /*UDPCONVERGENCELAYER_H_*/
