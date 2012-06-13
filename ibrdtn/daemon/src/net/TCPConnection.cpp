/*
 * TCPConnection.cpp
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

#include "config.h"
#include "Configuration.h"
#include "core/BundleCore.h"
#include "core/BundleEvent.h"
#include "storage/BundleStorage.h"
#include "core/FragmentManager.h"

#include "net/TCPConvergenceLayer.h"
#include "net/BundleReceivedEvent.h"
#include "net/ConnectionEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "routing/RequeueBundleEvent.h"

#include <ibrdtn/data/ScopeControlHopLimitBlock.h>
#include <ibrcommon/net/tcpclient.h>
#include <ibrcommon/TimeMeasurement.h>
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/thread/Conditional.h>
#include <ibrcommon/Logger.h>

#include <iostream>
#include <iomanip>

#ifdef WITH_TLS
#include <openssl/x509.h>
#include "security/SecurityCertificateManager.h"
#include <ibrcommon/TLSExceptions.h>
#endif

#ifdef WITH_BUNDLE_SECURITY
#include "security/SecurityManager.h"
#endif

namespace dtn
{
	namespace net
	{
		/*
		 * class TCPConnection
		 */
		TCPConnection::TCPConnection(TCPConvergenceLayer & tcpsrv, ibrcommon::tcpstream *stream, const dtn::data::EID & name, const size_t timeout)
#ifdef WITH_TLS
		 : _peer(), _node(name), _tcpstream(stream), _tlsstream(stream), _stream(*this, _tlsstream, dtn::daemon::Configuration::getInstance().getNetwork().getTCPChunkSize()),
		   _sender(*this), _keepalive_sender(*this, _keepalive_timeout), _name(name), _timeout(timeout), _lastack(0), _keepalive_timeout(0), _callback(tcpsrv), _flags(0), _aborted(false)
#else
		 : _peer(), _node(name), _tcpstream(stream), _stream(*this, *_tcpstream, dtn::daemon::Configuration::getInstance().getNetwork().getTCPChunkSize()),
		   _sender(*this), _keepalive_sender(*this, _keepalive_timeout), _name(name), _timeout(timeout), _lastack(0), _keepalive_timeout(0), _callback(tcpsrv), _flags(0), _aborted(false)
#endif
		{
			_stream.exceptions(std::ios::badbit | std::ios::eofbit);

			if ( dtn::daemon::Configuration::getInstance().getNetwork().getTCPOptionNoDelay() )
			{
				stream->enableNoDelay();
			}

			// add default TCP connection
			_node.clear();
			_node.add( dtn::core::Node::URI(Node::NODE_CONNECTED, Node::CONN_TCPIP, "0.0.0.0", 0, 30) );

			_flags |= dtn::streams::StreamContactHeader::REQUEST_ACKNOWLEDGMENTS;
			_flags |= dtn::streams::StreamContactHeader::REQUEST_NEGATIVE_ACKNOWLEDGMENTS;

			if (dtn::daemon::Configuration::getInstance().getNetwork().doFragmentation())
			{
				_flags |= dtn::streams::StreamContactHeader::REQUEST_FRAGMENTATION;
			}

#ifdef WITH_TLS
			// set tls mode to server
			_tlsstream.setServer(true);
#endif
		}

		TCPConnection::TCPConnection(TCPConvergenceLayer &tcpsrv, const dtn::core::Node &node, const dtn::data::EID &name, const size_t timeout)
#ifdef WITH_TLS
		 : _peer(), _node(node), _tcpstream(new ibrcommon::tcpclient()), _tlsstream(_tcpstream.get()), _stream(*this, _tlsstream, dtn::daemon::Configuration::getInstance().getNetwork().getTCPChunkSize()),
		   _sender(*this), _keepalive_sender(*this, _keepalive_timeout), _name(name), _timeout(timeout), _lastack(0), _keepalive_timeout(0), _callback(tcpsrv), _flags(0), _aborted(false)
