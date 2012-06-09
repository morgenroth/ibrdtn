/*
 * lowpansocket.h
 *
 *  Created on: 01.06.2010
 *      Author: stschmid
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
