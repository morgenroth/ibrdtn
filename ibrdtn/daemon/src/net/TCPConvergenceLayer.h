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

#include "core/NodeEvent.h"
#include "core/Node.h"
#include "net/ConvergenceLayer.h"
#include "net/DiscoveryService.h"
#include "net/DiscoveryServiceProvider.h"

#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/Serializer.h>
#include <ibrdtn/streams/StreamConnection.h>
#include <ibrdtn/streams/StreamContactHeader.h>

#include <ibrcommon/net/tcpserver.h>
#include <ibrcommon/net/tcpstream.h>
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/thread/Queue.h>
#include <memory>

#ifdef WITH_TLS
#include <ibrcommon/net/TLSStream.h>
#endif

using namespace dtn::streams;

namespace dtn
{
	namespace net
	{
		class TCPConvergenceLayer;

		class TCPConnection : public StreamConnection::Callback, public ibrcommon::DetachedThread
		{
		public:
			/**
			 * Constructor for a new TCPConnection object.
			 * @param tcpsrv
			 * @param stream TCP stream to talk to the other peer.
			 * @param name
			 * @param timeout
			 * @return
			 */
			TCPConnection(TCPConvergenceLayer &tcpsrv, ibrcommon::tcpstream *stream, const dtn::data::EID &name, const size_t timeout = 10);

			/**
			 * Constructor for a new TCPConnection object.
			 * @param tcpsrv
			 * @param node The node to talk to.
			 * @param name
			 * @param timeout
			 * @return
			 */
			TCPConnection(TCPConvergenceLayer &tcpsrv, const dtn::core::Node &node, const dtn::data::EID &name, const size_t timeout = 10);

			/**
			 * Destructor
			 * @return
			 */
			virtual ~TCPConnection();

			/**
			 * This method is called after accept()
			 */
			virtual void initialize();

			/**
			 * shutdown the whole tcp connection
			 */
			void shutdown();

			/**
			 * Get the header of this connection
			 * @return
			 */
			const StreamContactHeader& getHeader() const;

			/**
			 * Get the associated Node object
			 * @return
			 */
			const dtn::core::Node& getNode() const;

			/**
			 * callback methods for tcpstream
			 */
			virtual void eventShutdown(StreamConnection::ConnectionShutdownCases csc);
			virtual void eventTimeout();
			virtual void eventError();
			virtual void eventConnectionUp(const StreamContactHeader &header);
			virtual void eventConnectionDown();

			virtual void eventBundleRefused();
			virtual void eventBundleForwarded();
			virtual void eventBundleAck(size_t ack);

			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			/**
			 * queue a bundle for this connection
			 * @param bundle
			 */
			void queue(const dtn::data::BundleID &bundle);

			bool match(const dtn::core::Node &n) const;
			bool match(const dtn::data::EID &destination) const;
			bool match(const dtn::core::NodeEvent &evt) const;

			friend TCPConnection& operator>>(TCPConnection &conn, dtn::data::Bundle &bundle);
			friend TCPConnection& operator<<(TCPConnection &conn, const dtn::data::Bundle &bundle);

#ifdef WITH_TLS
			/*!
			 * \brief enables TLS for this connection
			 */
			void enableTLS();
#endif

		protected:
			void rejectTransmission();

			void setup();
			void connect();
			void run();
			void finally();
			void __cancellation();

			void clearQueue();

			void keepalive();
			bool good() const;

		private:
			class KeepaliveSender : public ibrcommon::JoinableThread
			{
			public:
				KeepaliveSender(TCPConnection &connection, size_t &keepalive_timeout);
				~KeepaliveSender();

				/**
				 * run method of the thread
				 */
				void run();

				/**
				 * soft-cancellation function
				 */
				void __cancellation();

			private:
				ibrcommon::Conditional _wait;
				TCPConnection &_connection;
				size_t &_keepalive_timeout;
			};

			class Sender : public ibrcommon::JoinableThread, public ibrcommon::Queue<dtn::data::BundleID>
			{
			public:
				Sender(TCPConnection &connection);
				virtual ~Sender();

			protected:
				void run();
				void finally();
				void __cancellation();

			private:
				TCPConnection &_connection;
				//dtn::data::BundleID _current_transfer;
			};

			StreamContactHeader _peer;
			dtn::core::Node _node;

			std::auto_ptr<ibrcommon::tcpstream> _tcpstream;

#ifdef WITH_TLS
			ibrcommon::TLSStream _tlsstream;
#endif

			StreamConnection _stream;

			// This thread gets awaiting bundles of the queue
			// and transmit them to the peer.
			Sender _sender;

			// Keepalive sender
			KeepaliveSender _keepalive_sender;

			// handshake variables
			dtn::data::EID _name;
			size_t _timeout;

			ibrcommon::Queue<dtn::data::MetaBundle> _sentqueue;
			size_t _lastack;
			size_t _keepalive_timeout;

			TCPConvergenceLayer &_callback;

			/* flags to be used in this nodes StreamContactHeader */
			char _flags;

			/* with this boolean the connection is marked as aborted */
			bool _aborted;
		};

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
			void __cancellation();

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

			ibrcommon::tcpserver _tcpsrv;

			ibrcommon::Conditional _connections_cond;
			std::list<TCPConnection*> _connections;
			std::list<ibrcommon::vinterface> _interfaces;
			std::map<ibrcommon::vinterface, unsigned int> _portmap;
			unsigned int _any_port;
		};
	}
}

#endif /* TCPCONVERGENCELAYER_H_ */
