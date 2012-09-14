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
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/net/LinkManager.h"
#include <arpa/inet.h>
#include <string.h>
#include <sstream>
#include <regex.h>

#ifndef HAVE_BZERO
#define bzero(s,n) (memset((s), '\0', (n)), (void) 0)
#endif

namespace ibrcommon
{
	const std::string vaddress::__REGEX_IPV4_ADDRESS__ = "^[[:digit:]]\\{1,3\\}.[[:digit:]]\\{1,3\\}.[[:digit:]]\\{1,3\\}.[[:digit:]]\\{1,3\\}$";
	const std::string vaddress::__REGEX_IPV6_ADDRESS__ = "^[[:alnum:]:]\\{3,40\\}\\(%[[:alnum:]]*\\)\\{0,1\\}$";


	vaddress::vaddress(const Family &family)
	 : _family(family), _address(), _iface(0)
	{
	}

	vaddress::vaddress(const std::string &address)
	 : _family(VADDRESS_UNSPEC), _address(address), _iface(0)
	{
		// guess the address family
		regex_t re;

		// test for ipv4 addresses
		if ( regcomp(&re, __REGEX_IPV4_ADDRESS__.c_str(), 0) )
		{
			// failed
			return;
		}

		// test against the regular expression
		if (!regexec(&re, address.c_str(), 0, NULL, 0))
		{
			_family = VADDRESS_INET;
			regfree(&re);
			return;
		}

		regfree(&re);

		// test for ipv6 addresses
		if ( regcomp(&re, __REGEX_IPV6_ADDRESS__.c_str(), 0) )
		{
			// failed
			return;
		}

		// test against the regular expression
		if (!regexec(&re, address.c_str(), 0, NULL, 0))
		{
			_family = VADDRESS_INET6;
			regfree(&re);
			return;
		}

		regfree(&re);

		// test for unix file
		ibrcommon::File f(address);
		if (f.exists())
		{
			_family = VADDRESS_UNIX;
		}
	}

	vaddress::vaddress(const Family &family, const std::string &address, const int iface)
	 : _family(family), _address(address), _iface(iface)
	{
	}

	vaddress::vaddress(const Family &family, const std::string &address)
	 : _family(family), _address(address), _iface(0)
	{
	}

	vaddress::~vaddress()
	{
	}

	bool vaddress::operator<(const ibrcommon::vaddress &dhs) const
	{
		if (_family < dhs._family) return true;
		if (_address < dhs._address) return true;
		if (_iface < dhs._iface) return true;
		return false;
	}

	// strip off network mask
	const std::string vaddress::strip_netmask(const std::string &data)
	{
		// search the last slash
		size_t pos = data.find_last_of("/");
		if (pos == std::string::npos) return data;

		return data.substr(0, pos);
	}

	vaddress::Family vaddress::getFamily() const
	{
		return _family;
	}

	const std::string vaddress::get(bool internal) const
	{
		if (_address.length() == 0) throw address_not_set();
		std::string ret = _address;

		// strip netmask number
		size_t slashpos = _address.find("/");
		if (slashpos != string::npos)
		{
			ret = _address.substr(0, slashpos);
		}

		if ((_iface > 0) && (internal))
		{
			stringstream ss; ss << ret << "%" << ibrcommon::LinkManager::getInstance().getInterface(_iface);
			ret = ss.str();
		}

		return ret;
	}

	bool vaddress::operator!=(const vaddress &obj) const
	{
		if (_family != obj._family) return true;
		if (_address != obj._address) return true;
		return false;
	}
	
	bool vaddress::operator==(const vaddress &obj) const
	{
		if (_family != obj._family) return false;
		if (_address != obj._address) return false;
		return true;
	}

	struct addrinfo* vaddress::addrinfo(struct addrinfo *hints) const
	{
		struct addrinfo *res;

		if (_family != VADDRESS_UNSPEC)
		{
			hints->ai_family = _family;
		}

		if (0 != getaddrinfo(get().c_str(), NULL, hints, &res))
			throw vsocket_exception("failed to getaddrinfo with address: " + get());

		return res;
	}

	struct addrinfo* vaddress::addrinfo(struct addrinfo *hints, unsigned int port) const
	{
		struct addrinfo *res;

		if (_family != VADDRESS_UNSPEC)
		{
			hints->ai_family = _family;
		}

		std::stringstream port_ss; port_ss << port;
		try {
			if (0 != getaddrinfo(get().c_str(), port_ss.str().c_str(), hints, &res))
				throw vsocket_exception("failed to getaddrinfo with address: " + get());
		}
		catch (const vaddress::address_not_set&)
		{
			if (0 != getaddrinfo(NULL, port_ss.str().c_str(), hints, &res))
				throw vsocket_exception("failed to getaddrinfo");
		}

		return res;
	}

	const std::string vaddress::toString() const
	{
		if (_address.length() == 0) return "<any>";

		if (_iface > 0)
		{
			stringstream ss; ss << _address << "%" << ibrcommon::LinkManager::getInstance().getInterface(_iface);
			return ss.str();
		}

		return _address;
	}
}
