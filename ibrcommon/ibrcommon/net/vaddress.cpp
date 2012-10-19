/*
 * vaddress.cpp
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
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/net/socket.h"
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

namespace ibrcommon
{
	const std::string vaddress::SCOPE_GLOBAL = "global";
	const std::string vaddress::SCOPE_LINKLOCAL = "local";

	vaddress::vaddress()
	 : _address(), _scope()
	{
	}

	vaddress::vaddress(const std::string &address, const std::string &scope)
	 : _address(address), _scope(scope)
	{
	}

	vaddress::~vaddress()
	{
	}

	bool vaddress::operator<(const ibrcommon::vaddress &dhs) const
	{
		if (_address < dhs._address) return true;
		if (_scope < dhs._scope) return true;
		return false;
	}


	bool vaddress::operator!=(const vaddress &obj) const
	{
		if (_address != obj._address) return true;
		if (_scope != obj._scope) return true;
		return false;
	}

	bool vaddress::operator==(const vaddress &obj) const
	{
		if (_address != obj._address) return false;
		if (_scope != obj._scope) return false;
		return true;
	}

	sa_family_t vaddress::getFamily() const throw (address_exception)
	{
		struct addrinfo hints;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = PF_UNSPEC;
		hints.ai_flags = AI_NUMERICHOST;

		struct addrinfo *res;
		int ret = 0;

		if ((ret = ::getaddrinfo(get().c_str(), NULL, &hints, &res)) != 0)
		{
			throw socket_exception("getaddrinfo(): " + std::string(gai_strerror(ret)));
		}

		sa_family_t fam = res->ai_family;
		freeaddrinfo(res);

		return fam;
	}

	std::string vaddress::getScope() const throw (address_exception)
	{
		return _scope;
	}

	const std::string vaddress::get() const throw (address_not_set)
	{
		if (_address.length() == 0) throw address_not_set();
		return _address;
	}

	const std::string vaddress::toString() const
	{
		if (_address.length() == 0) return "<any>";
		return _address;
	}
}
