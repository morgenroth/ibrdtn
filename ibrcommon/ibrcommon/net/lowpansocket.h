/*
 * lowpansocket.h
 *
 *   Copyright 2011 Stefan Schmidt
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

#ifndef LOWPANSOCKET_H_
#define LOWPANSOCKET_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/Exceptions.h"

/* Move to a better place */
#include "ibrcommon/net/ieee802154.h"

namespace ibrcommon
{
	class lowpansocket
	{
	public:
		class SocketException : public ibrcommon::Exception
		{
		public:
			SocketException(string error) : ibrcommon::Exception(error)
			{};
		};

		class peer
		{
			friend class lowpansocket;

		protected:
			peer(lowpansocket &socket, const struct sockaddr_ieee802154 &dest, const unsigned int panid);

			struct sockaddr_ieee802154 _destaddress;
			lowpansocket &_socket;

		public:
			int send(const char *data, const size_t length);
		};

		virtual ~lowpansocket();

		virtual void shutdown();

		int receive(char* data, size_t maxbuffer, std::string &address, uint16_t &shortaddr, uint16_t &pan_id);

		lowpansocket::peer getPeer(unsigned int address, const unsigned int panid);
		static void getAddress(struct ieee802154_addr *ret, const vinterface &iface);

	protected:
		lowpansocket(u_char proto = 0) throw (SocketException);
		ibrcommon::vsocket _vsocket;
		struct sockaddr_ieee802154 _sockaddr;

	};
}
#endif /* LOWPANSOCKET_H_ */
