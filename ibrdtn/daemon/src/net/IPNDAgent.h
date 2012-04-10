/*
 * IPNDAgent.h
 *
 *  Created on: 09.09.2009
 *      Author: morgenro
 *
 * IPND is based on the Internet-Draft for DTN-IPND.
 *
 * MUST support multicast
 * SHOULD support broadcast and unicast
 *
 *
 *
 */

#ifndef IPNDAGENT_H_
#define IPNDAGENT_H_

#include "Component.h"
#include "net/DiscoveryAgent.h"
#include "net/DiscoveryAnnouncement.h"
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/net/udpsocket.h>
#include <ibrcommon/net/vsocket.h>
#include <ibrcommon/net/LinkManager.h>
#include <list>
#include <map>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		class IPNDAgent : public DiscoveryAgent, public dtn::daemon::IndependentComponent, public ibrcommon::LinkManager::EventCallback
		{
		public:
			IPNDAgent(int port, const ibrcommon::vaddress &address);
			virtual ~IPNDAgent();

			void bind(const ibrcommon::vinterface &net);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			void eventNotify(const ibrcommon::LinkManagerEvent &evt);

		protected:
			void sendAnnoucement(const u_int16_t &sn, std::list<DiscoveryService> &services);
			virtual void componentRun();
			virtual void componentUp();
			virtual void componentDown();
			void __cancellation();

		private:
			void send(const DiscoveryAnnouncement &a, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr, const unsigned int port);

			DiscoveryAnnouncement::DiscoveryVersion _version;
			ibrcommon::vsocket _socket;
			ibrcommon::vaddress _destination;
			std::list<ibrcommon::vinterface> _interfaces;

			int _port;
			int _fd;
		};
	}
}

#endif /* IPNDAGENT_H_ */
