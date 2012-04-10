/*
 * UnicastSocket.h
 *
 *  Created on: 28.06.2010
 *      Author: morgenro
 */

#ifndef UNICASTSOCKET_H_
#define UNICASTSOCKET_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/net/udpsocket.h"

namespace ibrcommon
{
	class UnicastSocket : public ibrcommon::udpsocket
	{
	public:
		UnicastSocket();
		virtual ~UnicastSocket();

		void bind(int port, const vaddress &address);
		void bind(int port, const vinterface &iface);
		void bind();
	};
}

#endif /* UNICASTSOCKET_H_ */
