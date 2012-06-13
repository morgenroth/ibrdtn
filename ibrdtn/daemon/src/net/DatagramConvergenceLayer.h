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
#include "net/DatagramConnection.h"
#include "net/ConvergenceLayer.h"
#include "net/DiscoveryAgent.h"
#include "net/DiscoveryServiceProvider.h"
#include "net/DatagramConnectionParameter.h"
#include "core/Node.h"
#include <ibrcommon/net/vinterface.h>

#include <list>

namespace dtn
{
	namespace net
	{
		class DatagramService
		{
		public:
			enum FLOWCONTROL
			{
				FLOW_NONE = 0,
				FLOW_STOPNWAIT = 1
			};

			virtual ~DatagramService() {};

			/**
			 * Bind to the local socket.
			 * @throw If the bind fails, an DatagramException is thrown.
			 */
			virtual void bind() throw (DatagramException) = 0;

			/**
			 * Shutdown the socket. Unblock all calls on the socket (recv, send, etc.)
			 */
			virtual void shutdown() = 0;

			/**
			 * Send the payload as datagram to a defined destination
			 * @param address The destination address encoded as string.
			 * @param buf The buffer to send.
			 * @param length The number of available bytes in the buffer.
			 * @throw If the transmission wasn't successful this method will throw an exception.
			 */
			virtual void send(const char &type, const char &flags, const unsigned int &seqno, const std::string &address, const char *buf, size_t length) throw (DatagramException) = 0;

			/**
			 * Send the payload as datagram to all neighbors (broadcast)
			 * @param buf The buffer to send.
			 * @param length The number of available bytes in the buffer.
			 * @throw If the transmission wasn't successful this method will throw an exception.
			 */
			virtual void send(const char &type, const char &flags, const unsigned int &seqno, const char *buf, size_t length) throw (DatagramException) = 0;

			/**
			 * Receive an incoming datagram.
			 * @param buf A buffer to catch the incoming data.
			 * @param length The length of the buffer.
			 * @param address A buffer for the address of the sender.
			 * @throw If the receive call failed for any reason, an DatagramException is thrown.
			 * @return The number of received bytes.
			 */
			virtual size_t recvfrom(char *buf, size_t length, char &type, char &flags, unsigned int &seqno, std::string &address) throw (DatagramException) = 0;

			/**
			 * Get the tag for this service used in discovery messages.
			 * @return The tag as string.
			 */
			virtual const std::string getServiceTag() const = 0;

			/**
			 * Get the service description for this convergence layer. This
			 * data is used to contact this node.
			 * @return The service description as string.
			 */
			virtual const std::string getServiceDescription() const = 0;

			/**
			 * The used interface as vinterface object.
			 * @return A vinterface object.
			 */
			virtual const ibrcommon::vinterface& getInterface() const = 0;

			/**
			 * The protocol identifier for this type of service.
			 * @return
			 */
			virtual dtn::core::Node::Protocol getProtocol() const = 0;

			/**
			 * Returns the parameter for the connection.
			 * @return
			 */
			virtual const DatagramConnectionParameter& getParameter() const = 0;
		};

		class DatagramConvergenceLayer : public ConvergenceLayer, public dtn::daemon::IndependentComponent,
			public EventReceiver, public DatagramConnectionCallback
		{
		public:
			enum HEADER_FLAGS
			{
				HEADER_UNKOWN = 0,
				HEADER_BROADCAST = 1,
				HEADER_SEGMENT = 2,
				HEADER_ACK = 4
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
			virtual void componentUp();
			virtual void componentRun();
			virtual void componentDown();
			void __cancellation();

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
