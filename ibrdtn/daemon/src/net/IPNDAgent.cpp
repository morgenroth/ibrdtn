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
#include <ibrcommon/net/vaddress.h>
#include <ibrcommon/link/LinkManager.h>

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
				IBRCOMMON_LOGGER_TAG("DiscoveryAgent", info) << "DTN2 compatibility mode" << IBRCOMMON_LOGGER_ENDL;
				_version = DiscoveryAnnouncement::DTND_IPDISCOVERY;
				break;
			};
		}

		IPNDAgent::~IPNDAgent()
		{
			_send_socket.destroy();
			_recv_socket.destroy();
		}

		void IPNDAgent::add(const ibrcommon::vaddress &address) {
			IBRCOMMON_LOGGER_TAG("DiscoveryAgent", info) << "listen to " << address.toString() << IBRCOMMON_LOGGER_ENDL;
			_destinations.push_back(address);
		}

		void IPNDAgent::bind(const ibrcommon::vinterface &net)
		{
			IBRCOMMON_LOGGER_TAG("DiscoveryAgent", info) << "add interface " << net.toString() << IBRCOMMON_LOGGER_ENDL;
			_interfaces.push_back(net);
		}

		void IPNDAgent::leave_interface(const ibrcommon::vinterface &iface)
		{
			ibrcommon::multicastsocket &msock = dynamic_cast<ibrcommon::multicastsocket&>(**_recv_socket.getAll().begin());

			for (std::list<ibrcommon::vaddress>::const_iterator it_addr = _destinations.begin(); it_addr != _destinations.end(); it_addr++)
			{
				try {
					msock.leave(*it_addr, iface);
				} catch (const ibrcommon::socket_raw_error &e) {
					if (e.error() != EADDRNOTAVAIL) {
						IBRCOMMON_LOGGER(error) << "leave failed on " << iface.toString() << "; " << e.what() << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const ibrcommon::socket_exception &e) {
					IBRCOMMON_LOGGER_DEBUG(10) << "can not leave " << (*it_addr).toString() << " on " << iface.toString() << IBRCOMMON_LOGGER_ENDL;
				} catch (const ibrcommon::Exception&) {
					IBRCOMMON_LOGGER(error) << "can not leave " << (*it_addr).toString() << " on " << iface.toString() << IBRCOMMON_LOGGER_ENDL;
				}
			}
		}

		void IPNDAgent::join_interface(const ibrcommon::vinterface &iface)
		{
			ibrcommon::multicastsocket &msock = dynamic_cast<ibrcommon::multicastsocket&>(**_recv_socket.getAll().begin());

			for (std::list<ibrcommon::vaddress>::const_iterator it_addr = _destinations.begin(); it_addr != _destinations.end(); it_addr++)
			{
				try {
					msock.join(*it_addr, iface);
				} catch (const ibrcommon::socket_raw_error &e) {
					if (e.error() != EADDRINUSE) {
						IBRCOMMON_LOGGER(error) << "join failed on " << iface.toString() << "; " << e.what() << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const ibrcommon::socket_exception &e) {
					IBRCOMMON_LOGGER_DEBUG(10) << "can not join " << (*it_addr).toString() << " on " << iface.toString() << "; " << e.what() << IBRCOMMON_LOGGER_ENDL;
				}
			}
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
				ibrcommon::udpsocket &sock = dynamic_cast<ibrcommon::udpsocket&>(**iter);

				// prevent broadcasting in the wrong address family
				if (addr.family() != sock.get_family()) continue;

				try {
					sock.sendto(data.c_str(), data.length(), 0, addr);
				} catch (const ibrcommon::socket_exception &e) {
					IBRCOMMON_LOGGER_DEBUG(5) << "can not send message to " << addr.toString() << " via " << sock.get_address().toString() << "/" << iface.toString() << "; socket exception: " << e.what() << IBRCOMMON_LOGGER_ENDL;
				}
			}
		}

		void IPNDAgent::sendAnnoucement(const uint16_t &sn, std::list<dtn::net::DiscoveryServiceProvider*> &providers)
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

		void IPNDAgent::eventNotify(const ibrcommon::LinkEvent &evt)
		{
			switch (evt.getAction())
			{
				case ibrcommon::LinkEvent::ACTION_ADDRESS_ADDED:
				{
					const ibrcommon::vaddress &bindaddr = evt.getAddress();
					ibrcommon::udpsocket *sock = new ibrcommon::udpsocket(bindaddr);
					try {
						sock->up();
						_send_socket.add(sock, evt.getInterface());
						join_interface(evt.getInterface());
					} catch (const ibrcommon::socket_exception&) {
						delete sock;
					}
					break;
				}

				case ibrcommon::LinkEvent::ACTION_ADDRESS_REMOVED:
				{
					ibrcommon::socketset socks = _send_socket.get(evt.getInterface());
					for (ibrcommon::socketset::iterator iter = socks.begin(); iter != socks.end(); iter++) {
						ibrcommon::udpsocket *sock = dynamic_cast<ibrcommon::udpsocket*>(*iter);
						if (sock->get_address().address() == evt.getAddress().address()) {
							// leave interfaces if a interface is totally gone
							if (socks.size() == 1) {
								leave_interface(evt.getInterface());
							}

							_send_socket.remove(sock);
							sock->down();
							delete sock;
							break;
						}
					}
					break;
				}

				case ibrcommon::LinkEvent::ACTION_LINK_DOWN:
				{
					leave_interface(evt.getInterface());

					ibrcommon::socketset socks = _send_socket.get(evt.getInterface());
					for (ibrcommon::socketset::iterator iter = socks.begin(); iter != socks.end(); iter++) {
						ibrcommon::udpsocket *sock = dynamic_cast<ibrcommon::udpsocket*>(*iter);
						_send_socket.remove(sock);
						sock->down();
						delete sock;
					}
					break;
				}

				default:
					break;
			}
		}

		void IPNDAgent::componentUp() throw ()
		{
			// setup the receive socket
			_recv_socket.up();

			// join multicast groups
			try {
				for (std::list<ibrcommon::vinterface>::const_iterator it_iface = _interfaces.begin(); it_iface != _interfaces.end(); it_iface++)
				{
					const ibrcommon::vinterface &iface = (*it_iface);

					// create sockets for all addresses on the interface
					std::list<ibrcommon::vaddress> addrs = iface.getAddresses();

					for (std::list<ibrcommon::vaddress>::iterator iter = addrs.begin(); iter != addrs.end(); iter++) {
						ibrcommon::vaddress &addr = (*iter);
						_send_socket.add(new ibrcommon::udpsocket(addr), iface);
					}

					// subscribe to NetLink events on our interfaces
					ibrcommon::LinkManager::getInstance().addEventListener(iface, this);

					// join to all multicast addresses on this interface
					join_interface(iface);
				}
			} catch (std::bad_cast&) {
				throw ibrcommon::socket_exception("no multicast socket found");
			}

			// setup all send sockets
			_send_socket.up();
		}

		void IPNDAgent::componentDown() throw ()
		{
			// unsubscribe to NetLink events
			ibrcommon::LinkManager::getInstance().removeEventListener(this);

			// shutdown the send socket
			_send_socket.destroy();

			// shutdown the receive socket
			_recv_socket.down();

			stop();
			join();
		}

		void IPNDAgent::componentRun() throw ()
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
								ret_services.push_back(dtn::net::DiscoveryService("tcpcl", "ip=" + sender.address() + ";port=4556;"));
							}

							// add all services to the return set
							for (std::list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); iter++) {
								const DiscoveryService &service = (*iter);

								// add source address if not set
								if ( (service.getParameters().find("port=") != std::string::npos) &&
										(service.getParameters().find("ip=") == std::string::npos) ) {
									// create a new service object
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
				} catch (const ibrcommon::vsocket_interrupt&) {
					return;
				} catch (const ibrcommon::vsocket_timeout&) { };

				// trigger timeout, if one second is elapsed
				tm.stop(); if (tm.getMilliseconds() > 1000)
				{
					tm.start();
					timeout();
				}
			}
		}

		void IPNDAgent::__cancellation() throw ()
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
