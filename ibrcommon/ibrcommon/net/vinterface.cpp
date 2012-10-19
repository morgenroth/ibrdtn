/*
 * vinterface.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ibrcommon/config.h"
#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vsocket.h"
#include <net/if.h>

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

	uint32_t vinterface::getIndex() const
	{
		return ::if_nametoindex(_name.c_str());
	}

	const std::list<vaddress> vinterface::getAddresses() const
	{
		if (empty()) throw interface_not_set();
		return ibrcommon::LinkManager::getInstance().getAddressList(*this);
	}

	bool vinterface::up() const {
		return !getAddresses().empty();
	}

	bool vinterface::operator<(const vinterface &obj) const
	{
		return (_name < obj._name);
	}

	bool vinterface::operator==(const vinterface &obj) const
	{
		return (obj._name == _name);
	}

	bool vinterface::operator!=(const vinterface &obj) const
	{
		return (obj._name != _name);
	}

	bool vinterface::empty() const
	{
		return (_name.length() == 0);
	}
}
