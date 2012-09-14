/*
 * DiscoveryAgent.h
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

#ifndef DISCOVERYAGENT_H_
#define DISCOVERYAGENT_H_

#include "core/Node.h"
#include "net/Neighbor.h"
#include "net/DiscoveryAnnouncement.h"
#include "net/DiscoveryServiceProvider.h"
#include "net/DiscoveryService.h"
#include "Configuration.h"

#include <list>

namespace dtn
{
	namespace net
	{
		class DiscoveryAgent
		{
		public:
			DiscoveryAgent(const dtn::daemon::Configuration::Discovery &config);
			virtual ~DiscoveryAgent() = 0;

			void received(const dtn::net::DiscoveryAnnouncement &announcement, size_t timeout = 0);

			void addService(dtn::net::DiscoveryServiceProvider *provider);

		protected:
			virtual void sendAnnoucement(const u_int16_t &sn, std::list<dtn::net::DiscoveryServiceProvider*> &provider) = 0;

			void timeout();

			const dtn::daemon::Configuration::Discovery &_config;

		private:
			std::list<Neighbor> _neighbors;
			u_int16_t _sn;
			std::list<dtn::net::DiscoveryServiceProvider*> _provider;
			size_t _last_announce_sent;
		};
	}
}

#endif /* DISCOVERYAGENT_H_ */
