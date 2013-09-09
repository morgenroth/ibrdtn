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

#ifdef __WIN32__
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <string.h>

namespace ibrcommon
{
#if (defined HAVE_LIBNL3) || (defined HAVE_LIBNL)
	const std::string vaddress::SCOPE_GLOBAL = "global";
#else
	const std::string vaddress::SCOPE_GLOBAL = "universe";
#endif
	const std::string vaddress::SCOPE_LINKLOCAL = "link";

	const std::string vaddress::VADDR_LOCALHOST = "localhost";
	const std::string vaddress::VADDR_ANY = "any";

	vaddress::vaddress()
	 : _address(VADDR_ANY), _service(), _scope(), _family(AF_UNSPEC)
	{
	}

	vaddress::vaddress(const int port, sa_family_t family)
	 : _address(VADDR_ANY), _service(), _scope(), _family(family)
	{
		std::stringstream ss;
		ss << port;
		_service = ss.str();
	}

	vaddress::vaddress(const std::string &address, const int port, sa_family_t family)
	 : _address(address), _service(), _scope(), _family(family)
	{
		std::stringstream ss;
		ss << port;
		_service = ss.str();
	}

	vaddress::vaddress(const std::string &address, const std::string &service, sa_family_t family)
	 : _address(address), _service(service), _scope(), _family(family)
	{
	}

	vaddress::vaddress(const std::string &address, const std::string &service, const std::string &scope, sa_family_t family)
	 : _address(address), _service(service), _scope(scope), _family(family)
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

	bool vaddress::isLocal() const
	{
		return (_address == VADDR_LOCALHOST);
	}

	bool vaddress::isAny() const
	{
		return (_address == VADDR_ANY);
	}

	sa_family_t vaddress::family() const throw (address_exception)
	{
		if (_family != AF_UNSPEC)
			return _family;

		struct addrinfo hints;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = PF_UNSPEC;
		hints.ai_socktype = SOCK_DGRAM;

		struct addrinfo *res;
		int ret = 0;

		const char *address = NULL;
		const char *service = NULL;

		// throw exception if the address is not set.
		// without an address we can not determine the address family
		address = this->address().c_str();

		try {
			service = this->service().c_str();
		} catch (const vaddress::service_not_set&) { };

		if ((ret = ::getaddrinfo(address, service, &hints, &res)) != 0)
		{
			throw address_exception("getaddrinfo(): " + std::string(gai_strerror(ret)));
		}

		sa_family_t fam = static_cast<sa_family_t>(res->ai_family);
		freeaddrinfo(res);

		return fam;
	}

	std::string vaddress::scope() const throw (scope_not_set)
	{
		if (_scope.length() == 0) throw scope_not_set();
		return _scope;
	}

	const std::string vaddress::address() const throw (address_not_set)
	{
		if (_address.length() == 0) throw address_not_set();
		if (isAny()) throw address_not_set();
		return _address;
	}

	const std::string vaddress::service() const throw (service_not_set)
	{
		if (_service.length() == 0) throw service_not_set();
		return _service;
	}

	void vaddress::setService(const uint32_t port)
	{
		std::stringstream ss;
		ss << port;
		_service = ss.str();
	}

	void vaddress::setService(const std::string &service)
	{
		_service = service;
	}

	const std::string vaddress::toString() const
	{
		std::stringstream ss;

		try {
			try {
				const std::string service = this->service();
				const std::string address = this->address();
				ss << "[" << address << "]:" + service;
			} catch (const service_not_set&) {
				ss << this->address();
			}

			try {
				const std::string scope = this->scope();
				ss << " (" << scope << ")";
			} catch (const scope_not_set&) { }
		} catch (const address_not_set&) {
			ss << "<any>";
			try {
				const std::string service = this->service();
				ss << ":" + service;
			} catch (const service_not_set&) { }
		}

		return ss.str();
	}
}
