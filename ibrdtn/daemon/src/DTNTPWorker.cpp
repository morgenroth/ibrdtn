/*
 * DTNTPWorker.cpp
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

#include "DTNTPWorker.h"
#include "core/NodeEvent.h"
#include "core/BundleCore.h"
#include "core/TimeEvent.h"
#include "core/TimeAdjustmentEvent.h"
#include <ibrdtn/utils/Clock.h>
#include <ibrdtn/utils/Utils.h>
#include <ibrdtn/data/AgeBlock.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/ScopeControlHopLimitBlock.h>
#include <ibrcommon/TimeMeasurement.h>
#include <ibrcommon/Logger.h>

#include <sys/time.h>

namespace dtn
{
	namespace daemon
	{
		const unsigned int DTNTPWorker::PROTO_VERSION = 1;

		DTNTPWorker::DTNTPWorker()
		 : _conf(dtn::daemon::Configuration::getInstance().getTimeSync()), _qot_current_tic(0), _sigma(_conf.getSigma()),
		   _epsilon(1 / _sigma), _quality_diff(1 - _conf.getSyncLevel()), _sync_age(0)
		{
			AbstractWorker::initialize("/dtntp", 60, true);

			if (_conf.hasReference())
			{
				_last_sync.origin_quality = 0.0;
				_last_sync.peer_quality = 1.0;
				_last_sync.peer_timestamp = _last_sync.origin_timestamp;
			}
			else
			{
				_last_sync.origin_quality = 0.0;
				_last_sync.peer_quality = 0.0;
			}

			// set current quality to last sync quality
			dtn::utils::Clock::quality = _last_sync.peer_quality;

			if (_conf.syncOnDiscovery())
			{
				bindEvent(dtn::core::NodeEvent::className);
			}

			bindEvent(dtn::core::TimeEvent::className);

			// debug quality of time
			IBRCOMMON_LOGGER_DEBUG(10) << "quality of time is " << dtn::utils::Clock::quality << IBRCOMMON_LOGGER_ENDL;
		}

		DTNTPWorker::~DTNTPWorker()
		{
			unbindEvent(dtn::core::NodeEvent::className);
			unbindEvent(dtn::core::TimeEvent::className);
		}

		DTNTPWorker::TimeSyncMessage::TimeSyncMessage()
		 : type(TIMESYNC_REQUEST), origin_quality(dtn::utils::Clock::quality), peer_quality(0.0)
		{
			timerclear(&origin_timestamp);
			timerclear(&peer_timestamp);

			dtn::utils::Clock::gettimeofday(&origin_timestamp);
		}

		DTNTPWorker::TimeSyncMessage::~TimeSyncMessage()
		{
		}

		std::ostream &operator<<(std::ostream &stream, const DTNTPWorker::TimeSyncMessage &obj)
		{
			std::stringstream ss;

			stream << (char)obj.type;

			ss.clear(); ss.str(""); ss << obj.origin_quality;
			stream << dtn::data::BundleString(ss.str());

			stream << dtn::data::SDNV(obj.origin_timestamp.tv_sec);
			stream << dtn::data::SDNV(obj.origin_timestamp.tv_usec);

			ss.clear(); ss.str(""); ss << obj.peer_quality;
			stream << dtn::data::BundleString(ss.str());

			stream << dtn::data::SDNV(obj.peer_timestamp.tv_sec);
			stream << dtn::data::SDNV(obj.peer_timestamp.tv_usec);

			return stream;
		}

		std::istream &operator>>(std::istream &stream, DTNTPWorker::TimeSyncMessage &obj)
		{
			char type = 0;
			std::stringstream ss;
			dtn::data::BundleString bs;
			dtn::data::SDNV sdnv;

			stream >> type;
			obj.type = DTNTPWorker::TimeSyncMessage::MSG_TYPE(type);

			stream >> bs;
			ss.clear();
			ss.str((const std::string&)bs);
			ss >> obj.origin_quality;

			stream >> sdnv; obj.origin_timestamp.tv_sec = sdnv.getValue();
			stream >> sdnv; obj.origin_timestamp.tv_usec = sdnv.getValue();

			stream >> bs;
			ss.clear();
			ss.str((const std::string&)bs);
			ss >> obj.peer_quality;

			stream >> sdnv; obj.peer_timestamp.tv_sec = sdnv.getValue();
			stream >> sdnv; obj.peer_timestamp.tv_usec = sdnv.getValue();

			return stream;
		}

		void DTNTPWorker::raiseEvent(const dtn::core::Event *evt)
		{
			try {
				const dtn::core::TimeEvent &t = dynamic_cast<const dtn::core::TimeEvent&>(*evt);

				if (t.getAction() == dtn::core::TIME_SECOND_TICK)
				{
					ibrcommon::MutexLock l(_sync_lock);

					// remove outdated blacklist entries
					{
						ibrcommon::MutexLock l(_blacklist_lock);
						for (std::map<EID, size_t>::iterator iter = _sync_blacklist.begin(); iter != _sync_blacklist.end(); iter++)
						{
							size_t bl_age = (*iter).second;

							// do not query again if the blacklist entry is valid
							if (bl_age < t.getUnixTimestamp())
							{
								_sync_blacklist.erase((*iter).first);
							}
						}
					}

					if ((_conf.getQualityOfTimeTick() > 0) && !_conf.hasReference())
					{
						/**
						 * decrease the quality of time each x tics
						 */
						if (_qot_current_tic == _conf.getQualityOfTimeTick())
						{
							// get current time values
							_sync_age++;

							// adjust own quality of time
							dtn::utils::Clock::quality = _last_sync.peer_quality * (1 / ::pow(_sigma, _sync_age) );

							// debug quality of time
							IBRCOMMON_LOGGER_DEBUG(25) << "new quality = " << _last_sync.peer_quality << " * (1 / (" << _sigma << " ^ " << _sync_age << "))" << IBRCOMMON_LOGGER_ENDL;
							IBRCOMMON_LOGGER_DEBUG(25) << "new quality of time is " << dtn::utils::Clock::quality << IBRCOMMON_LOGGER_ENDL;

							// reset the tick counter
							_qot_current_tic = 0;
						}
						else
						{
							// increment the tick counter
							_qot_current_tic++;
						}
					}
					else
					{
						/**
						 * evaluate the current local time
						 */
						if (dtn::utils::Clock::quality == 0)
						{
							if (t.getTimestamp() > 0)
							{
								dtn::utils::Clock::quality = 1;
								IBRCOMMON_LOGGER(warning) << "The local clock seems to be okay again. Expiration enabled." << IBRCOMMON_LOGGER_ENDL;
							}
						}
					}
				}
			} catch (const std::bad_cast&) { };

			try {
				const dtn::core::NodeEvent &n = dynamic_cast<const dtn::core::NodeEvent&>(*evt);
				const dtn::core::Node &node = n.getNode();

				if (n.getAction() == dtn::core::NODE_INFO_UPDATED)
				{
					// only query for time sync if the other node supports this
					if (!node.has("dtntp")) return;

					// get discovery attribute
					const std::list<dtn::core::Node::Attribute> attrs = node.get("dtntp");

					// decode attribute parameter
					unsigned int version = 0;
					size_t timestamp = 0;
					float quality = 0.0;
					decode(attrs.front(), version, timestamp, quality);

					// we do only support version = 1
					if (version != 1) return;

					// do not sync if the quality is worse than ours
					if ((quality * _quality_diff) <= dtn::utils::Clock::quality) return;

					// get the EID of the peer
					const dtn::data::EID &peer = n.getNode().getEID();

					// check sync blacklist
					{
						ibrcommon::MutexLock l(_blacklist_lock);
						if (_sync_blacklist.find(peer) != _sync_blacklist.end())
						{
							size_t bl_age = _sync_blacklist[peer];

							// do not query again if the blacklist entry is valid
							if (bl_age > dtn::utils::Clock::getUnixTimestamp())
							{
								return;
							}
						}

						// create a new blacklist entry
						_sync_blacklist[peer] = dtn::utils::Clock::getUnixTimestamp() + 60;
					}

					// send a time sync bundle
					dtn::data::Bundle b;

					// add an age block
					b.push_back<dtn::data::AgeBlock>();

					ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

					// create the payload of the message
					{
						ibrcommon::BLOB::iostream stream = ref.iostream();

						// create a new timesync request
						TimeSyncMessage msg;

						// write the message
						(*stream) << msg;
					}

					// add the payload to the message
					b.push_back(ref);

					// set the source and destination
					b._source = dtn::core::BundleCore::local + "/dtntp";
					b._destination = peer + "/dtntp";

					// set high priority
					b.set(dtn::data::PrimaryBlock::PRIORITY_BIT1, false);
					b.set(dtn::data::PrimaryBlock::PRIORITY_BIT2, true);

					// set the the destination as singleton receiver
					b.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);

					// set the lifetime of the bundle to 60 seconds
					b._lifetime = 60;

					// add a schl block
					dtn::data::ScopeControlHopLimitBlock &schl = b.push_front<dtn::data::ScopeControlHopLimitBlock>();
					schl.setLimit(1);

					transmit(b);
				}
			} catch (const std::bad_cast&) { };
		}

		void DTNTPWorker::update(const ibrcommon::vinterface&, std::string &name, std::string &data) throw(NoServiceHereException)
		{
			if (!_conf.sendDiscoveryAnnouncements()) throw NoServiceHereException("Discovery of time sync mechanisms disabled.");

			std::stringstream ss;
			ss << "version=" << PROTO_VERSION << ";quality=" << dtn::utils::Clock::quality << ";timestamp=" << dtn::utils::Clock::getTime() << ";";
			name = "dtntp";
			data = ss.str();
		}

		void DTNTPWorker::decode(const dtn::core::Node::Attribute &attr, unsigned int &version, size_t &timestamp, float &quality)
		{
			// parse parameters
			std::vector<std::string> parameters = dtn::utils::Utils::tokenize(";", attr.value);
			std::vector<std::string>::const_iterator param_iter = parameters.begin();

			while (param_iter != parameters.end())
			{
				std::vector<std::string> p = dtn::utils::Utils::tokenize("=", (*param_iter));

				if (p[0].compare("version") == 0)
				{
					std::stringstream ss(p[1]);
					ss >> version;
				}

				if (p[0].compare("timestamp") == 0)
				{
					std::stringstream ss(p[1]);
					ss >> timestamp;
				}

				if (p[0].compare("quality") == 0)
				{
					std::stringstream ss(p[1]);
					ss >> quality;
				}

				param_iter++;
			}
		}

