/*
 * lowpansocket.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Stefan Schmidt <stefan@datenfreihafen.org>
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

#include "ibrcommon/config.h"
#include "ibrcommon/net/lowpansocket.h"
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>

#if defined HAVE_LIBNL || HAVE_LIBNL3
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#endif

extern "C" {
#include "ibrcommon/net/ieee802154.h"
#include "ibrcommon/net/nl802154.h"
extern struct nla_policy ieee802154_policy[IEEE802154_ATTR_MAX + 1];
};

namespace ibrcommon
{
	lowpansocket::lowpansocket(u_char proto) throw (SocketException)
	{
		int sock = 0;

		// Create socket for listening for client connection requests.
		if ((sock = ::socket(PF_IEEE802154, SOCK_DGRAM, 0)) < 0) // Ignoring proto for now
		{
			throw SocketException("lowpansocket: cannot create listen socket");
		}

		bzero(&_sockaddr, sizeof(_sockaddr));
		_sockaddr.family = AF_IEEE802154;
		_sockaddr.addr.addr_type = IEEE802154_ADDR_SHORT;

		// add the new fd to the virtual socket
		_vsocket.add(sock);
	}

	lowpansocket::~lowpansocket()
	{
		shutdown();
	}

	void lowpansocket::shutdown()
	{
		_vsocket.close();
	}

	int lowpansocket::receive(char* data, size_t maxbuffer, std::string &address, uint16_t &shortaddr, uint16_t &pan_id)
	{
		struct sockaddr_ieee802154 clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);

		std::list<int> fds;
		_vsocket.select(fds, NULL);
		int fd = fds.front();

		int ret = recvfrom(fd, data, maxbuffer, MSG_WAITALL, (struct sockaddr *) &clientAddress, &clientAddressLength);

		std::stringstream hwaddr;
		hwaddr << setfill('0');
		for (int i = 7; i >= 0; i--) {
			if (i < 7) hwaddr << ":";
			hwaddr << std::hex << setw(2) << (unsigned int)clientAddress.addr.hwaddr[i];
		}

		address = hwaddr.str();
		pan_id = ntohs(clientAddress.addr.pan_id);
		shortaddr = ntohs(clientAddress.addr.short_addr);

		return ret;
	}

	lowpansocket::peer::peer(lowpansocket &socket, const struct sockaddr_ieee802154 &dest, const unsigned int panid)
	 : _socket(socket)
	{
		bzero(&_destaddress, sizeof(_destaddress));
		_destaddress.family = AF_IEEE802154;
		_destaddress.addr.addr_type = IEEE802154_ADDR_SHORT;

		memcpy(&_destaddress.addr.short_addr, &dest.addr.short_addr, sizeof(_destaddress.addr.short_addr));
		_destaddress.addr.pan_id = panid;
	}

	int lowpansocket::peer::send(const char *data, const size_t length)
	{
		int stat = -1;

		// iterate over all sockets
		vsocket::fd_address_list fds = _socket._vsocket.get();

		for (vsocket::fd_address_list::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
		{
			vsocket::fd_address_entry e = (*iter);
			int fd = e.second;

			ssize_t ret = 0;

			::connect(_socket._vsocket.fd(), (struct sockaddr *) &_destaddress, sizeof(_destaddress));
			//printf("lowpan send() address %04x, PAN %04x\n", _destaddress.addr.short_addr, _destaddress.addr.pan_id);
			//return ::sendto(_socket._socket, data, length, 0, (struct sockaddr *) &_destaddress, sizeof(_destaddress));
			ret = ::send(fd, data, length, 0);

			// if the send was successful, set the correct return value
			if (ret != -1) stat = ret;
		}

		return stat;
	}

	lowpansocket::peer lowpansocket::getPeer(unsigned int address, const unsigned int panid)
	{
		struct sockaddr_ieee802154 destaddress;

		destaddress.addr.short_addr = address;
		return lowpansocket::peer(*this, destaddress, panid);
	}

	void lowpansocket::getAddress(struct ieee802154_addr *ret, const vinterface &iface)
	{
#if defined HAVE_LIBNL || HAVE_LIBNL3
#ifdef HAVE_LIBNL3
		struct nl_sock *nl = nl_socket_alloc();
#else
		struct nl_handle *nl = nl_handle_alloc();
#endif
		unsigned char *buf = NULL;
		struct sockaddr_nl nla;
		struct nlattr *attrs[IEEE802154_ATTR_MAX+1];
		struct genlmsghdr *ghdr;
		struct nlmsghdr *nlh;
		struct nl_msg *msg;
		int family;

		if (!nl)
		        return;

		genl_connect(nl);

		/* Build and send message */
		msg = nlmsg_alloc();
		family = genl_ctrl_resolve(nl, "802.15.4 MAC");
		genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, NLM_F_ECHO, IEEE802154_LIST_IFACE, 1);
		nla_put_string(msg, IEEE802154_ATTR_DEV_NAME, iface.toString().c_str());
		nl_send_auto_complete(nl, msg);
		nlmsg_free(msg);

		/* Receive and parse answer */
		nl_recv(nl, &nla, &buf, NULL);
		nlh = (struct nlmsghdr*)buf;
		genlmsg_parse(nlh, 0, attrs, IEEE802154_ATTR_MAX, ieee802154_policy);
		ghdr = (genlmsghdr*)nlmsg_data(nlh);
		if (!attrs[IEEE802154_ATTR_SHORT_ADDR] || !attrs[IEEE802154_ATTR_SHORT_ADDR])
		        return;

		// We only handle short addresses right now
		ret->addr_type = IEEE802154_ADDR_SHORT;
		ret->pan_id = nla_get_u16(attrs[IEEE802154_ATTR_PAN_ID]);
		ret->short_addr = nla_get_u16(attrs[IEEE802154_ATTR_SHORT_ADDR]);

		free(buf);
		nl_close(nl);

#ifdef HAVE_LIBNL3
		nl_socket_free(nl);
#else
		nl_handle_destroy(nl);
#endif
#endif
	}
}
