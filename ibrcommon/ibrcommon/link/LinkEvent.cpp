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
	ManualLinkEvent::ManualLinkEvent(EventType type, const vinterface &iface, const vaddress &addr, unsigned int state)
	 : _type(type), _iface(iface), _addr(addr), _state(state)
	{ };

	ManualLinkEvent::~ManualLinkEvent()
	{ };

	const vinterface& ManualLinkEvent::getInterface() const
	{
		return _iface;
	};

	const vaddress& ManualLinkEvent::getAddress() const
	{
		return _addr;
	};

	unsigned int ManualLinkEvent::getState() const
	{
		return _state;
	};

	LinkEvent::EventType ManualLinkEvent::getType() const
	{
		return _type;
	};

	std::string ManualLinkEvent::toString() const
	{
		std::stringstream ss;
		ss << "Event on " << _iface.toString() << ", type: " << _type << ", state: " << _state << ", " << _addr.toString();
		return ss.str();
	};
} /* namespace ibrcommon */
