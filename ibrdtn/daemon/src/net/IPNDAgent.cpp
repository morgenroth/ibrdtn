/*
 * IPNDAgent.cpp
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

#include "net/IPNDAgent.h"
#include "core/BundleCore.h"

#include <ibrdtn/data/Exceptions.h>

#include <ibrcommon/Logger.h>
#include <ibrcommon/TimeMeasurement.h>
#include <ibrcommon/net/socket.h>

#include <sstream>
#include <string.h>
#include <typeinfo>
#include <time.h>

namespace dtn
{
	namespace net
	{
		IPNDAgent::IPNDAgent(int port)
		 : DiscoveryAgent(dtn::daemon::Configuration::getInstance().getDiscovery()),
		   _version(DiscoveryAnnouncement::DISCO_VERSION_01)
		{
			// bind to receive socket
			_recv_socket.add(new ibrcommon::multicastsocket(port));

			switch (_config.version())
			{
			case 2:
				_version = DiscoveryAnnouncement::DISCO_VERSION_01;
				break;

			case 1:
				_version = DiscoveryAnnouncement::DISCO_VERSION_00;
				break;

			case 0:
				IBRCOMMON_LOGGER(info) << "DiscoveryAgent: DTN2 compatibility mode" << IBRCOMMON_LOGGER_ENDL;
				_version = DiscoveryAnnouncement::DTND_IPDISCOVERY;
				break;
			};
		}

		IPNDAgent::~IPNDAgent()
		{
		}

		void IPNDAgent::add(const ibrcommon::vaddress &address) {
			IBRCOMMON_LOGGER(info) << "DiscoveryAgent: listen to " << address.toString() << IBRCOMMON_LOGGER_ENDL;
			_destinations.push_back(address);

			// add new socket for this address
			_send_socket.add(new ibrcommon::udpsocket(address));
		}

		void IPNDAgent::bind(const ibrcommon::vinterface &net, int port)
		{
			IBRCOMMON_LOGGER(info) << "DiscoveryAgent: add interface " << net.toString() << IBRCOMMON_LOGGER_ENDL;
			_interfaces.push_back(net);

			// TODO: create sockets for all addresses on the interface

//			// bind to all added interfaces for sending
//			for (std::list<ibrcommon::vinterface>::const_iterator iter = _interfaces.begin(); iter != _interfaces.end(); iter++)
//			{
//				try {
//					const ibrcommon::vinterface &iface = *iter;
//					if (!iface.empty())
//					{
//						// create one send socket for each interface
//						_send_socket.bind(iface, 0, SOCK_DGRAM);
//					}
//				} catch (const ibrcommon::Exception &ex) {
//					IBRCOMMON_LOGGER(error) << "[IPND] bind failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
//				}
//			}
		}

		void IPNDAgent::send(const DiscoveryAnnouncement &a, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr)
		{
			// serialize announcement
			stringstream ss; ss << a;
			const std::string data = ss.str();

			ibrcommon::socketset fds = _send_socket.get(iface);

			// send out discovery announcements on all bound sockets
			// (hopefully only one per interface)
			for (ibrcommon::socketset::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
			{
				try {
					ibrcommon::udpsocket &sock = dynamic_cast<ibrcommon::udpsocket&>(**iter);
					sock.sendto(data.c_str(), data.length(), 0, addr);

				} catch (const ibrcommon::socket_exception&) {
					IBRCOMMON_LOGGER_DEBUG(5) << "can not send message to " << addr.toString() << IBRCOMMON_LOGGER_ENDL;
				}
			}
		}

		void IPNDAgent::sendAnnoucement(const u_int16_t &sn, std::list<dtn::net::DiscoveryServiceProvider*> &providers)
		{
			DiscoveryAnnouncement announcement(_version, dtn::core::BundleCore::local);

			// set sequencenumber
			announcement.setSequencenumber(sn);

			for (std::list<ibrcommon::vinterface>::const_iterator it_iface = _interfaces.begin(); it_iface != _interfaces.end(); it_iface++)
			{
				const ibrcommon::vinterface &iface = (*it_iface);

				// clear all services
				announcement.clearServices();

				if (!_config.shortbeacon())
				{
					// add services
					for (std::list<DiscoveryServiceProvider*>::iterator iter = providers.begin(); iter != providers.end(); iter++)
					{
						DiscoveryServiceProvider &provider = (**iter);

						try {
							// update service information
							provider.update(iface, announcement);
						} catch (const dtn::net::DiscoveryServiceProvider::NoServiceHereException&) {

						}
					}
				}

				for (std::list<ibrcommon::vaddress>::iterator iter = _destinations.begin(); iter != _destinations.end(); iter++) {
					send(announcement, iface, (*iter));
				}
			}
		}

		void IPNDAgent::eventNotify(const ibrcommon::LinkManagerEvent &evt)
		{
			if (evt.getType() == ibrcommon::LinkManagerEvent::EVENT_ADDRESS_ADDED)
			{
				// TODO: add new udpsocket for the new address
//				for (std::list<ibrcommon::vaddress>::iterator iter = _destinations.begin(); iter != _destinations.end(); iter++) {
//					_recv_socket.join(*iter, evt.getInterface());
//				}
			}

			// TODO: shutdown and remove sockets on EVENT_ADDRESS_REMOVED
			
			// TODO: join/leave interfaces if a interface is totally new / gone
		}

		void IPNDAgent::componentUp()
		{
			// setup the receive socket
			_recv_socket.up();

			// setup all send sockets
			_send_socket.up();

			// join multicast groups
			try {
				ibrcommon::socketset socks = _recv_socket.getAll();
				if (socks.size() == 0) throw ibrcommon::socket_exception("no multicast socket found");
				ibrcommon::multicastsocket &msock = dynamic_cast<ibrcommon::multicastsocket&>(**socks.begin());

				for (std::list<ibrcommon::vinterface>::const_iterator it_iface = _interfaces.begin(); it_iface != _interfaces.end(); it_iface++)
				{
					for (std::list<ibrcommon::vaddress>::const_iterator it_addr = _destinations.begin(); it_addr != _destinations.end(); it_addr++)
					{
						try {
							msock.join(*it_addr, *it_iface);
						} catch (const ibrcommon::Exception&) {
							IBRCOMMON_LOGGER(error) << "can not join " << (*it_addr).toString() << " on " << (*it_iface).toString() << IBRCOMMON_LOGGER_ENDL;
						}
					}
				}
			} catch (std::bad_cast&) {
				throw ibrcommon::socket_exception("no multicast socket found");
			}

			// TODO: subscribe to NetLink events on our interfaces
		}

		void IPNDAgent::componentDown()
		{
			// TODO: unsubscribe to NetLink events

			// shutdown the send socket
			_send_socket.down();

			// shutdown the receive socket
			_recv_socket.down();

			stop();
			join();
		}

		void IPNDAgent::componentRun()
		{
			ibrcommon::TimeMeasurement tm;
			tm.start();

			while (true)
			{
				ibrcommon::socketset fds;

				struct timeval tv;

				// every second we want to transmit a discovery message, timeout of 1 seconds
				tv.tv_sec = 0;
				tv.tv_usec = 100000;

				try {
					// select on all bound sockets
					_recv_socket.select(&fds, NULL, NULL, &tv);

					// receive from all sockets
					for (ibrcommon::socketset::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
					{
						ibrcommon::multicastsocket &sock = dynamic_cast<ibrcommon::multicastsocket&>(**iter);

						char data[1500];
						ibrcommon::vaddress sender;
						DiscoveryAnnouncement announce(_version);

						std::list<DiscoveryService> ret_services;
						dtn::data::EID ret_source;

						int len = sock.recvfrom(data, 1500, 0, sender);

						if (len < 0) return;

						stringstream ss;
						ss.write(data, len);

						try {
							ss >> announce;

							if (announce.isShort())
							{
								// generate name with the sender address
								ret_source = dtn::data::EID("udp://[" + sender.address() + "]:4556");
							}
							else
							{
								ret_source = announce.getEID();
							}

							// get the list of services
							const std::list<DiscoveryService> &services = announce.getServices();

							if (services.empty())
							{
								// TODO: check format returned by sender.get()
								ret_services.push_back(dtn::net::DiscoveryService("tcpcl", "ip=" + sender.address() + ";port=4556;"));
							}

							// add all services to the return set
							for (std::list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); iter++) {
								const DiscoveryService &service = (*iter);

								// add source address if not set
								if ( (service.getParameters().find("port=") != std::string::npos) &&
										(service.getParameters().find("ip=") == std::string::npos) ) {
									// create a new service object
									// TODO: check format returned by sender.get()
									dtn::net::DiscoveryService ret_service(service.getName(), "ip=" + sender.address() + ";" + service.getParameters());

									// add service to the return set
									ret_services.push_back(ret_service);
								}
								else
								{
									// add service to the return set
									ret_services.push_back(service);
								}
							}

							// announce the received services
							received(ret_source, ret_services);
						} catch (const dtn::InvalidDataException&) {
						} catch (const ibrcommon::IOException&) {
						}

						yield();
					}
				} catch (const ibrcommon::vsocket_timeout&) { };

				// trigger timeout, if one second is elapsed
				tm.stop(); if (tm.getMilliseconds() > 1000)
				{
					tm.start();
					timeout();
				}
			}
		}

		void IPNDAgent::__cancellation()
		{
			// shutdown and interrupt the receiving thread
			_recv_socket.down();
		}

		const std::string IPNDAgent::getName() const
		{
			return "IPNDAgent";
		}
	}
}
