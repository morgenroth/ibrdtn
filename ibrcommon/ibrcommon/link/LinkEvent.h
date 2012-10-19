/*
 * LinkEvent.h
 *
 *  Created on: 11.10.2012
 *      Author: morgenro
 */

#ifndef LINKEVENT_H_
#define LINKEVENT_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"

namespace ibrcommon
{
	class LinkEvent
	{
	public:
		enum EventType
		{
			EVENT_UNKOWN,
			EVENT_LINK_STATE,
			EVENT_ADDRESS_REMOVED,
			EVENT_ADDRESS_ADDED
		};

		virtual ~LinkEvent() {};

		virtual const vinterface& getInterface() const = 0;
		virtual const vaddress& getAddress() const = 0;
		virtual unsigned int getState() const = 0;
		virtual EventType getType() const = 0;

		virtual std::string toString() const = 0;
	};

	class ManualLinkEvent : public LinkEvent
	{
	public:
		ManualLinkEvent(EventType type, const vinterface &iface, const vaddress &addr, unsigned int state = 0);
		virtual ~ManualLinkEvent();

		virtual const vinterface& getInterface() const;
		virtual const vaddress& getAddress() const;
		virtual unsigned int getState() const;
		virtual EventType getType() const;

		virtual std::string toString() const;

	private:
		EventType _type;
		const vinterface _iface;
		const vaddress _addr;
		unsigned int _state;
	};

} /* namespace ibrcommon */
#endif /* LINKEVENT_H_ */
