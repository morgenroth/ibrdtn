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

#include <ibrcommon/net/lowpansocket.h>
#include <ibrcommon/net/lowpanstream.h>

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
			_params.max_msg_length = 113;
			_params.max_seq_numbers = 8;
			_params.flowcontrol = DatagramService::FLOW_NONE;

			// convert the panid into a string
			std::stringstream ss;
			ss << _panid;

			// assign the broadcast address
			_addr_broadcast = ibrcommon::vaddress("0xffff", ss.str());
		}

		LOWPANDatagramService::~LOWPANDatagramService()
		{
		}

		/**
		 * Bind to the local socket.
		 * @throw If the bind fails, an DatagramException is thrown.
		 */
		void LOWPANDatagramService::bind() throw (DatagramException)
		{
			try {
				_vsocket.destroy();
				_vsocket.add(new ibrcommon::lowpansocket(_panid, _iface));
				_vsocket.up();
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
			_vsocket.down();
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
				if (length > _params.max_msg_length)
				{
					IBRCOMMON_LOGGER(error) << "LOWPANConvergenceLayer::send buffer to big to be transferred (" << length << ")."<< IBRCOMMON_LOGGER_ENDL;
					throw DatagramException("send failed - buffer to big to be transferred");
				}

				ibrcommon::socketset socks = _vsocket.getAll();
				if (socks.size() == 0) return;
				ibrcommon::lowpansocket &sock = dynamic_cast<ibrcommon::lowpansocket&>(**socks.begin());

				// decode destination address
				ibrcommon::vaddress destaddr;
				size_t end_of_payload = length + 1;
				LOWPANDatagramService::decode(identifier, destaddr);

				// buffer for the datagram
				char tmp[length + 2];

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

				// send converted line
				sock.sendto(buf, end_of_payload, 0, destaddr);
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
				if (length > _params.max_msg_length)
				{
					IBRCOMMON_LOGGER(error) << "LOWPANConvergenceLayer::send buffer to big to be transferred (" << length << ")."<< IBRCOMMON_LOGGER_ENDL;
					throw DatagramException("send failed - buffer to big to be transferred");
				}

				ibrcommon::socketset socks = _vsocket.getAll();
				if (socks.size() == 0) return;
				ibrcommon::lowpansocket &sock = dynamic_cast<ibrcommon::lowpansocket&>(**socks.begin());

				// extend the buffer if the len is zero (ACKs)
				if (length == 0) length++;

				// buffer for the datagram plus local address
				char tmp[length + 2];

				// encode header: 2-bit unused, flags (2-bit) + seqno (4-bit)
				tmp[0] = (0x30 & (flags << 4)) | (0x0f & seqno);

				// set datagram to extended
				tmp[0] |= EXTENDED_MASK;

				// encode the type into the second byte
				tmp[1] = type;

				// copy payload to the new buffer
				::memcpy(&tmp[2], buf, length);

				// send converted line
				sock.sendto(buf, length + 2, 0, _addr_broadcast);
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
				ibrcommon::socketset readfds;
				_vsocket.select(&readfds, NULL, NULL, NULL);

				for (ibrcommon::socketset::iterator iter = readfds.begin(); iter != readfds.end(); iter++) {
					ibrcommon::lowpansocket &sock = dynamic_cast<ibrcommon::lowpansocket&>(**iter);

					char tmp[length + 2];

					// Receive full frame from socket
					ibrcommon::vaddress fromaddr;

					size_t ret = sock.recvfrom(tmp, length + 2, 0, fromaddr);

					// decode type, flags and seqno
					// extended mask are discovery and ACK datagrams
					if (tmp[0] & EXTENDED_MASK)
					{
						// in extended frames the second byte
						// contains the real type
						type = tmp[1];

						// copy payload to the destination buffer
						ret -= 2;
						::memcpy(buf, &tmp[2], ret);
					}
					else
					{
						// no extended mask means "segment"
						type = DatagramConvergenceLayer::HEADER_SEGMENT;

						// copy payload to the destination buffer
						ret -= 1;
						::memcpy(buf, &tmp[1], ret);
					}

					// first byte contains flags (4-bit) + seqno (4-bit)
					flags = 0x0f & (tmp[0] >> 4);
					seqno = 0x0f & tmp[0];

					// encode into the right format
					address = LOWPANDatagramService::encode(fromaddr);

					IBRCOMMON_LOGGER_DEBUG(20) << "LOWPANDatagramService::recvfrom() type: " << std::hex << (int)type << "; flags: " << std::hex << (int)flags << "; seqno: " << seqno << "; address: " << address << IBRCOMMON_LOGGER_ENDL;

					return ret;
				}
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
			std::stringstream ss_pan; ss_pan << _panid;

			// Get address via netlink
			ibrcommon::vaddress addr;
			ibrcommon::lowpansocket::getAddress(_iface, ss_pan.str(), addr);

			return LOWPANDatagramService::encode(addr);
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

		const DatagramService::Parameter& LOWPANDatagramService::getParameter() const
		{
			return _params;
		}

		const std::string LOWPANDatagramService::encode(const ibrcommon::vaddress &addr)
		{
			std::stringstream ss;
			ss << "addr=" << addr.address() << ";pan=" << addr.service() << ";";
			return ss.str();
		}

		void LOWPANDatagramService::decode(const std::string &identifier, ibrcommon::vaddress &addr)
		{
			std::string address;
			std::string service;

			// parse parameters
			std::vector<string> parameters = dtn::utils::Utils::tokenize(";", identifier);
			std::vector<string>::const_iterator param_iter = parameters.begin();

			while (param_iter != parameters.end())
			{
				std::vector<string> p = dtn::utils::Utils::tokenize("=", (*param_iter));

				if (p[0].compare("addr") == 0)
				{
					address = p[1];
				}

				if (p[0].compare("pan") == 0)
				{
					service = p[1];
				}

				param_iter++;
			}

			addr = ibrcommon::vaddress(address, service);
		}
	} /* namespace net */
} /* namespace dtn */
