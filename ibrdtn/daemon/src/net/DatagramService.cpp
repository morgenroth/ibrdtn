/*
 * DatagramService.cpp
 *
 *  Created on: 09.11.2012
 *      Author: morgenro
 */

#include "net/DatagramService.h"
#include "net/DiscoveryService.h"

namespace dtn
{
	namespace net
	{
		DatagramService::~DatagramService()
		{
		}

		const std::string DatagramService::getServiceTag() const
		{
			return dtn::net::DiscoveryService::asTag(getProtocol());
		}
	}
}
