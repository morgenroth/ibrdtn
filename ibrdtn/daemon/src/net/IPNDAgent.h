/*
 * IPNDAgent.h
 *
 * IPND is based on the Internet-Draft for DTN-IPND.
 *
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
