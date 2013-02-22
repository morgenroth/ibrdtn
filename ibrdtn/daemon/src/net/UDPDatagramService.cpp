/*
 * UDPDatagramService.cpp
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

#include "net/UDPDatagramService.h"
#include <ibrdtn/utils/Utils.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/net/socket.h>
#include <vector>
#include <string.h>

namespace dtn
{
	namespace net
	{
		UDPDatagramService::UDPDatagramService(const ibrcommon::vinterface &iface, int port, size_t mtu)
		 : _iface(iface), _bind_port(port)
		{
			// set connection parameters
			_params.max_msg_length = mtu - 2;	// minus 2 bytes because we encode seqno and flags into 2 bytes
			_params.max_seq_numbers = 8;		// seqno 0..7
			_params.flowcontrol = DatagramService::FLOW_STOPNWAIT;
			_params.initial_timeout = 50;		// initial timeout 50ms
			_params.seq_check = true;
		}

		UDPDatagramService::~UDPDatagramService()
		{
			// delete all sockets
			_vsocket.destroy();
		}

		/**
		 * Bind to the local socket.
		 * @throw If the bind fails, an DatagramException is thrown.
		 */
		void UDPDatagramService::bind() throw (DatagramException)
		{
			// delete all sockets
			_vsocket.destroy();

			try {
				if (_iface.empty()) {
					// bind socket to interface
					_vsocket.add(new ibrcommon::udpsocket(_bind_port));
				} else {
					// create sockets for all addresses on the interface
					std::list<ibrcommon::vaddress> addrs = _iface.getAddresses();

					// convert the port into a string
					std::stringstream ss; ss << _bind_port;

					for (std::list<ibrcommon::vaddress>::iterator iter = addrs.begin(); iter != addrs.end(); iter++) {
						ibrcommon::vaddress &addr = (*iter);

						// handle the addresses according to their family
						switch (addr.family()) {
						case AF_INET:
						case AF_INET6:
							addr.setService(ss.str());
							_vsocket.add(new ibrcommon::udpsocket(addr), _iface);
							break;
						default:
							break;
						}
					}
				}
			} catch (const ibrcommon::Exception&) {
				throw DatagramException("bind failed");
			}

			// setup socket operations
			_vsocket.up();
		}

		/**
		 * Shutdown the socket. Unblock all calls on the socket (recv, send, etc.)
		 */
		void UDPDatagramService::shutdown()
		{
			// abort socket operations
			_vsocket.down();
		}

		/**
		 * Send the payload as datagram to a defined destination
		 * @param address The destination address encoded as string.
		 * @param buf The buffer to send.
		 * @param length The number of available bytes in the buffer.
		 */
		void UDPDatagramService::send(const char &type, const char &flags, const unsigned int &seqno, const std::string &identifier, const char *buf, size_t length) throw (DatagramException)
		{
			try {
				char tmp[length + 2];

				// add a 2-byte header - type of frame first
				tmp[0] = type;

				// flags (4-bit) + seqno (4-bit)
				tmp[1] = (0xf0 & (flags << 4)) | (0x0f & seqno);

				// copy payload to the new buffer
				::memcpy(&tmp[2], buf, length);

				ibrcommon::vaddress destination;

				// decode address
				UDPDatagramService::decode(identifier, destination);

				IBRCOMMON_LOGGER_DEBUG_TAG("UDPDatagramService", 20) << "send() type: " << std::hex << (int)type << "; flags: " << std::hex << (int)flags << "; seqno: " << seqno << "; address: " << destination.toString() << IBRCOMMON_LOGGER_ENDL;

				// create vaddress
				ibrcommon::socketset sockset = _vsocket.getAll();
				if (sockset.size() > 0) {
					try {
						ibrcommon::udpsocket &sock = dynamic_cast<ibrcommon::udpsocket&>(**sockset.begin());
						sock.sendto(tmp, length + 2, 0, destination);
					} catch (const std::bad_cast&) {

					}
				}
			} catch (const ibrcommon::Exception&) {
				throw DatagramException("send failed");
			}
		}

		/**
		 * Send the payload as datagram to all neighbors (broadcast)
		 * @param buf The buffer to send.
		 * @param length The number of available bytes in the buffer.
		 */
		void UDPDatagramService::send(const char &type, const char &flags, const unsigned int &seqno, const char *buf, size_t length) throw (DatagramException)
		{
			try {
				char tmp[length];

				// add a 2-byte header - type of frame first
				tmp[0] = type;

				// flags (4-bit) + seqno (4-bit)
				tmp[1] = (0xf0 & (flags << 4)) | (0x0f & seqno);

				// copy payload to the new buffer
				::memcpy(&tmp[2], buf, length);

				IBRCOMMON_LOGGER_DEBUG_TAG("UDPDatagramService", 20) << "send() type: " << std::hex << (int)type << "; flags: " << std::hex << (int)flags << "; seqno: " << std::dec << seqno << IBRCOMMON_LOGGER_ENDL;

				// create broadcast address
				const ibrcommon::vaddress broadcast("ff02::1", BROADCAST_PORT, AF_INET6);

				ibrcommon::socketset sockset = _vsocket.getAll();
				if (sockset.size() > 0) {
					try {
						ibrcommon::udpsocket &sock = dynamic_cast<ibrcommon::udpsocket&>(**sockset.begin());
						sock.sendto(tmp, length + 2, 0, broadcast);
					} catch (const std::bad_cast&) {

					}
				}
			} catch (const ibrcommon::Exception&) {
				throw DatagramException("send failed");
			}
		}

		/**
		 * Receive an incoming datagram.
		 * @param buf A buffer to catch the incoming data.
		 * @param length The length of the buffer.
		 * @param address A buffer for the address of the sender.
		 * @throw If the receive call failed for any reason, an DatagramException is thrown.
		 * @return The number of received bytes.
		 */
		size_t UDPDatagramService::recvfrom(char *buf, size_t length, char &type, char &flags, unsigned int &seqno, std::string &address) throw (DatagramException)
		{
			try {
				ibrcommon::socketset readfds;
				_vsocket.select(&readfds, NULL, NULL, NULL);

				for (ibrcommon::socketset::iterator iter = readfds.begin(); iter != readfds.end(); iter++) {
					try {
						ibrcommon::udpsocket &sock = dynamic_cast<ibrcommon::udpsocket&>(**iter);

						char tmp[length + 2];
						ibrcommon::vaddress peeraddr;
						size_t ret = sock.recvfrom(tmp, length + 2, 0, peeraddr);

						// first byte if the type
						type = tmp[0];

						// second byte if flags (4-bit) + seqno (4-bit)
						flags = 0x0f & (tmp[1] >> 4);
						seqno = 0x0f & tmp[1];

						// return the encoded format
						address = UDPDatagramService::encode(peeraddr);

						// copy payload to the destination buffer
						::memcpy(buf, &tmp[2], ret - 2);

						IBRCOMMON_LOGGER_DEBUG_TAG("UDPDatagramService", 20) << "recvfrom() type: " << std::hex << (int)type << "; flags: " << std::hex << (int)flags << "; seqno: " << seqno << "; address: " << peeraddr.toString() << IBRCOMMON_LOGGER_ENDL;

						return ret - 2;
					} catch (const std::bad_cast&) {

					}
				}
			} catch (const ibrcommon::Exception&) {
				throw DatagramException("receive failed");
			}
		}

		/**
		 * Get the tag for this service used in discovery messages.
		 * @return The tag as string.
		 */
		const std::string UDPDatagramService::getServiceTag() const
		{
			return "dgram:udp";
		}

		/**
		 * Get the service description for this convergence layer. This
		 * data is used to contact this node.
		 * @return The service description as string.
		 */
		const std::string UDPDatagramService::getServiceDescription() const
		{
			std::string address;

			// get all addresses
			std::list<ibrcommon::vaddress> addrs = _iface.getAddresses();

			for (std::list<ibrcommon::vaddress>::iterator iter = addrs.begin(); iter != addrs.end(); iter++) {
				ibrcommon::vaddress &addr = (*iter);

				try {
					// handle the addresses according to their family
					switch (addr.family()) {
					case AF_INET:
					case AF_INET6:
						return UDPDatagramService::encode(addr, _bind_port);
						break;
					default:
						break;
					}
				} catch (const ibrcommon::vaddress::address_exception &ex) {
					IBRCOMMON_LOGGER_DEBUG_TAG("UDPDatagramService", 25) << ex.what() << IBRCOMMON_LOGGER_ENDL;
				}
			}

			// no addresses available, return empty string
			return "";
		}

		/**
		 * The used interface as vinterface object.
		 * @return A vinterface object.
		 */
		const ibrcommon::vinterface& UDPDatagramService::getInterface() const
		{
			return _iface;
		}

		/**
		 * The protocol identifier for this type of service.
		 * @return
		 */
		dtn::core::Node::Protocol UDPDatagramService::getProtocol() const
		{
			return dtn::core::Node::CONN_DGRAM_UDP;
		}

		const DatagramService::Parameter& UDPDatagramService::getParameter() const
		{
			return _params;
		}

		const std::string UDPDatagramService::encode(const ibrcommon::vaddress &address, const int port)
		{
			std::stringstream ss;
			ss << "ip=" << address.address() << ";port=";

			if (port == 0) ss << address.service();
			else ss << port;

			ss << ";";
			return ss.str();
		}

		void UDPDatagramService::decode(const std::string &identifier, ibrcommon::vaddress &address)
		{
			std::string addr;
			std::string port;

			// parse parameters
			std::vector<string> parameters = dtn::utils::Utils::tokenize(";", identifier);
			std::vector<string>::const_iterator param_iter = parameters.begin();

			while (param_iter != parameters.end())
			{
				std::vector<string> p = dtn::utils::Utils::tokenize("=", (*param_iter));

				if (p[0].compare("ip") == 0)
				{
					addr = p[1];
				}

				if (p[0].compare("port") == 0)
				{
					port = p[1];
				}

				param_iter++;
			}

			address = ibrcommon::vaddress(addr, port);
		}
	} /* namespace net */
} /* namespace dtn */
