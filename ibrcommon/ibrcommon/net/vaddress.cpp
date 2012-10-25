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
#ifdef HAVE_LIBNL3
	const std::string vaddress::SCOPE_GLOBAL = "global";
#elif HAVE_LIBNL
	const std::string vaddress::SCOPE_GLOBAL = "global";
#else
	const std::string vaddress::SCOPE_GLOBAL = "universe";
#endif
	const std::string vaddress::SCOPE_LINKLOCAL = "link";

	vaddress::vaddress()
	 : _address(), _service(), _scope()
	{
	}

	vaddress::vaddress(const int port)
	 : _address(), _service(), _scope()
	{
		std::stringstream ss;
		ss << port;
		_service = ss.str();
	}

	vaddress::vaddress(const std::string &address, const int port)
	 : _address(address), _service(), _scope()
	{
		std::stringstream ss;
		ss << port;
		_service = ss.str();
	}

	vaddress::vaddress(const std::string &address, const std::string &service, const std::string &scope)
	 : _address(address), _service(service), _scope(scope)
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

	sa_family_t vaddress::family() const throw (address_exception)
	{
		struct addrinfo hints;
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = PF_UNSPEC;
		hints.ai_flags = AI_NUMERICSERV;
		hints.ai_socktype = SOCK_DGRAM;

		struct addrinfo *res;
		int ret = 0;

		const char *address = NULL;
		const char *service = NULL;

		try {
			address = this->address().c_str();
		} catch (const vaddress::address_not_set&) {
			hints.ai_family = basesocket::DEFAULT_SOCKET_FAMILY;
			hints.ai_flags |= AI_PASSIVE;
		};

		try {
			service = this->service().c_str();
		} catch (const vaddress::service_not_set&) { };

		if ((ret = ::getaddrinfo(address, service, &hints, &res)) != 0)
		{
			throw socket_exception("getaddrinfo(): " + std::string(gai_strerror(ret)));
		}

		sa_family_t fam = res->ai_family;
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
		return _address;
	}

	const std::string vaddress::service() const throw (service_not_set)
	{
		if (_service.length() == 0) throw service_not_set();
		return _service;
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
				std::string service = this->service();
				ss << "[" << this->address() << "]:" + service;
			} catch (const service_not_set&) {
				ss << this->address();
			}

			try {
				std::string scope = this->scope();
				ss << " (" << scope << ")";
			} catch (const scope_not_set&) { }
		} catch (const address_not_set&) {
			ss << "<any>";
		}

		return ss.str();
	}
}
