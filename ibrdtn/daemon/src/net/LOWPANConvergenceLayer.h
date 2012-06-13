/*
 * LOWPANConnection.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef LOWPANCONVERGENCELAYER_H_
#define LOWPANCONVERGENCELAYER_H_

#include "Component.h"
#include "net/ConvergenceLayer.h"
#include "net/LOWPANConnection.h"
#include "net/DiscoveryAgent.h"
#include "net/DiscoveryServiceProvider.h"
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/net/lowpansocket.h>
#include <ibrcommon/net/lowpanstream.h>

#include <list>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		class LOWPANConnection;
		/**
		 * This class implements a ConvergenceLayer for LOWPAN.
		 */
		class LOWPANConvergenceLayer : public DiscoveryAgent, public ConvergenceLayer, public dtn::daemon::IndependentComponent, public ibrcommon::lowpanstream_callback, public EventReceiver, public DiscoveryServiceProvider
		{
		public:
			LOWPANConvergenceLayer(ibrcommon::vinterface net, int panid, unsigned int mtu = 115); //MTU is actually 127...

			virtual ~LOWPANConvergenceLayer();

			/**
			 * this method updates the given values
			 */
			void update(const ibrcommon::vinterface &iface, std::string &name, std::string &data) throw(dtn::net::DiscoveryServiceProvider::NoServiceHereException);

			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			/**
			 * Queueing a job for a specific node. Starting point for the DTN core to submit bundles to nodes behind the LoWPAN CL
			 * @param n Node reference
			 * @param job Job reference
			 */
			void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			virtual void raiseEvent(const Event *evt);

			/**
			 * Callback interface for sending data back from the lowpanstream to the CL
			 * @param buf Buffer with a data frame
			 * @param len Length of the buffer
			 * @param address IEEE 802.15.4 short address of the destination
			 */
			virtual void send_cb(char *buf, int len, unsigned int address);

			static const size_t BUFF_SIZE = 113;

			void remove(const LOWPANConnection *conn);

		protected:
			virtual void componentUp();
			virtual void componentRun();
			virtual void componentDown();
			void __cancellation();

			virtual void sendAnnoucement(const u_int16_t &sn, std::list<dtn::net::DiscoveryService> &services);

		private:
			ibrcommon::lowpansocket *_socket;

			ibrcommon::vinterface _net;
			int _panid;
			char *_ipnd_buf;
			int _ipnd_buf_len;

			ibrcommon::Mutex _connection_lock;
			std::list<LOWPANConnection*> ConnectionList;
			LOWPANConnection* getConnection(unsigned short address);

			unsigned int m_maxmsgsize;

			ibrcommon::Mutex m_writelock;
			ibrcommon::Mutex m_readlock;

			bool _running;
		};
	}
}
#endif /*LOWPANCONVERGENCELAYER_H_*/