//		void DTNTPWorker::shared_sync(const TimeBeacon &beacon)
//		{
//			// do not sync if we are a reference
//			if (_conf.hasReference()) return;
//
//			// adjust own quality of time
//			TimeBeacon current; get_time(current);
//			long int tdiff = current.sec - _last_sync.sec;
//
//			ibrcommon::MutexLock l(_sync_lock);
//
//			// if we have no time, take it
//			if (_last_sync.quality == 0)
//			{
//				dtn::utils::Clock::quality = beacon.quality * _epsilon;
//			}
//			// if our last sync is older than one second...
//			else if (tdiff > 0)
//			{
//				// sync our clock
//				double ext_faktor = beacon.quality / (beacon.quality + dtn::utils::Clock::quality);
//				double int_faktor = dtn::utils::Clock::quality / (beacon.quality + dtn::utils::Clock::quality);
//
//				// set the new time values
//				set_time( 	(beacon.sec * ext_faktor) + (current.sec * int_faktor),
//							(beacon.usec * ext_faktor) + (current.usec * int_faktor)
//						);
//			}
//			else
//			{
//				return;
//			}
//		}

		void DTNTPWorker::sync(const TimeSyncMessage &msg, struct timeval &offset)
		{
			// do not sync if we are a reference
			if (_conf.hasReference()) return;

			ibrcommon::MutexLock l(_sync_lock);

			// if the received quality of time is worse than ours, ignore it
			if (dtn::utils::Clock::quality >= msg.peer_quality) return;

			// the values are better, adapt them
			float rating = msg.peer_quality * _epsilon;

			// trigger time adjustment event
			dtn::core::TimeAdjustmentEvent::raise(offset, rating);

			// remember the last sync
			_last_sync = msg;
			_sync_age = 0;
		}

		void DTNTPWorker::callbackBundleReceived(const Bundle &b)
		{
			// do not sync with ourselves
			if (b._source.getNode() == dtn::core::BundleCore::local) return;

			try {
				// read payload block
				const dtn::data::PayloadBlock &p = b.getBlock<dtn::data::PayloadBlock>();

				// read the type of the message
				char type = 0; (*p.getBLOB().iostream()).get(type);

				switch (type)
				{
					case TimeSyncMessage::TIMESYNC_REQUEST:
					{
						dtn::data::Bundle response = b;
						response.relabel();

						// set the lifetime of the bundle to 60 seconds
						response._lifetime = 60;

						// switch the source and destination
						response._source = b._destination;
						response._destination = b._source;
						
						// set high priority
						response.set(dtn::data::PrimaryBlock::PRIORITY_BIT1, false);
						response.set(dtn::data::PrimaryBlock::PRIORITY_BIT2, true);

						// set the the destination as singleton receiver
						response.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);

						// modify the payload - locked
						{
							ibrcommon::BLOB::Reference ref = p.getBLOB();
							ibrcommon::BLOB::iostream stream = ref.iostream();

							// read the timesync message
							TimeSyncMessage msg;
							(*stream) >> msg;

							// clear the payload
							stream.clear();

							// fill in the own values
							msg.type = TimeSyncMessage::TIMESYNC_RESPONSE;
							msg.peer_quality = dtn::utils::Clock::quality;
							dtn::utils::Clock::gettimeofday(&msg.peer_timestamp);

							// write the response
							(*stream) << msg;
						}

						// add a second age block
						response.push_front<dtn::data::AgeBlock>();

						// modify the old schl block or add a new one
						try {
							dtn::data::ScopeControlHopLimitBlock &schl = response.getBlock<dtn::data::ScopeControlHopLimitBlock>();
							schl.setLimit(1);
						} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
							dtn::data::ScopeControlHopLimitBlock &schl = response.push_front<dtn::data::ScopeControlHopLimitBlock>();
							schl.setLimit(1);
						};

						// send the response
						transmit(response);
						break;
					}

					case TimeSyncMessage::TIMESYNC_RESPONSE:
					{
						// read the ageblock of the bundle
						const std::list<const dtn::data::AgeBlock*> ageblocks = b.getBlocks<dtn::data::AgeBlock>();
						const dtn::data::AgeBlock &peer_age = (*ageblocks.front());
						const dtn::data::AgeBlock &origin_age = (*ageblocks.back());

						timeval tv_age; timerclear(&tv_age);
						tv_age.tv_usec = origin_age.getMicroseconds();

						ibrcommon::BLOB::Reference ref = p.getBLOB();
						ibrcommon::BLOB::iostream stream = ref.iostream();

						TimeSyncMessage msg; (*stream) >> msg;

						timeval tv_local, rtt;
						dtn::utils::Clock::gettimeofday(&tv_local);

						// get the RTT
						timersub(&tv_local, &msg.origin_timestamp, &rtt);

						// get the propagation delay
						timeval prop_delay;
						timersub(&rtt, &tv_age, &prop_delay);

						// half the prop delay
						prop_delay.tv_sec /= 2;
						prop_delay.tv_usec /= 2;

						timeval sync_delay;
						timerclear(&sync_delay);
						sync_delay.tv_usec = peer_age.getMicroseconds() + prop_delay.tv_usec;

						timeval peer_timestamp;
						timeradd(&msg.peer_timestamp, &sync_delay, &peer_timestamp);

						timeval offset;
						timersub(&tv_local, &peer_timestamp, &offset);

						// print out offset to the local clock
						IBRCOMMON_LOGGER(info) << "DT-NTP bundle received; rtt = " << rtt.tv_sec << "s " << rtt.tv_usec << "us; prop. delay = " << prop_delay.tv_sec << "s " << prop_delay.tv_usec << "us; clock of " << b._source.getNode().getString() << " has a offset of " << offset.tv_sec << "s " << offset.tv_usec << "us" << IBRCOMMON_LOGGER_ENDL;

						// sync to this time message
						sync(msg, offset);

						// remove the blacklist entry
						ibrcommon::MutexLock l(_blacklist_lock);
						_sync_blacklist.erase(b._source.getNode());

						break;
					}
				}
			} catch (const ibrcommon::Exception&) { };
		}
	}
}
