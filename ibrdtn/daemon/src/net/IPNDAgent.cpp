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
#include "net/P2PDialupEvent.h"
#include "core/BundleCore.h"
#include "core/EventDispatcher.h"

#include <ibrdtn/data/Exceptions.h>

#include <ibrcommon/Logger.h>
#include <ibrcommon/TimeMeasurement.h>
#include <ibrcommon/net/socket.h>
#include <ibrcommon/net/vaddress.h>
#include <ibrcommon/link/LinkManager.h>

#ifdef __WIN32__
#define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#define EADDRINUSE WSAEADDRINUSE
#endif

#include <sstream>
#include <string.h>
#include <typeinfo>
#include <time.h>

namespace dtn
{
	namespace net
	{
		const std::string IPNDAgent::TAG = "IPNDAgent";

		IPNDAgent::IPNDAgent(int port)
		 : DiscoveryAgent(dtn::daemon::Configuration::getInstance().getDiscovery()),
		   _version(DiscoveryAnnouncement::DISCO_VERSION_01), _state(false), _port(port)
		{
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
			_socket.destroy();
		}

		void IPNDAgent::add(const ibrcommon::vaddress &address) {
			IBRCOMMON_LOGGER_TAG("DiscoveryAgent", info) << "listen to " << address.toString() << IBRCOMMON_LOGGER_ENDL;
			_destinations.insert(address);
		}

		void IPNDAgent::bind(const ibrcommon::vinterface &net)
		{
			IBRCOMMON_LOGGER_TAG("DiscoveryAgent", info) << "add interface " << net.toString() << IBRCOMMON_LOGGER_ENDL;

			// add the interface to the stored set
			ibrcommon::MutexLock l(_interface_lock);

			// only add the interface once
			if (_interfaces.find(net) != _interfaces.end()) return;

			// store the new interface in the list of interfaces
			_interfaces.insert(net);
		}

		void IPNDAgent::join(const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr) throw ()
		{
			// create a multicast socket and bind to given addr
			ibrcommon::multicastsocket *msock = new ibrcommon::multicastsocket(addr);

			// add multicast socket to _socket
			_socket.add(msock, iface);

			// if we are in UP state
			if (_state) {
				try {
					// bring up
					msock->up();

					// listen to multicast addresses
					for (std::set<ibrcommon::vaddress>::const_iterator it_addr = _destinations.begin(); it_addr != _destinations.end(); ++it_addr)
					{
						try {
							msock->join(*it_addr, iface);
						} catch (const ibrcommon::socket_raw_error &e) {
							if (e.error() == EADDRINUSE) {
								// silence error
							} else if (e.error() == 92) {
								// silence error - protocol not available
							} else {
								IBRCOMMON_LOGGER_TAG(IPNDAgent::TAG, error) << "join failed on " << iface.toString() << "; " << e.what() << IBRCOMMON_LOGGER_ENDL;
							}
						} catch (const ibrcommon::socket_exception &e) {
							IBRCOMMON_LOGGER_DEBUG_TAG(IPNDAgent::TAG, 10) << "can not join " << (*it_addr).toString() << " on " << iface.toString() << "; " << e.what() << IBRCOMMON_LOGGER_ENDL;
						}
					}
				} catch (const ibrcommon::socket_exception &ex) {
					IBRCOMMON_LOGGER_TAG(IPNDAgent::TAG, error) << "failed to set-up socket on " << iface.toString() << ": " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				}
			}
		}

