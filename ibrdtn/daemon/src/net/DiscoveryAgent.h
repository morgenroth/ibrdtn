/*
 * DiscoveryAgent.h
 *
 *  Created on: 09.09.2009
 *      Author: morgenro
 */

#ifndef DISCOVERYAGENT_H_
#define DISCOVERYAGENT_H_

#include "core/Node.h"
#include "net/Neighbor.h"
#include "net/DiscoveryAnnouncement.h"
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

			void addService(string name, string parameters);
			void addService(dtn::net::DiscoveryServiceProvider *provider);

		protected:
			virtual void sendAnnoucement(const u_int16_t &sn, std::list<dtn::net::DiscoveryService> &services) = 0;

			void timeout();

			const dtn::daemon::Configuration::Discovery &_config;

		private:
			std::list<Neighbor> _neighbors;
			u_int16_t _sn;
			std::list<dtn::net::DiscoveryService> _services;
			size_t _last_announce_sent;
		};
	}
}

#endif /* DISCOVERYAGENT_H_ */
