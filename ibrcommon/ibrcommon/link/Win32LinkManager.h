/*
 * Win32LinkManager.h
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

#include <winsock2.h>
#include <ntddndis.h>
#include <ws2ipdef.h>
#include <naptypes.h>
#include <iphlpapi.h>

namespace ibrcommon
{
	class Win32LinkManager : public LinkManager
	{
	public:
		Win32LinkManager();
		virtual ~Win32LinkManager();

		const vinterface getInterface(int index) const;
		const std::list<vaddress> getAddressList(const vinterface &iface, const std::string &scope = "");

	private:
		void freeAdapterInfo(IP_ADAPTER_ADDRESSES *pAddresses) const;
		IP_ADAPTER_ADDRESSES* getAdapterInfo() const;
		vaddress getAddress(SOCKET_ADDRESS &address) const;
	};
} /* namespace ibrcommon */
#endif /* DEFAULTLINKMANAGER_H_ */
