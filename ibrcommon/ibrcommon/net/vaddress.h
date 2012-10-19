/*
 * vaddress.h
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

#ifndef VADDRESS_H_
#define VADDRESS_H_

#include "ibrcommon/Exceptions.h"
#include <sys/socket.h>
#include <netdb.h>

namespace ibrcommon
{
	class vaddress
	{
		public:
			class address_not_set : public Exception
			{
			public:
				address_not_set(string error = "address is not specified") : Exception(error)
				{};
			};

			class family_not_set : public Exception
			{
			public:
				family_not_set(string error = "family is not specified") : Exception(error)
				{};
			};

			enum Scope
			{
				SCOPE_UNKOWN,
				SCOPE_GLOBAL,
				SCOPE_LINKLOCAL
			};

			vaddress();
			vaddress(const std::string &address, const std::string &scope);
//			vaddress(const Family &family = VADDRESS_INET);
//			vaddress(const Family &family, const std::string &address);
//			vaddress(const Family &family, const std::string &address, const int iface, const Scope scope = SCOPE_UNKOWN);
			virtual ~vaddress();

			sa_family_t getFamily() const;
			Scope getScope() const;
			const std::string get(bool internal = true) const;

			bool operator!=(const vaddress &obj) const;
			bool operator==(const vaddress &obj) const;

			const std::string toString() const;

//			struct addrinfo* addrinfo(struct addrinfo *hints) const;
//			struct addrinfo* addrinfo(struct addrinfo *hints, unsigned int port) const;

//			static const std::string strip_netmask(const std::string &data);

			bool operator<(const ibrcommon::vaddress &dhs) const;

		private:
//			static const std::string __REGEX_IPV6_ADDRESS__;
//			static const std::string __REGEX_IPV4_ADDRESS__;

//			Family _family;
//			Scope _scope;
			std::string _address;
			std::string _scope;
//			int _iface;
	};
}

#endif /* VADDRESS_H_ */
