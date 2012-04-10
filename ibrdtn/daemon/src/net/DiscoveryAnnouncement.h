/*
 * DiscoveryAnnouncement.h
 *
 *  Created on: 09.09.2009
 *      Author: morgenro
 */

#ifndef DISCOVERYANNOUNCEMENT_H_
#define DISCOVERYANNOUNCEMENT_H_

#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/BundleString.h>
#include "net/DiscoveryService.h"
#include <string>
#include <list>

namespace dtn
{
	namespace net
	{
		class DiscoveryAnnouncement
		{
			enum BEACON_FLAGS_V1
			{
				BEACON_NO_FLAGS = 0x00,
				BEACON_SHORT = 0x01
			};

			enum BEACON_FLAGS_V2
			{
				BEACON_CONTAINS_EID = 0x01,
				BEACON_SERVICE_BLOCK = 0x02,
				BEACON_BLOOMFILTER = 0x04
			};

		public:
			enum DiscoveryVersion
			{
				DTND_IPDISCOVERY = 0x00,
				DISCO_VERSION_00 = 0x01,
				DISCO_VERSION_01 = 0x02
			};

			DiscoveryAnnouncement(const DiscoveryVersion version = DISCO_VERSION_00, dtn::data::EID eid = dtn::data::EID());

			virtual ~DiscoveryAnnouncement();

			dtn::data::EID getEID() const;

			const std::list<DiscoveryService> getServices() const;
			void clearServices();
			void addService(DiscoveryService service);
			const DiscoveryService& getService(string name) const;

			string toString() const;

			void setSequencenumber(u_int16_t sequence);

			bool isShort();

		private:
			friend std::ostream &operator<<(std::ostream &stream, const DiscoveryAnnouncement &announcement);
			friend std::istream &operator>>(std::istream &stream, DiscoveryAnnouncement &announcement);

			unsigned int _version;
			unsigned char _flags;
			dtn::data::EID _canonical_eid;
			u_int16_t _sn;
			dtn::data::BundleString _bloomfilter;

			std::list<DiscoveryService> _services;
		};
	}
}

#endif /* DISCOVERYANNOUNCEMENT_H_ */
