/*
 * UDPConvergenceLayer.cpp
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

#include "net/UDPConvergenceLayer.h"
#include "net/BundleReceivedEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "core/BundleEvent.h"
#include "core/BundleCore.h"
#include "routing/RequeueBundleEvent.h"
#include "Configuration.h"

#include <ibrdtn/utils/Utils.h>
#include <ibrdtn/data/Serializer.h>

#include <ibrcommon/net/socket.h>
#include <ibrcommon/net/vaddress.h>
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/MutexLock.h>

#include <sys/socket.h>
#include <poll.h>
#include <errno.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

#include <iostream>
#include <list>


using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		const int UDPConvergenceLayer::DEFAULT_PORT = 4556;

		UDPConvergenceLayer::UDPConvergenceLayer(ibrcommon::vinterface net, int port, unsigned int mtu)
		 : _net(net), _port(port), m_maxmsgsize(mtu), _running(false)
		{
		}

		UDPConvergenceLayer::~UDPConvergenceLayer()
		{
			componentDown();
		}

		dtn::core::Node::Protocol UDPConvergenceLayer::getDiscoveryProtocol() const
		{
			return dtn::core::Node::CONN_UDPIP;
		}

		void UDPConvergenceLayer::update(const ibrcommon::vinterface &iface, DiscoveryAnnouncement &announcement) throw (dtn::net::DiscoveryServiceProvider::NoServiceHereException)
		{
			// announce port only if we are bound to any interface
			if (_net.empty()) {
				std::stringstream service;
				// ... set the port only
				service << "port=" << _port << ";";
				announcement.addService( DiscoveryService("udpcl", service.str()));
				return;
			}

			// do not announce if this is not our interface
			if (iface != _net) throw dtn::net::DiscoveryServiceProvider::NoServiceHereException();
			
			// determine if we should enable crosslayer discovery by sending out our own address
			bool crosslayer = dtn::daemon::Configuration::getInstance().getDiscovery().enableCrosslayer();

			// this marker should set to true if we added an service description
			bool announced = false;

			try {
				// check if cross layer discovery is disabled
				if (!crosslayer) throw ibrcommon::Exception("crosslayer discovery disabled!");

				// get all global addresses of the interface
				std::list<ibrcommon::vaddress> list = _net.getAddresses();

				// if no address is returned... (goto catch block)
				if (list.empty()) throw ibrcommon::Exception("no address found");

				for (std::list<ibrcommon::vaddress>::const_iterator addr_it = list.begin(); addr_it != list.end(); addr_it++)
				{
					const ibrcommon::vaddress &addr = (*addr_it);

					// only announce global scope addresses
					if (addr.scope() != ibrcommon::vaddress::SCOPE_GLOBAL) continue;

					std::stringstream service;
					// fill in the ip address
					service << "ip=" << addr.address() << ";port=" << _port << ";";
					announcement.addService( DiscoveryService("udpcl", service.str()));

					// set the announce mark
					announced = true;
				}
			} catch (const ibrcommon::Exception&) {
				// address collection process aborted
			}

			// if we still not announced anything...
			if (!announced) {
				// announce at least our local port
				std::stringstream service;
				service << "port=" << _port << ";";
				announcement.addService( DiscoveryService("udpcl", service.str()));
			}
		}

		void UDPConvergenceLayer::queue(const dtn::core::Node &node, const ConvergenceLayer::Job &job)
		{
			const std::list<dtn::core::Node::URI> uri_list = node.get(dtn::core::Node::CONN_UDPIP);
			if (uri_list.empty())
			{
				dtn::net::TransferAbortedEvent::raise(node.getEID(), job._bundle, dtn::net::TransferAbortedEvent::REASON_UNDEFINED);
				return;
			}

			dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();

			const dtn::core::Node::URI &uri = uri_list.front();

			std::string address = "0.0.0.0";
			unsigned int port = 0;

			// read values
			uri.decode(address, port);

			// get the address of the node
			ibrcommon::vaddress addr(address, port);

			try {
				// read the bundle out of the storage
				const dtn::data::Bundle bundle = storage.get(job._bundle);

				// create a dummy serializer
				dtn::data::DefaultSerializer dummy(std::cout);

				size_t header = dummy.getLength((const PrimaryBlock&)bundle);
				header += 20; // two times SDNV through fragmentation

				unsigned int size = dummy.getLength(bundle);

				if (size > m_maxmsgsize)
				{
					// abort transmission if fragmentation is disabled
					if (!dtn::daemon::Configuration::getInstance().getNetwork().doFragmentation()
							&& !bundle.get(dtn::data::PrimaryBlock::DONT_FRAGMENT))
					{
						throw ConnectionInterruptedException();
					}

					const size_t psize = bundle.getBlock<dtn::data::PayloadBlock>().getLength();
					const size_t fragment_size = m_maxmsgsize - header;
					const size_t fragment_count = (psize / fragment_size) + (((psize % fragment_size) > 0) ? 1 : 0);

					IBRCOMMON_LOGGER_DEBUG(15) << "MTU of " << m_maxmsgsize << " is too small to carry " << psize << " bytes of payload." << IBRCOMMON_LOGGER_ENDL;
					IBRCOMMON_LOGGER_DEBUG(15) << "create " << fragment_count << " fragments with " << fragment_size << " bytes each." << IBRCOMMON_LOGGER_ENDL;

					for (size_t i = 0; i < fragment_count; i++)
					{
						dtn::data::BundleFragment fragment(bundle, i * fragment_size, fragment_size);

						std::stringstream ss;
						dtn::data::DefaultSerializer serializer(ss);

						serializer << fragment;
						string data = ss.str();

						try {
							// set write lock
							ibrcommon::MutexLock l(m_writelock);

							// send converted line back to client.
							_sock.sendto(data.c_str(), data.length(), 0, addr);
						} catch (const ibrcommon::socket_exception&) {
							// CL is busy, requeue bundle
							dtn::routing::RequeueBundleEvent::raise(job._destination, job._bundle);

							return;
						}
					}
				}
				else
				{
					std::stringstream ss;
					dtn::data::DefaultSerializer serializer(ss);

					serializer << bundle;
					string data = ss.str();

					try {
						// set write lock
						ibrcommon::MutexLock l(m_writelock);

						// send converted line back to client.
						_sock.sendto(data.c_str(), data.length(), 0, addr);
					} catch (const ibrcommon::socket_exception&) {
						// CL is busy, requeue bundle
						dtn::routing::RequeueBundleEvent::raise(job._destination, job._bundle);
						return;
					}
				}

				// raise bundle event
				dtn::net::TransferCompletedEvent::raise(job._destination, bundle);
				dtn::core::BundleEvent::raise(bundle, dtn::core::BUNDLE_FORWARDED);
			} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
				// send transfer aborted event
				dtn::net::TransferAbortedEvent::raise(node.getEID(), job._bundle, dtn::net::TransferAbortedEvent::REASON_BUNDLE_DELETED);
			}

		}

		void UDPConvergenceLayer::receive(dtn::data::Bundle &bundle, dtn::data::EID &sender)
		{
			ibrcommon::MutexLock l(m_readlock);

			char data[m_maxmsgsize];

			// data waiting
			ibrcommon::socketset readfds;

			try {
				// wait for incoming messages
				_vsocket.select(&readfds, NULL, NULL, NULL);

				if (readfds.size() > 0) {
					ibrcommon::datagramsocket *sock = static_cast<ibrcommon::datagramsocket*>(*readfds.begin());

					ibrcommon::vaddress fromaddr;
					size_t len = sock->recvfrom(data, m_maxmsgsize, 0, fromaddr);

					std::stringstream ss; ss << "udp://" << fromaddr.toString();
					sender = dtn::data::EID(ss.str());

					if (len > 0)
					{
						// read all data into a stream
						stringstream ss;
						ss.write(data, len);

						// get the bundle
						dtn::data::DefaultDeserializer(ss, dtn::core::BundleCore::getInstance()) >> bundle;
					}
				}
			} catch (const ibrcommon::socket_exception&) {

			}
		}

		void UDPConvergenceLayer::componentUp()
		{
			try {
				// create main socket for all send actions
				_sock.up();

				// TODO: add one udpsocket for each address on the interface (_net)
				_vsocket.add(new ibrcommon::udpsocket(_port), _net);

				_vsocket.up();
			} catch (const ibrcommon::socket_exception&) {

			}
		}

		void UDPConvergenceLayer::componentDown()
		{
			_sock.down();
			_vsocket.destroy();
			stop();
			join();
		}

		void UDPConvergenceLayer::componentRun()
		{
			_running = true;

			while (_running)
			{
				try {
					dtn::data::Bundle bundle;
					EID sender;

					receive(bundle, sender);

					// raise default bundle received event
					dtn::net::BundleReceivedEvent::raise(sender, bundle, false, true);

				} catch (const dtn::InvalidDataException &ex) {
					IBRCOMMON_LOGGER(warning) << "Received a invalid bundle: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				} catch (const ibrcommon::IOException &ex) {

				}
				yield();
			}
		}

		void UDPConvergenceLayer::__cancellation()
		{
			_running = false;
			_sock.down();
			_vsocket.down();
		}

		const std::string UDPConvergenceLayer::getName() const
		{
			return "UDPConvergenceLayer";
		}
	}
}
