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
		enum Action
		{
			ACTION_UNKOWN,
			ACTION_LINK_RUNNING,
			ACTION_LINK_UP,
			ACTION_LINK_DOWN,
			ACTION_ADDRESS_REMOVED,
			ACTION_ADDRESS_ADDED
		};

		LinkEvent(Action action, const vinterface &iface, const vaddress &addr);
		virtual ~LinkEvent();

		virtual const vinterface& getInterface() const;
		virtual const vaddress& getAddress() const;
		virtual Action getAction() const;

		virtual std::string toString() const;

	private:
		Action _action;
		const vinterface _iface;
		const vaddress _addr;
	};

} /* namespace ibrcommon */
#endif /* LINKEVENT_H_ */
