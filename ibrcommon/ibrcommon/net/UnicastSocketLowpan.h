/*
 * UnicastSocket.h
 *
 *  Created on: 28.06.2010
 *      Author: morgenro
 */

#ifndef UNICASTSOCKETLOWPAN_H_
#define UNICASTSOCKETLOWPAN_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/net/lowpansocket.h"

namespace ibrcommon
{
	class UnicastSocketLowpan : public ibrcommon::lowpansocket
	{
	public:
		UnicastSocketLowpan();
		virtual ~UnicastSocketLowpan();

		void bind(int panid, const vaddress &address);
		void bind(int panid, const vinterface &iface);
	};
}

#endif /* UNICASTSOCKETLOWPAN_H_ */
