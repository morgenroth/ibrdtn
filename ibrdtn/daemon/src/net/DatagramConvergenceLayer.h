/*
 * DatagramConvergenceLayer.h
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

#ifndef DATAGRAMCONVERGENCELAYER_H_
#define DATAGRAMCONVERGENCELAYER_H_

#include "Component.h"
#include "net/DatagramService.h"
#include "net/DatagramConnection.h"
#include "net/ConvergenceLayer.h"
#include "net/DiscoveryAgent.h"
#include "net/DiscoveryServiceProvider.h"

#include <list>

namespace dtn
{
	namespace net
	{
		class DatagramConvergenceLayer : public ConvergenceLayer, public dtn::daemon::IndependentComponent,
			public EventReceiver, public DatagramConnectionCallback
		{
		public:
			enum HEADER_FLAGS
			{
				HEADER_UNKOWN = 0,
				HEADER_BROADCAST = 1,
				HEADER_SEGMENT = 2,
				HEADER_ACK = 4,
				HEADER_NACK = 8
			};

			DatagramConvergenceLayer(DatagramService *ds);
			virtual ~DatagramConvergenceLayer();

			/**
			 * Returns the protocol identifier
			 * @return
			 */
			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			/**
			 * Queueing a job for a specific node. Starting point for the DTN core to submit
			 * bundles to nodes behind the convergence layer
			 * @param n Node reference
			 * @param job Job reference
			 */
			void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			/**
			 * Public method for event callbacks
			 * @param evt
			 */
			virtual void raiseEvent(const dtn::core::Event *evt);

		protected:
			virtual void componentUp() throw ();
			virtual void componentRun() throw ();;
			virtual void componentDown() throw ();
			void __cancellation() throw ();

			void sendAnnoucement();

			/**
			 * callback send for connections
			 * @param connection
			 * @param destination
			 * @param buf
			 * @param len
			 */
			void callback_send(DatagramConnection &connection, const char &flags, const unsigned int &seqno, const std::string &destination, const char *buf, int len) throw (DatagramException);

			void callback_ack(DatagramConnection &connection, const unsigned int &seqno, const std::string &destination) throw (DatagramException);

			void connectionUp(const DatagramConnection *conn);
			void connectionDown(const DatagramConnection *conn);

		private:
			DatagramConnection& getConnection(const std::string &identifier);

			DatagramService *_service;

			ibrcommon::Mutex _send_lock;

			ibrcommon::Conditional _connection_cond;
			std::list<DatagramConnection*> _connections;

			bool _running;

			u_int16_t _discovery_sn;
		};
	} /* namespace data */
} /* namespace dtn */
#endif /* DATAGRAMCONVERGENCELAYER_H_ */
