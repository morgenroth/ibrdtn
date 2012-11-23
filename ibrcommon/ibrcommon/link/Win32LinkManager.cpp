/*
 * Win32LinkManager.cpp
 *
 *  Created on: 11.10.2012
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/link/Win32LinkManager.h"

namespace ibrcommon
{
	Win32LinkManager::Win32LinkManager()
	{
	}

	Win32LinkManager::~Win32LinkManager()
	{
	}

	const vinterface Win32LinkManager::getInterface(int index) const
	{
		vinterface ret;
		// TODO: find the interface for the given index
		return ret;
	}

	const std::list<vaddress> Win32LinkManager::getAddressList(const vinterface &iface, const std::string &scope)
	{
		std::list<vaddress> ret;

		// TODO: return the list of addresses bound to given interface

		return ret;
	}

} /* namespace ibrcommon */
