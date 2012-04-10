/*
 * IPNDAgent.cpp
 *
 *  Created on: 14.09.2009
 *      Author: morgenro
 */

#include "net/IPNDAgent.h"
#include "core/BundleCore.h"
#include <ibrdtn/data/Exceptions.h>
#include <sstream>
#include <string.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/net/MulticastSocket.h>
#include <ibrcommon/TimeMeasurement.h>
#include "Configuration.h"
#include <typeinfo>
#include <time.h>

namespace dtn
{
	namespace net
	{
		IPNDAgent::IPNDAgent(int port, const ibrcommon::vaddress &address)
		 : DiscoveryAgent(dtn::daemon::Configuration::getInstance().getDiscovery()),
		   _version(DiscoveryAnnouncement::DISCO_VERSION_01), _destination(address), _port(port)
		{
			// broadcast addresses should be usable more than once.
			_socket.set(ibrcommon::vsocket::VSOCKET_REUSEADDR);

			if (_destination.isMulticast())
			{
				IBRCOMMON_LOGGER(info) << "DiscoveryAgent: multicast mode " << address.toString() << ":" << port << IBRCOMMON_LOGGER_ENDL;
				_socket.set(ibrcommon::vsocket::VSOCKET_MULTICAST);
			}
			else
			{
				IBRCOMMON_LOGGER(info) << "DiscoveryAgent: broadcast mode " << address.toString() << ":" << port << IBRCOMMON_LOGGER_ENDL;
				_socket.set(ibrcommon::vsocket::VSOCKET_BROADCAST);
			}

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

		void IPNDAgent::bind(const ibrcommon::vinterface &net)
		{
			IBRCOMMON_LOGGER(info) << "DiscoveryAgent: bind to interface " << net.toString() << IBRCOMMON_LOGGER_ENDL;
			_interfaces.push_back(net);
		}

		void IPNDAgent::send(const DiscoveryAnnouncement &a, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr, const unsigned int port)
		{
			// serialize announcement
			stringstream ss; ss << a;
			const std::string data = ss.str();

			std::list<int> fds = _socket.get(iface);
			for (std::list<int>::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
			{
				try {
					size_t ret = 0;
					int flags = 0;

					struct addrinfo hints, *ainfo;
					memset(&hints, 0, sizeof hints);

					hints.ai_socktype = SOCK_DGRAM;
					ainfo = addr.addrinfo(&hints, port);

					ret = sendto(*iter, data.c_str(), data.length(), flags, ainfo->ai_addr, ainfo->ai_addrlen);

					freeaddrinfo(ainfo);
				} catch (const ibrcommon::vsocket_exception&) {
					IBRCOMMON_LOGGER_DEBUG(5) << "can not send message to " << addr.toString() << IBRCOMMON_LOGGER_ENDL;
				}
			}
		}

		void IPNDAgent::sendAnnoucement(const u_int16_t &sn, std::list<DiscoveryService> &services)
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
					for (std::list<DiscoveryService>::iterator iter = services.begin(); iter != services.end(); iter++)
					{
						DiscoveryService &service = (*iter);

						try {
							// update service information
							service.update(iface);

							// add service to discovery message
							announcement.addService(service);
						} catch (const dtn::net::DiscoveryServiceProvider::NoServiceHereException&) {

						}
					}
				}

				send(announcement, iface, _destination, _port);
			}
		}

		void IPNDAgent::eventNotify(const ibrcommon::LinkManagerEvent &evt)
		{
			if (evt.getType() == ibrcommon::LinkManagerEvent::EVENT_ADDRESS_ADDED)
			{
				if (_destination.isMulticast())
				{
					ibrcommon::MulticastSocket ms(_fd);
					ms.joinGroup(_destination, evt.getInterface());
				}
			}
		}

		void IPNDAgent::componentUp()
		{
			// create one socket for each interface
			for (std::list<ibrcommon::vinterface>::const_iterator iter = _interfaces.begin(); iter != _interfaces.end(); iter++)
			{
				try {
					const ibrcommon::vinterface &iface = *iter;
					if (!iface.empty())
					{
						_socket.bind(iface, 0, SOCK_DGRAM);
					}
				} catch (const ibrcommon::Exception &ex) {
					IBRCOMMON_LOGGER(error) << "[IPND] bind failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				}
			}

			try {
				// bind on ALL interfaces
				_fd = _socket.bind(_port, SOCK_DGRAM);

				// only if the destination is a multicast address
				if (_destination.isMulticast())
				{
					// enable multicasting on the socket
					ibrcommon::MulticastSocket ms(_fd);

					for (std::list<ibrcommon::vinterface>::const_iterator iter = _interfaces.begin(); iter != _interfaces.end(); iter++)
					{
						ms.joinGroup(_destination, *iter);
					}
				}
			} catch (const ibrcommon::Exception &ex) {
				IBRCOMMON_LOGGER(error) << "[IPND] bind on broadcast address failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			// set this socket as listener to socket events
			_socket.setEventCallback(this);
		}

		void IPNDAgent::componentDown()
		{
			// unset this socket as listener to socket events
			_socket.setEventCallback(NULL);

			// shutdown the sockets
			_socket.shutdown();

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
					_socket.select(fds, &tv);

					// receive from all sockets
					for (std::list<int>::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
					{
						char data[1500];
						std::string sender;
						DiscoveryAnnouncement announce(_version);

						int len = ibrcommon::recvfrom(*iter, data, 1500, sender);

						if (announce.isShort())
						{
							// TODO: generate name with the sender address
						}

						if (announce.getServices().empty())
						{
							announce.addService(dtn::net::DiscoveryService("tcpcl", "ip=" + sender + ";port=4556;"));
						}

						if (len < 0) return;

						stringstream ss;
						ss.write(data, len);

						try {
							ss >> announce;
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
			_socket.close();
		}

		const std::string IPNDAgent::getName() const
		{
			return "IPNDAgent";
		}
	}
}
