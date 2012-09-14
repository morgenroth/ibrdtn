/*
 * vinterface.h
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

#ifndef VINTERFACE_H_
#define VINTERFACE_H_

#include "ibrcommon/net/vaddress.h"

#include <list>
#include <string>

namespace ibrcommon
{
	class LinkManagerEvent;

	class vinterface
	{
	public:
		class interface_not_set : public Exception
		{
		public:
			interface_not_set(string error = "interface is not specified") : Exception(error)
			{};
		};

		vinterface();
		vinterface(std::string name);
		virtual ~vinterface();

		uint32_t getIndex() const;
		const std::list<vaddress> getAddresses(const vaddress::Family f = vaddress::VADDRESS_UNSPEC) const;
		const std::string toString() const;
		bool empty() const;

		bool operator<(const vinterface &obj) const;
		bool operator==(const vinterface &obj) const;

		void eventNotify(const LinkManagerEvent &evt);

	private:
		std::string _name;
	};
}

#endif /* VINTERFACE_H_ */
