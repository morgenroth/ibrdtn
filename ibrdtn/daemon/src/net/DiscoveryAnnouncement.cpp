/*
 * DiscoveryAnnouncement.cpp
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

#include "net/DiscoveryAnnouncement.h"
#include <ibrdtn/data/Exceptions.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrcommon/Logger.h>
#include <netinet/in.h>
#include <typeinfo>
#include <iostream>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		DiscoveryAnnouncement::DiscoveryAnnouncement(const DiscoveryVersion version, dtn::data::EID eid)
		 : _version(version), _flags(BEACON_NO_FLAGS), _canonical_eid(eid)
		{
		}

		DiscoveryAnnouncement::~DiscoveryAnnouncement()
		{
		}

		bool DiscoveryAnnouncement::isShort()
		{
			switch (_version)
			{
			case DISCO_VERSION_00:
				return (_flags & DiscoveryAnnouncement::BEACON_SHORT);

			case DISCO_VERSION_01:
				return !(_flags & DiscoveryAnnouncement::BEACON_SERVICE_BLOCK);
			};

			return false;
		}

		void DiscoveryAnnouncement::setEID(const dtn::data::EID &eid)
		{
			_canonical_eid = eid;
		}

		dtn::data::EID DiscoveryAnnouncement::getEID() const
		{
			return _canonical_eid;
		}

		const list<DiscoveryService> DiscoveryAnnouncement::getServices() const
		{
			return _services;
		}

		void DiscoveryAnnouncement::clearServices()
		{
			_services.clear();
		}

		const DiscoveryService& DiscoveryAnnouncement::getService(string name) const
		{
			for (std::list<DiscoveryService>::const_iterator iter = _services.begin(); iter != _services.end(); iter++)
			{
				if ((*iter).getName() == name)
				{
					return (*iter);
				}
			}

			throw dtn::MissingObjectException("No service found with tag " + name);
		}

		void DiscoveryAnnouncement::addService(DiscoveryService service)
		{
			_services.push_back(service);
		}

		void DiscoveryAnnouncement::setSequencenumber(u_int16_t sequence)
		{
			_sn = sequence;
		}

		std::ostream &operator<<(std::ostream &stream, const DiscoveryAnnouncement &announcement)
		{
			const list<DiscoveryService> &services = announcement._services;

			switch (announcement._version)
			{
				case DiscoveryAnnouncement::DISCO_VERSION_00:
				{
					if (services.empty())
					{
						unsigned char flags = DiscoveryAnnouncement::BEACON_SHORT | announcement._flags;
						stream << (unsigned char)DiscoveryAnnouncement::DISCO_VERSION_00 << flags;
						return stream;
					}

					dtn::data::BundleString eid(announcement._canonical_eid.getString());
					dtn::data::SDNV beacon_len;

					// determine the beacon length
					beacon_len += eid.getLength();

					// add service block length
					for (list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); iter++)
					{
						beacon_len += (*iter).getLength();
					}

					stream << (unsigned char)DiscoveryAnnouncement::DISCO_VERSION_00 << announcement._flags << beacon_len << eid;

					for (list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); iter++)
					{
						stream << (*iter);
					}

					break;
				}

				case DiscoveryAnnouncement::DISCO_VERSION_01:
				{
					unsigned char flags = 0;

					stream << (unsigned char)DiscoveryAnnouncement::DISCO_VERSION_01;

					if (announcement._canonical_eid != EID())
					{
						flags |= DiscoveryAnnouncement::BEACON_CONTAINS_EID;
					}

					if (!services.empty())
					{
						flags |= DiscoveryAnnouncement::BEACON_SERVICE_BLOCK;
					}

					stream << flags;

					// sequencenumber
					u_int16_t sn = htons(announcement._sn);
					stream.write( (char*)&sn, 2 );

					if ( flags && DiscoveryAnnouncement::BEACON_CONTAINS_EID )
					{
						dtn::data::BundleString eid(announcement._canonical_eid.getString());
						stream << eid;
					}

					if ( flags && DiscoveryAnnouncement::BEACON_SERVICE_BLOCK )
					{
						stream << dtn::data::SDNV(services.size());

						for (list<DiscoveryService>::const_iterator iter = services.begin(); iter != services.end(); iter++)
						{
							stream << (*iter);
						}
					}

					break;
				}

				case DiscoveryAnnouncement::DTND_IPDISCOVERY:
				{
					u_int8_t cl_type = 1;
					char zero = '\0';
					u_int8_t interval = 10;
					// u_int32_t inet_addr;
					u_int16_t inet_port = htons(4556);
					std::string eid = announcement._canonical_eid.getString();
					u_int16_t eid_len = htons(eid.length());
					unsigned int add_zeros = (4 - (eid.length() % 4)) % 4;
					u_int16_t length = htons(12 + eid.length() + add_zeros);


					stream << (unsigned char)cl_type;
					stream.write((char*)&interval, 1);
					stream.write((char*)&length, 2);

//					std::list<dtn::daemon::Configuration::NetConfig> interfaces = dtn::daemon::Configuration::getInstance().getInterfaces();
//					dtn::daemon::Configuration::NetConfig &i = interfaces.front();

//					struct sockaddr_in sock_address;
//
//					// set the local interface address
//					i.interface.getAddress(&sock_address.sin_addr);
//
//					stream.write((char*)&sock_address.sin_addr, 4);
					stream.write(&zero, 1);
					stream.write(&zero, 1);
					stream.write(&zero, 1);
					stream.write(&zero, 1);

					stream.write((char*)&inet_port, 2);
					stream.write((char*)&eid_len, 2);
					stream << eid;

					for (unsigned int i = 0; i < add_zeros; i++)
					{
						stream.write((char*)&zero, 1);
					}

					break;
				}
			}

			return stream;
		}

		std::istream &operator>>(std::istream &stream, DiscoveryAnnouncement &announcement)
		{
			unsigned char version = 0;

			// do we running DTN2 compatibility mode?
			if (announcement._version == DiscoveryAnnouncement::DTND_IPDISCOVERY)
			{
				// set version to IPDiscovery (DTN2)
				version = DiscoveryAnnouncement::DTND_IPDISCOVERY;
			}
			else
			{
				// read IPND version of the frame
				version = stream.get();
			}

			switch (version)
			{
			case DiscoveryAnnouncement::DISCO_VERSION_00:
			{
				IBRCOMMON_LOGGER_DEBUG(60) << "beacon version 1 received" << IBRCOMMON_LOGGER_ENDL;

				dtn::data::SDNV beacon_len;
				dtn::data::SDNV eid_len;

				stream.get((char&)announcement._flags);

				// catch a short beacon
				if (DiscoveryAnnouncement::BEACON_SHORT == announcement._flags)
				{
					announcement._canonical_eid = dtn::data::EID();
					return stream;
				}

				stream >> beacon_len; int remain = beacon_len.getValue();

				dtn::data::BundleString eid;
				stream >> eid; remain -= eid.getLength();
				announcement._canonical_eid = dtn::data::EID((std::string)eid);

				// get the services
				list<DiscoveryService> &services = announcement._services;

				// clear the services
				services.clear();

				while (remain > 0)
				{
					// decode the service blocks
					DiscoveryService service;
					stream >> service;
					services.push_back(service);
					remain -= service.getLength();
				}
				break;
			}

			case DiscoveryAnnouncement::DISCO_VERSION_01:
			{
				IBRCOMMON_LOGGER_DEBUG(60) << "beacon version 2 received" << IBRCOMMON_LOGGER_ENDL;

				stream.get((char&)announcement._flags);

				IBRCOMMON_LOGGER_DEBUG(65) << "beacon flags: " << hex << (int)announcement._flags << IBRCOMMON_LOGGER_ENDL;

				u_int16_t sn = 0;
				stream.read((char*)&sn, 2);

				// convert from network byte order
				u_int16_t sequencenumber = ntohs(sn);

				IBRCOMMON_LOGGER_DEBUG(65) << "beacon sequence number: " << sequencenumber << IBRCOMMON_LOGGER_ENDL;

				if (announcement._flags & DiscoveryAnnouncement::BEACON_CONTAINS_EID)
				{
					dtn::data::BundleString eid;
					stream >> eid;

					announcement._canonical_eid = dtn::data::EID((std::string)eid);

					IBRCOMMON_LOGGER_DEBUG(65) << "beacon eid: " << (std::string)eid << IBRCOMMON_LOGGER_ENDL;
				}

				if (announcement._flags & DiscoveryAnnouncement::BEACON_SERVICE_BLOCK)
				{
					// get the services
					list<DiscoveryService> &services = announcement._services;

					// read the number of services
					dtn::data::SDNV num_services;
					stream >> num_services;

					IBRCOMMON_LOGGER_DEBUG(65) << "beacon services (" << num_services.getValue() << "): " << IBRCOMMON_LOGGER_ENDL;

					// clear the services
					services.clear();

					for (unsigned int i = 0; i < num_services.getValue(); i++)
					{
						// decode the service blocks
						DiscoveryService service;
						stream >> service;
						services.push_back(service);

						IBRCOMMON_LOGGER_DEBUG(65) << "\t " << service.getName() << " [" << service.getParameters() << "]" << IBRCOMMON_LOGGER_ENDL;
					}
				}

				if (announcement._flags & DiscoveryAnnouncement::BEACON_BLOOMFILTER)
				{
					// TODO: read the bloomfilter
				}

				break;
			}

			case DiscoveryAnnouncement::DTND_IPDISCOVERY:
			{
				u_int8_t cl_type;
				u_int8_t interval;
				u_int16_t length;
				u_int32_t inet_addr;
				u_int16_t inet_port;
				u_int16_t eid_len;

				IBRCOMMON_LOGGER_DEBUG(60) << "beacon IPDiscovery (DTN2) frame received" << IBRCOMMON_LOGGER_ENDL;

				stream.read((char*)&cl_type, 1);
				stream.read((char*)&interval, 1);
				stream.read((char*)&length, 2);
				stream.read((char*)&inet_addr, 4);
				stream.read((char*)&inet_port, 2);
				stream.read((char*)&eid_len, 2);

				char eid[eid_len];
				stream.read((char*)&eid, eid_len);

				announcement._version = DiscoveryAnnouncement::DTND_IPDISCOVERY;
				announcement._canonical_eid = EID(std::string(eid));

				break;
			}

			default:
				IBRCOMMON_LOGGER_DEBUG(60) << "unknown beacon received" << IBRCOMMON_LOGGER_ENDL;

				// Error, throw Exception!
				throw InvalidProtocolException("The received data does not match the discovery protocol.");

				break;
			}

			return stream;
		}

		string DiscoveryAnnouncement::toString() const
		{
			stringstream ss;
			ss << "ANNOUNCE: " << _canonical_eid.getString();
			return ss.str();
		}
	}
}