#else
		 : _peer(), _node(node), _tcpstream(new ibrcommon::tcpclient()), _stream(*this, *_tcpstream, dtn::daemon::Configuration::getInstance().getNetwork().getTCPChunkSize()),
		   _sender(*this), _keepalive_sender(*this, _keepalive_timeout), _name(name), _timeout(timeout), _lastack(0), _keepalive_timeout(0), _callback(tcpsrv), _flags(0), _aborted(false)
#endif
		{
			_stream.exceptions(std::ios::badbit | std::ios::eofbit);

			_flags |= dtn::streams::StreamContactHeader::REQUEST_ACKNOWLEDGMENTS;
			_flags |= dtn::streams::StreamContactHeader::REQUEST_NEGATIVE_ACKNOWLEDGMENTS;

			if (dtn::daemon::Configuration::getInstance().getNetwork().doFragmentation())
			{
				_flags |= dtn::streams::StreamContactHeader::REQUEST_FRAGMENTATION;
			}
		}

		TCPConnection::~TCPConnection()
		{
			// join the keepalive sender thread
			_keepalive_sender.join();

			// wait until the sender thread is finished
			_sender.join();
		}

		void TCPConnection::queue(const dtn::data::BundleID &bundle)
		{
			_sender.push(bundle);
		}

		const StreamContactHeader& TCPConnection::getHeader() const
		{
			return _peer;
		}

		const dtn::core::Node& TCPConnection::getNode() const
		{
			return _node;
		}

		void TCPConnection::rejectTransmission()
		{
			_stream.reject();
		}

		void TCPConnection::eventShutdown(StreamConnection::ConnectionShutdownCases)
		{
		}

		void TCPConnection::eventTimeout()
		{
			// event
			ConnectionEvent::raise(ConnectionEvent::CONNECTION_TIMEOUT, _node);

			// stop the receiver thread
			this->stop();
		}

		void TCPConnection::eventError()
		{
		}

		void TCPConnection::eventConnectionUp(const StreamContactHeader &header)
		{
			_peer = header;

			// copy old attributes and urls to the new node object
			Node n_old = _node;
			_node = Node(header._localeid);
			_node += n_old;

			_keepalive_timeout = header._keepalive * 1000;

#ifdef WITH_TLS
			/* if both nodes support TLS, activate it */
			if((_peer._flags & dtn::streams::StreamContactHeader::REQUEST_TLS)
					&& (_flags & dtn::streams::StreamContactHeader::REQUEST_TLS)){
				try{
					X509 *peer_cert = _tlsstream.activate();
					if(!dtn::security::SecurityCertificateManager::validateSubject(peer_cert, _peer.getEID())){
						IBRCOMMON_LOGGER(warning) << "TCPConnection: certificate does not fit the EID." << IBRCOMMON_LOGGER_ENDL;
						throw ibrcommon::TLSCertificateVerificationException("certificate does not fit the EID");
					}
				} catch(...){
					if(dtn::daemon::Configuration::getInstance().getSecurity().TLSRequired()){
						/* close the connection */
						IBRCOMMON_LOGGER(notice) << "TCPConnection: TLS failed, closing the connection." << IBRCOMMON_LOGGER_ENDL;
						throw;
					} else {
						IBRCOMMON_LOGGER(notice) << "TCPConnection: TLS failed, continuing unauthenticated." << IBRCOMMON_LOGGER_ENDL;
					}
				}
			} else {
				/* TLS not supported by both Nodes, check if its required */
				if(dtn::daemon::Configuration::getInstance().getSecurity().TLSRequired()){
					/* close the connection */
					IBRCOMMON_LOGGER(notice) << "TCPConnection: TLS not supported by both Peers. Closing the connection." << IBRCOMMON_LOGGER_ENDL;
					throw ibrcommon::TLSException("TLS not supported by peer.");
				} else if(_flags & dtn::streams::StreamContactHeader::REQUEST_TLS){
					IBRCOMMON_LOGGER(notice) << "TCPConnection: TLS not supported by peer. Continuing without TLS." << IBRCOMMON_LOGGER_ENDL;
				}
				/* else: this node does not support TLS, should have already printed a warning */
			}
#endif

			// set the incoming timer if set (> 0)
			if (_peer._keepalive > 0)
			{
				// set the timer
				_tcpstream->setTimeout(header._keepalive * 2);
			}

			// enable idle timeout
			size_t _idle_timeout = dtn::daemon::Configuration::getInstance().getNetwork().getTCPIdleTimeout();
			if (_idle_timeout > 0)
			{
				_stream.enableIdleTimeout(_idle_timeout);
			}

			// raise up event
			ConnectionEvent::raise(ConnectionEvent::CONNECTION_UP, _node);
		}

		void TCPConnection::eventConnectionDown()
		{
			IBRCOMMON_LOGGER_DEBUG(40) << "TCPConnection::eventConnectionDown()" << IBRCOMMON_LOGGER_ENDL;

			try {
				// shutdown the keepalive sender thread
				_keepalive_sender.stop();

				// stop the sender
				_sender.stop();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_DEBUG(50) << "TCPConnection::eventConnectionDown(): ThreadException (" << ex.what() << ")" << IBRCOMMON_LOGGER_ENDL;
			}

			if (_peer._localeid != dtn::data::EID())
			{
				// event
				ConnectionEvent::raise(ConnectionEvent::CONNECTION_DOWN, _node);
			}
		}

		void TCPConnection::eventBundleRefused()
		{
			try {
				const dtn::data::BundleID bundle = _sentqueue.getnpop();

				// requeue the bundle
				TransferAbortedEvent::raise(EID(_node.getEID()), bundle, dtn::net::TransferAbortedEvent::REASON_REFUSED);

				// set ACK to zero
				_lastack = 0;

			} catch (const ibrcommon::QueueUnblockedException&) {
				// pop on empty queue!
				IBRCOMMON_LOGGER(error) << "transfer refused without a bundle in queue" << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void TCPConnection::eventBundleForwarded()
		{
			try {
				const dtn::data::MetaBundle bundle = _sentqueue.getnpop();

				// signal completion of the transfer
				TransferCompletedEvent::raise(_node.getEID(), bundle);

				// raise bundle event
				dtn::core::BundleEvent::raise(bundle, BUNDLE_FORWARDED);

				// set ACK to zero
				_lastack = 0;
			} catch (const ibrcommon::QueueUnblockedException&) {
				// pop on empty queue!
				IBRCOMMON_LOGGER(error) << "transfer completed without a bundle in queue" << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void TCPConnection::eventBundleAck(size_t ack)
		{
			_lastack = ack;
		}

		void TCPConnection::initialize()
		{
			// start the receiver for incoming bundles + handshake
			try {
				start();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER(error) << "failed to start thread in TCPConnection\n" << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void TCPConnection::shutdown()
		{
			// shutdown
			_stream.shutdown(StreamConnection::CONNECTION_SHUTDOWN_ERROR);

			try {
				// abort the connection thread
				ibrcommon::DetachedThread::stop();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_DEBUG(50) << "TCPConnection::shutdown(): ThreadException (" << ex.what() << ")" << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void TCPConnection::__cancellation()
		{
			// mark the connection as aborted
			_aborted = true;

			// close the stream
			try {
				(*_tcpstream).close();
			} catch (const ibrcommon::ConnectionClosedException&) { };
		}

		void TCPConnection::finally()
		{
			IBRCOMMON_LOGGER_DEBUG(60) << "TCPConnection down" << IBRCOMMON_LOGGER_ENDL;

			try {
				// shutdown the keepalive sender thread
				_keepalive_sender.stop();

				// shutdown the sender thread
				_sender.stop();
			} catch (const std::exception&) { };

			// close the tcpstream
			try {
				_tcpstream->close();
			} catch (const ibrcommon::ConnectionClosedException&) { };

			try {
				_callback.connectionDown(this);
			} catch (const ibrcommon::MutexException&) { };

			// clear the queue
			clearQueue();
		}

		void TCPConnection::setup()
		{
			// nothing to do here
		}

		void TCPConnection::connect()
		{
			// variables for address and port
			std::string address = "0.0.0.0";
			unsigned int port = 0;

			// try to connect to the other side
			try {
				const std::list<dtn::core::Node::URI> uri_list = _node.get(dtn::core::Node::CONN_TCPIP);

				for (std::list<dtn::core::Node::URI>::const_iterator iter = uri_list.begin(); iter != uri_list.end(); iter++)
				{
					// break-out if the connection has been aborted
					if (_aborted) throw ibrcommon::tcpclient::SocketException("connection has been aborted");

					try {
						// decode address and port
						const dtn::core::Node::URI &uri = (*iter);
						uri.decode(address, port);

						ibrcommon::tcpclient &client = dynamic_cast<ibrcommon::tcpclient&>(*_tcpstream);

						IBRCOMMON_LOGGER_DEBUG(15) << "Initiate TCP connection to " << address << ":" << port << IBRCOMMON_LOGGER_ENDL;
						client.open(address, port, _timeout);

						if ( dtn::daemon::Configuration::getInstance().getNetwork().getTCPOptionNoDelay() )
						{
							_tcpstream->enableNoDelay();
						}

						// add TCP connection descriptor to the node object
						_node.clear();
						_node.add( dtn::core::Node::URI(Node::NODE_CONNECTED, Node::CONN_TCPIP, uri.value, 0, 30) );

						// connection successful
						return;
					} catch (const ibrcommon::tcpclient::SocketException&) { };
				}

				// no connection has been established
				throw ibrcommon::tcpclient::SocketException("no address available to connect");

			} catch (const ibrcommon::tcpclient::SocketException&) {
				// error on open, requeue all bundles in the queue
				IBRCOMMON_LOGGER(warning) << "connection to " << _node.toString() << " failed" << IBRCOMMON_LOGGER_ENDL;
				_stream.shutdown(StreamConnection::CONNECTION_SHUTDOWN_ERROR);
				throw;
			} catch (const bad_cast&) { };
		}

		void TCPConnection::run()
		{
			try {
				// connect to the peer
				connect();

				// do the handshake
				_stream.handshake(_name, _timeout, _flags);

				// start the sender
				_sender.start();

				// start keepalive sender
				_keepalive_sender.start();

				while (!_stream.eof())
				{
					try {
						// create a new empty bundle
						dtn::data::Bundle bundle;

						// deserialize the bundle
						(*this) >> bundle;

						// check the bundle
						if ( ( bundle._destination == EID() ) || ( bundle._source == EID() ) )
						{
							// invalid bundle!
							throw dtn::data::Validator::RejectedException("destination or source EID is null");
						}

						// increment value in the scope control hop limit block
						try {
							dtn::data::ScopeControlHopLimitBlock &schl = bundle.getBlock<dtn::data::ScopeControlHopLimitBlock>();
							schl.increment();
						} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { }

						// raise default bundle received event
						dtn::net::BundleReceivedEvent::raise(_peer._localeid, bundle, false, true);
					}
					catch (const dtn::data::Validator::RejectedException &ex)
					{
						// bundle rejected
						rejectTransmission();

						// display the rejection
						IBRCOMMON_LOGGER(warning) << "bundle has been rejected: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
					catch (const dtn::InvalidDataException &ex) {
						// bundle rejected
						rejectTransmission();

						// display the rejection
						IBRCOMMON_LOGGER(warning) << "invalid bundle-data received: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}

					yield();
				}
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER(error) << "failed to start thread in TCPConnection\n" << ex.what() << IBRCOMMON_LOGGER_ENDL;
				_stream.shutdown(StreamConnection::CONNECTION_SHUTDOWN_ERROR);
			} catch (const std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "TCPConnection::run(): std::exception (" << ex.what() << ")" << IBRCOMMON_LOGGER_ENDL;
				_stream.shutdown(StreamConnection::CONNECTION_SHUTDOWN_ERROR);
			}
		}


		TCPConnection& operator>>(TCPConnection &conn, dtn::data::Bundle &bundle)
		{
			std::iostream &stream = conn._stream;

			// check if the stream is still good
			if (!stream.good()) throw ibrcommon::IOException("stream went bad");

			// create a deserializer for next bundle
			dtn::data::DefaultDeserializer deserializer(stream, dtn::core::BundleCore::getInstance());

			// enable/disable fragmentation support according to the contact header.
			deserializer.setFragmentationSupport(conn._peer._flags & StreamContactHeader::REQUEST_FRAGMENTATION);

			// read the bundle (or the fragment if fragmentation is enabled)
			deserializer >> bundle;

			return conn;
		}

		TCPConnection& operator<<(TCPConnection &conn, const dtn::data::Bundle &bundle)
		{
			// prepare a measurement
			ibrcommon::TimeMeasurement m;

			std::iostream &stream = conn._stream;

			// get the offset, if this bundle has been reactively fragmented before
			size_t offset = 0;
			if (dtn::daemon::Configuration::getInstance().getNetwork().doFragmentation()
					&& !bundle.get(dtn::data::PrimaryBlock::DONT_FRAGMENT))
			{
				offset = dtn::core::FragmentManager::getOffset(conn.getNode().getEID(), bundle);
			}

			// create a serializer
			dtn::data::DefaultSerializer serializer(stream);

			// put the bundle into the sentqueue
			conn._sentqueue.push(bundle);

			// start the measurement
			m.start();

			try {
				// activate exceptions for this method
				if (!stream.good()) throw ibrcommon::IOException("stream went bad");

				if (offset > 0)
				{
					// transmit the fragment
					serializer << dtn::data::BundleFragment(bundle, offset, -1);
				}
				else
				{
					// transmit the bundle
					serializer << bundle;
				}

				// flush the stream
				stream << std::flush;

				// stop the time measurement
				m.stop();

				// get throughput
				double kbytes_per_second = (serializer.getLength(bundle) / m.getSeconds()) / 1024;

				// print out throughput
				IBRCOMMON_LOGGER_DEBUG(5) << "transfer finished after " << m << " with "
						<< std::setiosflags(std::ios::fixed) << std::setprecision(2) << kbytes_per_second << " kb/s" << IBRCOMMON_LOGGER_ENDL;

			} catch (const ibrcommon::Exception &ex) {
				// the connection not available
				IBRCOMMON_LOGGER_DEBUG(10) << "connection error: " << ex.what() << IBRCOMMON_LOGGER_ENDL;

				// forward exception
				throw;
			}

			return conn;
		}

		TCPConnection::KeepaliveSender::KeepaliveSender(TCPConnection &connection, size_t &keepalive_timeout)
		 : _connection(connection), _keepalive_timeout(keepalive_timeout)
		{

		}

		TCPConnection::KeepaliveSender::~KeepaliveSender()
		{
		}

		void TCPConnection::KeepaliveSender::run()
		{
			try {
				ibrcommon::MutexLock l(_wait);
				while (true)
				{
					try {
						_wait.wait(_keepalive_timeout);
					} catch (const ibrcommon::Conditional::ConditionalAbortException &ex) {
						if (ex.reason == ibrcommon::Conditional::ConditionalAbortException::COND_TIMEOUT)
						{
							// send a keepalive
							_connection.keepalive();
						}
						else
						{
							throw;
						}
					}
				}
			} catch (const std::exception&) { };
		}

		void TCPConnection::KeepaliveSender::__cancellation()
		{
			ibrcommon::MutexLock l(_wait);
			_wait.abort();
		}

		TCPConnection::Sender::Sender(TCPConnection &connection)
		 : _connection(connection)
		{
		}

		TCPConnection::Sender::~Sender()
		{
		}

		void TCPConnection::Sender::__cancellation()
		{
			// cancel the main thread in here
			ibrcommon::Queue<dtn::data::BundleID>::abort();
		}

		void TCPConnection::Sender::run()
		{
			try {
				dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();

				while (_connection.good())
				{
					dtn::data::BundleID transfer = ibrcommon::Queue<dtn::data::BundleID>::getnpop(true);

					try {
						// read the bundle out of the storage
						dtn::data::Bundle bundle = storage.get(transfer);

#ifdef WITH_BUNDLE_SECURITY
						const dtn::daemon::Configuration::Security::Level seclevel =
								dtn::daemon::Configuration::getInstance().getSecurity().getLevel();

						if (seclevel & dtn::daemon::Configuration::Security::SECURITY_LEVEL_AUTHENTICATED)
						{
							try {
								dtn::security::SecurityManager::getInstance().auth(bundle);
							} catch (const dtn::security::SecurityManager::KeyMissingException&) {
								// sign requested, but no key is available
								IBRCOMMON_LOGGER(warning) << "No key available for sign process." << IBRCOMMON_LOGGER_ENDL;
							}
						}
#endif
						// send bundle
						_connection << bundle;
					} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
						// send transfer aborted event
						TransferAbortedEvent::raise(_connection._node.getEID(), transfer, dtn::net::TransferAbortedEvent::REASON_BUNDLE_DELETED);
					}

					// idle a little bit
					yield();
				}
			} catch (const ibrcommon::QueueUnblockedException &ex) {
				IBRCOMMON_LOGGER_DEBUG(50) << "TCPConnection::Sender::run(): aborted" << IBRCOMMON_LOGGER_ENDL;
				return;
			} catch (const std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "TCPConnection::Sender terminated by exception: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			_connection.stop();
		}

		void TCPConnection::clearQueue()
		{
			// requeue all bundles still queued
			try {
				while (true)
				{
					const dtn::data::BundleID id = _sender.getnpop();

					// raise transfer abort event for all bundles without an ACK
					dtn::routing::RequeueBundleEvent::raise(_node.getEID(), id);
				}
			} catch (const ibrcommon::QueueUnblockedException&) {
				// queue emtpy
			}

			// requeue all bundles still in transit
			try {
				while (true)
				{
					const dtn::data::BundleID id = _sentqueue.getnpop();

					if ((_lastack > 0) && (_peer._flags & dtn::streams::StreamContactHeader::REQUEST_FRAGMENTATION))
					{
						// some data are already acknowledged
						// store this information in the fragment manager
						dtn::core::FragmentManager::setOffset(_peer.getEID(), id, _lastack);

						// raise transfer abort event for all bundles without an ACK
						dtn::routing::RequeueBundleEvent::raise(_node.getEID(), id);
					}
					else
					{
						// raise transfer abort event for all bundles without an ACK
						dtn::routing::RequeueBundleEvent::raise(_node.getEID(), id);
					}

					// set last ack to zero
					_lastack = 0;
				}
			} catch (const ibrcommon::QueueUnblockedException&) {
				// queue emtpy
			}
		}

#ifdef WITH_TLS
		void dtn::net::TCPConnection::enableTLS()
		{
			_flags |= dtn::streams::StreamContactHeader::REQUEST_TLS;
		}
#endif

		void TCPConnection::keepalive()
		{
			_stream.keepalive();
		}

		bool TCPConnection::good() const
		{
			return _tcpstream->good();
		}

		void TCPConnection::Sender::finally()
		{
		}

		bool TCPConnection::match(const dtn::core::Node &n) const
		{
			return (_node == n);
		}

		bool TCPConnection::match(const dtn::data::EID &destination) const
		{
			return (_node.getEID() == destination.getNode());
		}

		bool TCPConnection::match(const NodeEvent &evt) const
		{
			const dtn::core::Node &n = evt.getNode();
			return match(n);
		}
	}
}
