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

namespace ibrcommon
{
	class vaddress
	{
		public:
			class address_exception : public Exception
			{
			public:
				address_exception(string error = "unspecific address error") : Exception(error)
				{};
			};

			class address_not_set : public address_exception
			{
			public:
				address_not_set(string error = "address is not specified") : address_exception(error)
				{};
			};

			class service_not_set : public address_exception
			{
			public:
				service_not_set(string error = "service is not specified") : address_exception(error)
				{};
			};

			class scope_not_set : public address_exception
			{
			public:
				scope_not_set(string error = "scope is not specified") : address_exception(error)
				{};
			};

			const static std::string SCOPE_GLOBAL;
			const static std::string SCOPE_LINKLOCAL;

			vaddress();
			vaddress(const int port);
			vaddress(const std::string &address, const int port);
			vaddress(const std::string &address, const std::string &service = "", const std::string &scope = "");
			virtual ~vaddress();

			sa_family_t family() const throw (address_exception);
			std::string scope() const throw (scope_not_set);
			const std::string address() const throw (address_not_set);
			const std::string service() const throw (service_not_set);

			bool operator<(const vaddress &dhs) const;
			bool operator!=(const vaddress &obj) const;
			bool operator==(const vaddress &obj) const;

			const std::string toString() const;

		private:
			std::string _address;
			std::string _service;
			std::string _scope;
	};
}

#endif /* VADDRESS_H_ */
