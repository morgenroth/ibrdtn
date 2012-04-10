/*
 * MulticastSocket.h
 *
 *  Created on: 28.06.2010
 *      Author: morgenro
 */

#ifndef MULTICASTSOCKET_H_
#define MULTICASTSOCKET_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/net/udpsocket.h"

namespace ibrcommon
{
	/**
	 * A MulticastSocket is derived from udpsocket and provides operations
	 * for sending and receiving multicast messages over UDP.
	 */
	class MulticastSocket
	{
	public:
		MulticastSocket(const int fd);
		virtual ~MulticastSocket();

		/**
		 * Set the default interface for outgoing datagrams
		 * @param iface
		 */
		void setInterface(const vinterface &iface);

		/**
		 * Join a multicast group.
		 * @param group The multicast group to join.
		 */
		void joinGroup(const vaddress &group);

		/**
		 * Join a multicast group.
		 * @param group The multicast group to join.
		 * @param iface The interface to use for this multicast group.
		 */
		void joinGroup(const vaddress &group, const vinterface &iface);

		/**
		 * Leave a multicast group.
		 * @param group
		 */
		void leaveGroup(const vaddress &group);

		/**
		 * Leave a multicast group.
		 * @param group The multicast group to leave.
		 * @param iface The interface used for this multicast group.
		 */
		void leaveGroup(const vaddress &group, const vinterface &iface);

	private:
		int _fd;
	};
}

#endif /* MULTICASTSOCKET_H_ */
