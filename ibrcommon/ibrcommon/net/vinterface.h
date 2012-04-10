/*
 * vinterface.h
 *
 *  Created on: 14.12.2010
 *      Author: morgenro
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
