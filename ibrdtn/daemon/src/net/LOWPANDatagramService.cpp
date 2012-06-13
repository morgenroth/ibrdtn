/*
 * LOWPANDatagramService.cpp
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

#include "net/LOWPANDatagramService.h"
#include <ibrdtn/utils/Utils.h>

#include <ibrcommon/net/lowpanstream.h>
#include <ibrcommon/net/UnicastSocketLowpan.h>

#include <ibrcommon/Logger.h>

#include <string.h>

#define EXTENDED_MASK	0x40
#define SEQ_NUM_MASK	0x07

namespace dtn
{
	namespace net
	{
		LOWPANDatagramService::LOWPANDatagramService(const ibrcommon::vinterface &iface, int panid)
		 : _panid(panid), _iface(iface)
		{
			// set connection parameters
			_params.max_msg_length = 111;
			_params.max_seq_numbers = 8;
			_params.flowcontrol = DatagramConnectionParameter::FLOW_NONE;

			_socket = new ibrcommon::UnicastSocketLowpan();
		}

		LOWPANDatagramService::~LOWPANDatagramService()
		{
			delete _socket;
		}

		/**
		 * Bind to the local socket.
		 * @throw If the bind fails, an DatagramException is thrown.
		 */
		void LOWPANDatagramService::bind() throw (DatagramException)
		{
			try {
				// bind socket to interface
				ibrcommon::UnicastSocketLowpan &sock = dynamic_cast<ibrcommon::UnicastSocketLowpan&>(*_socket);
				sock.bind(_panid, _iface);
			} catch (const std::bad_cast&) {
				throw DatagramException("bind failed");
			} catch (const ibrcommon::Exception&) {
				throw DatagramException("bind failed");
			}
		}

		/**
		 * Shutdown the socket. Unblock all calls on the socket (recv, send, etc.)
		 */
		void LOWPANDatagramService::shutdown()
		{
			// abort socket operations
			_socket->shutdown();
		}

		/**
		 * Send the payload as datagram to a defined destination
		 * @param address The destination address encoded as string.
		 * @param buf The buffer to send.
		 * @param length The number of available bytes in the buffer.
		 */
		void LOWPANDatagramService::send(const char &type, const char &flags, const unsigned int &seqno, const std::string &identifier, const char *buf, size_t length) throw (DatagramException)
		{
			try {
				// Add own address at the end
				struct sockaddr_ieee802154 sockaddr;
				unsigned short local_addr;

				if (length > _params.max_msg_length)
				{
					IBRCOMMON_LOGGER(error) << "LOWPANConvergenceLayer::send buffer to big to be transferred (" << length << ")."<< IBRCOMMON_LOGGER_ENDL;
					throw DatagramException("send failed - buffer to big to be transferred");
				}

				// get the local address
				_socket->getAddress(&sockaddr.addr, _iface);
				local_addr = sockaddr.addr.short_addr;

				// decode destination address
				uint16_t addr = 0;
				int panid = 0;
				size_t end_of_payload = length + 1;
				LOWPANDatagramService::decode(identifier, addr, panid);

				// get a lowpan peer
				ibrcommon::lowpansocket::peer p = _socket->getPeer(addr, panid);

				// buffer for the datagram plus local address
				char tmp[length + 4];

				// encode the flags (4-bit) + seqno (4-bit)
				tmp[0] = (0x30 & (flags << 4)) | (0x0f & seqno);

				// Need to encode flags, type and seqno into the message
				if (type == DatagramConvergenceLayer::HEADER_SEGMENT)
				{
					// copy payload to the new buffer
					::memcpy(&tmp[1], buf, length);
				}
				else
				{
					// set datagram to extended
					tmp[0] |= EXTENDED_MASK;

					// encode the type into the second byte
					tmp[1] = type;

					// copy payload to the new buffer
					::memcpy(&tmp[2], buf, length);

					// payload will be one byte longer with extended header
					end_of_payload++;
				}

				// Add own address at the end
				memcpy(&tmp[end_of_payload], &local_addr, 2);

				// send converted line
				if (p.send(buf, end_of_payload + 2) == -1)
				{
					// CL is busy
					throw DatagramException("send on socket failed");
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
		void LOWPANDatagramService::send(const char &type, const char &flags, const unsigned int &seqno, const char *buf, size_t length) throw (DatagramException)
		{
			try {
				// Add own address at the end
				struct sockaddr_ieee802154 sockaddr;
				unsigned short local_addr;

				if (length > _params.max_msg_length)
				{
					IBRCOMMON_LOGGER(error) << "LOWPANConvergenceLayer::send buffer to big to be transferred (" << length << ")."<< IBRCOMMON_LOGGER_ENDL;
					throw DatagramException("send failed - buffer to big to be transferred");
				}

				// get the local address
				_socket->getAddress(&sockaddr.addr, _iface);
				local_addr = sockaddr.addr.short_addr;

				// get a lowpan peer
				ibrcommon::lowpansocket::peer p = _socket->getPeer(BROADCAST_ADDR, _panid);

				// extend the buffer if the len is zero (ACKs)
				if (length == 0) length++;

				// buffer for the datagram plus local address
				char tmp[length + 4];

				// encode header: 2-bit unused, flags (2-bit) + seqno (4-bit)
				tmp[0] = (0x30 & (flags << 4)) | (0x0f & seqno);

				// set datagram to extended
				tmp[0] |= EXTENDED_MASK;

				// encode the type into the second byte
				tmp[1] = type;

				// copy payload to the new buffer
				::memcpy(&tmp[2], buf, length);

				// Add own address at the end
				memcpy(&tmp[length+2], &local_addr, 2);

				// send converted line
				if (p.send(buf, length + 4) == -1)
				{
					// CL is busy
					throw DatagramException("send on socket failed");
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
		size_t LOWPANDatagramService::recvfrom(char *buf, size_t length, char &type, char &flags, unsigned int &seqno, std::string &address) throw (DatagramException)
		{
			try {
				char tmp[length + 4];

				// Receive full frame from socket
				std::string address;
				uint16_t from = 0;
				uint16_t pan_id = 0;
				size_t ret = _socket->receive(tmp, length + 4, address, from, pan_id);

				// decode type, flags and seqno
				// extended mask are discovery and ACK datagrams
				if (tmp[0] & EXTENDED_MASK)
				{
					// in extended frames the second byte
					// contains the real type
					type = tmp[1];

					// copy payload to the destination buffer
					::memcpy(buf, &tmp[2], ret - 4);
				}
				else
				{
					// no extended mask means "segment"
					type = DatagramConvergenceLayer::HEADER_SEGMENT;

					// copy payload to the destination buffer
					::memcpy(buf, &tmp[1], ret - 3);
				}

				// first byte contains flags (4-bit) + seqno (4-bit)
				flags = 0x0f & (tmp[0] >> 4);
				seqno = 0x0f & tmp[0];

				address = LOWPANDatagramService::encode(from, pan_id);

				IBRCOMMON_LOGGER_DEBUG(20) << "LOWPANDatagramService::recvfrom() type: " << std::hex << (int)type << "; flags: " << std::hex << (int)flags << "; seqno: " << seqno << "; address: " << address << IBRCOMMON_LOGGER_ENDL;

				return ret - 2;
			} catch (const ibrcommon::Exception&) {
				throw DatagramException("receive failed");
			}

			return 0;
		}

		/**
		 * Get the tag for this service used in discovery messages.
		 * @return The tag as string.
		 */
		const std::string LOWPANDatagramService::getServiceTag() const
		{
			return "dgram:lowpan";
		}

		/**
		 * Get the service description for this convergence layer. This
		 * data is used to contact this node.
		 * @return The service description as string.
		 */
		const std::string LOWPANDatagramService::getServiceDescription() const
		{
			struct sockaddr_ieee802154 address;
			address.addr.addr_type = IEEE802154_ADDR_SHORT;
			address.addr.pan_id = _panid;

			// Get address via netlink
			ibrcommon::lowpansocket::getAddress(&address.addr, _iface);

			return LOWPANDatagramService::encode(address.addr.short_addr, _panid);
		}

		/**
		 * The used interface as vinterface object.
		 * @return A vinterface object.
		 */
		const ibrcommon::vinterface& LOWPANDatagramService::getInterface() const
		{
			return _iface;
		}

		/**
		 * The protocol identifier for this type of service.
		 * @return
		 */
		dtn::core::Node::Protocol LOWPANDatagramService::getProtocol() const
		{
			return dtn::core::Node::CONN_DGRAM_LOWPAN;
		}

		const DatagramConnectionParameter& LOWPANDatagramService::getParameter() const
		{
			return _params;
		}

		const std::string LOWPANDatagramService::encode(const uint16_t &addr, const int &panid)
		{
			std::stringstream ss;
			ss << "addr=" << addr << ";pan=" << panid << ";";
			return ss.str();
		}

		void LOWPANDatagramService::decode(const std::string &identifier, uint16_t &addr, int &panid)
		{
			// parse parameters
			std::vector<string> parameters = dtn::utils::Utils::tokenize(";", identifier);
			std::vector<string>::const_iterator param_iter = parameters.begin();

			while (param_iter != parameters.end())
			{
				std::vector<string> p = dtn::utils::Utils::tokenize("=", (*param_iter));

				if (p[0].compare("addr") == 0)
				{
					std::stringstream port_stream;
					port_stream << p[1];
					port_stream >> addr;
				}

				if (p[0].compare("pan") == 0)
				{
					std::stringstream port_stream;
					port_stream << p[1];
					port_stream >> panid;
				}

				param_iter++;
			}
		}
	} /* namespace net */
} /* namespace dtn */
