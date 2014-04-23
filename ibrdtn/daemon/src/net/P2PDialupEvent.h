/*
 * P2PDialupEvent.h
 *
 *  Created on: 25.02.2013
 *      Author: morgenro
 */

#ifndef P2PDIALUPEVENT_H_
#define P2PDIALUPEVENT_H_

#include "core/Event.h"
#include <ibrcommon/net/vinterface.h>

namespace dtn
{
	namespace net
	{
		class P2PDialupEvent : public dtn::core::Event
		{
		public:
			enum p2p_event_type {
				INTERFACE_UP,
				INTERFACE_DOWN
			};

			virtual ~P2PDialupEvent();

			const std::string getName() const;

			std::string getMessage() const;

			static void raise(p2p_event_type, const ibrcommon::vinterface&);

			const p2p_event_type type;
			const ibrcommon::vinterface iface;

		private:
			P2PDialupEvent(p2p_event_type t, const ibrcommon::vinterface&);
		};
	} /* namespace net */
} /* namespace dtn */
#endif /* P2PDIALUPEVENT_H_ */
