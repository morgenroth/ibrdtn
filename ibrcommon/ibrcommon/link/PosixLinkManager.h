/*
 * DefaultLinkManager.h
 *
 *  Created on: 11.10.2012
 *      Author: morgenro
 */

#ifndef DEFAULTLINKMANAGER_H_
#define DEFAULTLINKMANAGER_H_

#include "ibrcommon/link/LinkManager.h"
#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"
#include <list>

namespace ibrcommon
{
	class PosixLinkManager : public LinkManager
	{
	public:
		PosixLinkManager();
		virtual ~PosixLinkManager();

		const vinterface getInterface(int index) const;
		const std::list<vaddress> getAddressList(const vinterface &iface, const std::string &scope = "");
	};
} /* namespace ibrcommon */
#endif /* DEFAULTLINKMANAGER_H_ */