		void IPNDAgent::leave(const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr) throw ()
		{
			// get all sockets bound to the given interface
			ibrcommon::socketset ifsocks = _socket.get(iface);

			for (ibrcommon::socketset::iterator it = ifsocks.begin(); it != ifsocks.end(); ++it)
			{
				ibrcommon::multicastsocket *msock = dynamic_cast<ibrcommon::multicastsocket*>(*it);
				if (msock == NULL) continue;

				if (msock->get_address() == addr) {
					// remove the socket
					_socket.remove(msock);

					// shutdown the socket
					try {
						msock->down();
					} catch (const ibrcommon::socket_exception &ex) {
						IBRCOMMON_LOGGER_DEBUG_TAG(IPNDAgent::TAG, 10) << "leave failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}

					// delete the socket
					delete msock;

					return;
				}
			}
		}

		void IPNDAgent::leave(const ibrcommon::vinterface &iface) throw ()
		{
			// get all sockets bound to the given interface
			ibrcommon::socketset ifsocks = _socket.get(iface);

			for (ibrcommon::socketset::iterator it = ifsocks.begin(); it != ifsocks.end(); ++it)
			{
				ibrcommon::multicastsocket *msock = dynamic_cast<ibrcommon::multicastsocket*>(*it);
				if (msock == NULL) continue;

				// remove the socket
				_socket.remove(msock);

				// shutdown the socket
				try {
					msock->down();
				} catch (const ibrcommon::socket_exception &ex) {
					IBRCOMMON_LOGGER_DEBUG_TAG(IPNDAgent::TAG, 10) << "leave failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				}

				// delete the socket
				delete msock;
			}
		}

		void IPNDAgent::join(const ibrcommon::vinterface &iface) throw ()
		{
			std::list<ibrcommon::vaddress> addrs = iface.getAddresses();

			for (std::list<ibrcommon::vaddress>::const_iterator it = addrs.begin(); it != addrs.end(); ++it)
			{
				ibrcommon::vaddress addr = (*it);
				addr.setService(_port);
				join(iface, addr);
			}
		}

		void IPNDAgent::send(const DiscoveryAnnouncement &a, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr)
		{
			// serialize announcement
			stringstream ss; ss << a;
			const std::string data = ss.str();

			// get all sockets for the given interface
			ibrcommon::socketset fds = _socket.get(iface);

			// send out discovery announcements on all bound sockets
			// (hopefully only one per interface)
			for (ibrcommon::socketset::const_iterator iter = fds.begin(); iter != fds.end(); ++iter)
			{
				try {
					ibrcommon::udpsocket &sock = dynamic_cast<ibrcommon::udpsocket&>(**iter);

					try {
						// prevent broadcasting in the wrong address family
						if (addr.family() != sock.get_family()) continue;

						sock.sendto(data.c_str(), data.length(), 0, addr);
					} catch (const ibrcommon::socket_exception &e) {
						IBRCOMMON_LOGGER_DEBUG_TAG(IPNDAgent::TAG, 5) << "can not send message to " << addr.toString() << " via " << sock.get_address().toString() << "/" << iface.toString() << "; socket exception: " << e.what() << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::vaddress::address_exception &ex) {
						IBRCOMMON_LOGGER_TAG(IPNDAgent::TAG, warning) << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const std::bad_cast&) {
					IBRCOMMON_LOGGER_TAG(IPNDAgent::TAG, error) << "Socket for sending isn't a udpsocket." << IBRCOMMON_LOGGER_ENDL;
				}
			}
		}

		void IPNDAgent::sendAnnoucement(const uint16_t &sn, std::list<dtn::net::DiscoveryServiceProvider*> &providers)
		{
			DiscoveryAnnouncement announcement(_version, dtn::core::BundleCore::local);

			// set sequencenumber
			announcement.setSequencenumber(sn);

			ibrcommon::MutexLock l(_interface_lock);

			for (std::set<ibrcommon::vinterface>::const_iterator it_iface = _interfaces.begin(); it_iface != _interfaces.end(); ++it_iface)
			{
				const ibrcommon::vinterface &iface = (*it_iface);

				// clear all services
				announcement.clearServices();

				if (!_config.shortbeacon())
				{
					// add services
					for (std::list<DiscoveryServiceProvider*>::iterator iter = providers.begin(); iter != providers.end(); ++iter)
					{
						DiscoveryServiceProvider &provider = (**iter);

						try {
							// update service information
							provider.update(iface, announcement);
						} catch (const dtn::net::DiscoveryServiceProvider::NoServiceHereException&) {

						}
					}
				}

				for (std::set<ibrcommon::vaddress>::iterator iter = _destinations.begin(); iter != _destinations.end(); ++iter) {
					send(announcement, iface, (*iter));
				}
			}
		}

		void IPNDAgent::raiseEvent(const Event *evt) throw ()
		{
			try {
				const dtn::net::P2PDialupEvent &dialup = dynamic_cast<const dtn::net::P2PDialupEvent&>(*evt);

				switch (dialup.type)
				{
					case dtn::net::P2PDialupEvent::INTERFACE_UP:
					{
						// add the interface to the stored set
						{
							ibrcommon::MutexLock l(_interface_lock);

							// only add the interface once
							if (_interfaces.find(dialup.iface) != _interfaces.end()) break;

							// store the new interface in the list of interfaces
							_interfaces.insert(dialup.iface);
						}

						// subscribe to NetLink events on our interfaces
						ibrcommon::LinkManager::getInstance().addEventListener(dialup.iface, this);

						// join to all multicast addresses on this interface
						join(dialup.iface);
						break;
					}

					case dtn::net::P2PDialupEvent::INTERFACE_DOWN:
					{
						// check if the interface is bound by us
						{
							ibrcommon::MutexLock l(_interface_lock);

							// only remove the interface if it exists
							if (_interfaces.find(dialup.iface) == _interfaces.end()) break;

							// remove the interface from the stored set
							_interfaces.erase(dialup.iface);
						}

						// subscribe to NetLink events on our interfaces
						ibrcommon::LinkManager::getInstance().removeEventListener(dialup.iface, this);

						// leave the multicast groups on the interface
						leave(dialup.iface);
						break;
					}
				}
			} catch (std::bad_cast&) {

			}
		}

		void IPNDAgent::eventNotify(const ibrcommon::LinkEvent &evt)
		{
			// check first if we are really bound to the interface
			{
				ibrcommon::MutexLock l(_interface_lock);
				if (_interfaces.find(evt.getInterface()) == _interfaces.end()) return;
			}

			switch (evt.getAction())
			{
				case ibrcommon::LinkEvent::ACTION_ADDRESS_ADDED:
				{
					ibrcommon::vaddress addr = evt.getAddress();
					addr.setService(_port);
					join(evt.getInterface(), addr);
					break;
				}

				case ibrcommon::LinkEvent::ACTION_ADDRESS_REMOVED:
				{
					ibrcommon::vaddress addr = evt.getAddress();
					addr.setService(_port);
					leave(evt.getInterface(), addr);
					break;
				}

				case ibrcommon::LinkEvent::ACTION_LINK_DOWN:
				{
					// leave the multicast groups on the interface
					leave(evt.getInterface());
					break;
				}

				default:
					break;
			}
		}

		void IPNDAgent::componentUp() throw ()
		{
			// routine checked for throw() on 15.02.2013

			try {
				// setup the sockets
				_socket.up();

				// set state to UP
				_state = true;
			} catch (const ibrcommon::socket_exception &ex) {
				IBRCOMMON_LOGGER_TAG(IPNDAgent::TAG, error) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			// join multicast groups
			try {
				ibrcommon::MutexLock l(_interface_lock);

				for (std::set<ibrcommon::vinterface>::const_iterator it_iface = _interfaces.begin(); it_iface != _interfaces.end(); ++it_iface)
				{
					const ibrcommon::vinterface &iface = (*it_iface);

					// subscribe to NetLink events on our interfaces
					ibrcommon::LinkManager::getInstance().addEventListener(iface, this);

					// join to all multicast addresses on this interface
					join(iface);
				}
			} catch (std::bad_cast&) {
				throw ibrcommon::socket_exception("no multicast socket found");
			}

			// listen on P2P dial-up events
			dtn::core::EventDispatcher<dtn::net::P2PDialupEvent>::add(this);
		}

		void IPNDAgent::componentDown() throw ()
		{
			// un-listen on P2P dial-up events
			dtn::core::EventDispatcher<dtn::net::P2PDialupEvent>::remove(this);

			// unsubscribe to NetLink events
			ibrcommon::LinkManager::getInstance().removeEventListener(this);

			// mark the send socket as down
			_state = false;

			// shutdown the receive socket
			_socket.down();

			ibrcommon::JoinableThread::stop();
			ibrcommon::JoinableThread::join();
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
					_socket.select(&fds, NULL, NULL, &tv);

					// receive from all sockets
					for (ibrcommon::socketset::const_iterator iter = fds.begin(); iter != fds.end(); ++iter)
					{
						ibrcommon::multicastsocket &sock = dynamic_cast<ibrcommon::multicastsocket&>(**iter);

						char data[1500];
						ibrcommon::vaddress sender;
						DiscoveryAnnouncement announce(_version);

						std::list<DiscoveryService> ret_services;
						dtn::data::EID ret_source;

						ssize_t len = sock.recvfrom(data, 1500, 0, sender);

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

							if (services.empty() && announce.isShort())
							{
								ret_services.push_back(dtn::net::DiscoveryService("tcpcl", "ip=" + sender.address() + ";port=4556;"));
							}

							// add all services to the return set
							for (std::list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); ++iter) {
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
				tm.stop(); if (tm.getMilliseconds() > 1000.0)
				{
					tm.start();
					timeout();
				}
			}
		}

		void IPNDAgent::__cancellation() throw ()
		{
			// shutdown and interrupt the receiving thread
			_socket.down();
		}

		const std::string IPNDAgent::getName() const
		{
			return "IPNDAgent";
		}
	}
}
