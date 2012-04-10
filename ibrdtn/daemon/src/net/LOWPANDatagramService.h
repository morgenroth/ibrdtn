/*
 * LOWPANDatagramService.h
 *
 *  Created on: 24.11.2011
 *      Author: morgenro
 */

#ifndef LOWPANDATAGRAMSERVICE_H_
#define LOWPANDATAGRAMSERVICE_H_

#include "net/DatagramConvergenceLayer.h"
#include "net/DatagramConnectionParameter.h"
#include <ibrcommon/net/lowpansocket.h>

namespace dtn
{
	namespace net
	{
		class LOWPANDatagramService : public dtn::net::DatagramService
		{
		public:
			LOWPANDatagramService(const ibrcommon::vinterface &iface, int panid);
			virtual ~LOWPANDatagramService();

			/**
			 * Bind to the local socket.
			 * @throw If the bind fails, an DatagramException is thrown.
			 */
			virtual void bind() throw (DatagramException);

			/**
			 * Shutdown the socket. Unblock all calls on the socket (recv, send, etc.)
			 */
			virtual void shutdown();

			/**
			 * Send the payload as datagram to a defined destination
			 * @param address The destination address encoded as string.
			 * @param buf The buffer to send.
			 * @param length The number of available bytes in the buffer.
			 */
			virtual void send(const char &type, const char &flags, const unsigned int &seqno, const std::string &address, const char *buf, size_t length) throw (DatagramException);

			/**
			 * Send the payload as datagram to all neighbors (broadcast)
			 * @param buf The buffer to send.
			 * @param length The number of available bytes in the buffer.
			 */
			virtual void send(const char &type, const char &flags, const unsigned int &seqno, const char *buf, size_t length) throw (DatagramException);

			/**
			 * Receive an incoming datagram.
			 * @param buf A buffer to catch the incoming data.
			 * @param length The length of the buffer.
			 * @param address A buffer for the address of the sender.
			 * @throw If the receive call failed for any reason, an DatagramException is thrown.
			 * @return The number of received bytes.
			 */
			virtual size_t recvfrom(char *buf, size_t length, char &type, char &flags, unsigned int &seqno, std::string &address) throw (DatagramException);

			/**
			 * Get the tag for this service used in discovery messages.
			 * @return The tag as string.
			 */
			virtual const std::string getServiceTag() const;

			/**
			 * Get the service description for this convergence layer. This
			 * data is used to contact this node.
			 * @return The service description as string.
			 */
			virtual const std::string getServiceDescription() const;

			/**
			 * The used interface as vinterface object.
			 * @return A vinterface object.
			 */
			virtual const ibrcommon::vinterface& getInterface() const;

			/**
			 * The protocol identifier for this type of service.
			 * @return
			 */
			virtual dtn::core::Node::Protocol getProtocol() const;

			/**
			 * Returns the parameter for the connection.
			 * @return
			 */
			virtual const DatagramConnectionParameter& getParameter() const;

		private:
			static const std::string encode(const uint16_t &addr, const int &panid);
			static void decode(const std::string &identifier, uint16_t &addr, int &panid);

			const static uint16_t BROADCAST_ADDR = 0;
			int _panid;
			const ibrcommon::vinterface _iface;
			DatagramConnectionParameter _params;

			ibrcommon::lowpansocket *_socket;
		};
	} /* namespace net */
} /* namespace dtn */
#endif /* LOWPANDATAGRAMSERVICE_H_ */
