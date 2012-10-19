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

			const static std::string SCOPE_GLOBAL;
			const static std::string SCOPE_LINKLOCAL;

			vaddress();
			vaddress(const std::string &address, const std::string &scope = "");
			virtual ~vaddress();

			sa_family_t getFamily() const throw (address_exception);
			std::string getScope() const throw (address_exception);
			const std::string get() const throw (address_not_set);

			bool operator<(const vaddress &dhs) const;
			bool operator!=(const vaddress &obj) const;
			bool operator==(const vaddress &obj) const;

			const std::string toString() const;

		private:
			std::string _address;
			std::string _scope;
	};
}

#endif /* VADDRESS_H_ */
