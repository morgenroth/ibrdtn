/*
 * MulticastSocket.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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
