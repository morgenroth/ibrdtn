/*
 * LinkEvent.cpp
 *
 *  Created on: 11.10.2012
 *      Author: morgenro
 */

#include "LinkEvent.h"
#include <sstream>

namespace ibrcommon
{
	LinkEvent::LinkEvent(Action action, const vinterface &iface, const vaddress &addr)
	 : _action(action), _iface(iface), _addr(addr)
	{ }

	LinkEvent::~LinkEvent()
	{ }

	const vinterface& LinkEvent::getInterface() const
	{
		return _iface;
	}

	const vaddress& LinkEvent::getAddress() const
	{
		return _addr;
	}

	LinkEvent::Action LinkEvent::getAction() const
	{
		return _action;
	}

	std::string LinkEvent::toString() const
	{
		std::stringstream ss;
		ss << "interface: " << _iface.toString() << ", action: " << _action << ", address: " << _addr.toString();
		return ss.str();
	}
} /* namespace ibrcommon */
