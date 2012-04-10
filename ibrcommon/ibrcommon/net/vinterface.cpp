/*
 * vinterface.cpp
 *
 *  Created on: 14.12.2010
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/net/LinkManager.h"

namespace ibrcommon
{
	vinterface::vinterface()
	 : _name()
	{
	}

	vinterface::vinterface(std::string name)
	 : _name(name)
	{
	}

	vinterface::~vinterface()
	{
	}

	const std::string vinterface::toString() const
	{
		if (_name.length() == 0) return "<any>";
		return _name;
	}

	const std::list<vaddress> vinterface::getAddresses(const vaddress::Family f) const
	{
		if (empty()) throw interface_not_set();
		return ibrcommon::LinkManager::getInstance().getAddressList(*this, f);
	}

	bool vinterface::operator<(const vinterface &obj) const
	{
		return (_name < obj._name);
	}

	bool vinterface::operator==(const vinterface &obj) const
	{
		return (obj._name == _name);
	}

	bool vinterface::empty() const
	{
		return (_name.length() == 0);
	}
}
