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

#include "net/IPNDAgent.h"
#include "core/BundleCore.h"
#include <ibrdtn/data/Exceptions.h>
#include <sstream>
#include <string.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/TimeMeasurement.h>
#include "Configuration.h"
#include <typeinfo>
#include <time.h>

namespace dtn
{
	namespace net
	{
		IPNDAgent::IPNDAgent(int port)
		 : DiscoveryAgent(dtn::daemon::Configuration::getInstance().getDiscovery()),
		   _version(DiscoveryAnnouncement::DISCO_VERSION_01), _port(port), _send_socket(true)
		{
			// multicast addresses should be usable more than once.
			_recv_socket.set(ibrcommon::vsocket::VSOCKET_REUSEADDR);

			// enable multicast communication
			_recv_socket.set(ibrcommon::vsocket::VSOCKET_MULTICAST);
			_send_socket.set(ibrcommon::vsocket::VSOCKET_MULTICAST);

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
			IBRCOMMON_LOGGER(info) << "DiscoveryAgent: listen to [" << address.toString() << "]:" << _port << IBRCOMMON_LOGGER_ENDL;
			_destinations.push_back(address);
		}

		void IPNDAgent::bind(const ibrcommon::vinterface &net)
		{
			IBRCOMMON_LOGGER(info) << "DiscoveryAgent: add interface " << net.toString() << IBRCOMMON_LOGGER_ENDL;
			_interfaces.push_back(net);
		}

		void IPNDAgent::send(const DiscoveryAnnouncement &a, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr, const unsigned int port)
		{
			// serialize announcement
			stringstream ss; ss << a;
			const std::string data = ss.str();

			std::list<int> fds = _send_socket.get(iface);
			for (std::list<int>::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
			{
				try {
					int flags = 0;

					struct addrinfo hints, *ainfo;
					memset(&hints, 0, sizeof hints);

					hints.ai_socktype = SOCK_DGRAM;
					ainfo = addr.addrinfo(&hints, port);

					::sendto(*iter, data.c_str(), data.length(), flags, ainfo->ai_addr, ainfo->ai_addrlen);

					freeaddrinfo(ainfo);
				} catch (const ibrcommon::vsocket_exception&) {
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
					send(announcement, iface, (*iter), _port);
				}
			}
		}

		void IPNDAgent::eventNotify(const ibrcommon::LinkManagerEvent &evt)
		{
			if (evt.getType() == ibrcommon::LinkManagerEvent::EVENT_ADDRESS_ADDED)
			{
				for (std::list<ibrcommon::vaddress>::iterator iter = _destinations.begin(); iter != _destinations.end(); iter++) {
					_recv_socket.join(*iter, evt.getInterface());
				}
			}
		}

		void IPNDAgent::componentUp()
		{
			// bind to receive socket
			_recv_socket.bind(_port, SOCK_DGRAM);

			// bind to all added interfaces for sending
			for (std::list<ibrcommon::vinterface>::const_iterator iter = _interfaces.begin(); iter != _interfaces.end(); iter++)
			{
				try {
					const ibrcommon::vinterface &iface = *iter;
					if (!iface.empty())
					{
						// create one send socket for each interface
						_send_socket.bind(iface, 0, SOCK_DGRAM);
					}
				} catch (const ibrcommon::Exception &ex) {
					IBRCOMMON_LOGGER(error) << "[IPND] bind failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				}
			}

			// join multicast groups
			for (std::list<ibrcommon::vinterface>::const_iterator it_iface = _interfaces.begin(); it_iface != _interfaces.end(); it_iface++)
			{
				for (std::list<ibrcommon::vaddress>::const_iterator it_addr = _destinations.begin(); it_addr != _destinations.end(); it_addr++)
				{
					_recv_socket.join(*it_addr, *it_iface);
				}
			}

			// set this socket as listener to socket events
			_send_socket.setEventCallback(this);
		}

		void IPNDAgent::componentDown()
		{
			// unset this socket as listener to socket events
			_send_socket.setEventCallback(NULL);

			// shutdown the sockets
			_recv_socket.close();
			_send_socket.close();

			stop();
			join();
		}

		void IPNDAgent::componentRun()
		{
			ibrcommon::TimeMeasurement tm;
			tm.start();

			while (true)
			{
				std::list<int> fds;

				struct timeval tv;

				// every second we want to transmit a discovery message, timeout of 1 seconds
				tv.tv_sec = 0;
				tv.tv_usec = 100000;

				try {
					// select on all bound sockets
					_recv_socket.select(fds, &tv);

					// receive from all sockets
					for (std::list<int>::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
					{
						char data[1500];
						std::string sender;
						DiscoveryAnnouncement announce(_version);

						int len = ibrcommon::recvfrom(*iter, data, 1500, sender);

						if (len < 0) return;

						stringstream ss;
						ss.write(data, len);

						try {
							ss >> announce;

							if (announce.isShort())
							{
								// generate name with the sender address
								dtn::data::EID gen_eid("ip://" + sender);
								announce.setEID(gen_eid);
							}

							if (announce.getServices().empty())
							{
								announce.addService(dtn::net::DiscoveryService("tcpcl", "ip=" + sender + ";port=4556;"));
							}

							received(announce);
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
			// interrupt the receiving thread
			_recv_socket.close();
		}

		const std::string IPNDAgent::getName() const
		{
			return "IPNDAgent";
		}
	}
}
