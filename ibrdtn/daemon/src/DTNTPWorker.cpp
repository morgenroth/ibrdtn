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
		 : _sync_threshold(0.15), _announce_rating(false), _base_rating(0.0), _psi(0.99), _sigma(1.0)
		{
			AbstractWorker::initialize("/dtntp", 60, true);

			// get global configuration for time synchronization
			const dtn::daemon::Configuration::TimeSync &conf = dtn::daemon::Configuration::getInstance().getTimeSync();

			if (conf.hasReference())
			{
				// evaluate the current local time
				if (dtn::utils::Clock::getTime() > 0) {
					_base_rating = 1.0;
				} else {
					IBRCOMMON_LOGGER(warning) << "The local clock seems to be wrong. Expiration disabled." << IBRCOMMON_LOGGER_ENDL;
				}
			} else {
				_sigma = conf.getSigma();
				_psi = conf.getPsi();
			}

			// check if we should announce our own rating via discovery
			_announce_rating = conf.sendDiscoveryAnnouncements();

			// store the sync threshold locally
			_sync_threshold = conf.getSyncLevel();

			// subscribe to NodeEvent is we have to react on discovered nodes
			if (conf.syncOnDiscovery())
			{
				bindEvent(dtn::core::NodeEvent::className);
			}

			bindEvent(dtn::core::TimeEvent::className);

			// debug quality of time
			IBRCOMMON_LOGGER_DEBUG(10) << "quality of time is " << dtn::utils::Clock::rating << IBRCOMMON_LOGGER_ENDL;
		}

		DTNTPWorker::~DTNTPWorker()
		{
			unbindEvent(dtn::core::NodeEvent::className);
			unbindEvent(dtn::core::TimeEvent::className);
		}

		DTNTPWorker::TimeSyncMessage::TimeSyncMessage()
		 : type(TIMESYNC_REQUEST), origin_rating(dtn::utils::Clock::rating), peer_rating(0.0)
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

			ss.clear(); ss.str(""); ss << obj.origin_rating;
			stream << dtn::data::BundleString(ss.str());

			stream << dtn::data::SDNV(obj.origin_timestamp.tv_sec);
			stream << dtn::data::SDNV(obj.origin_timestamp.tv_usec);

			ss.clear(); ss.str(""); ss << obj.peer_rating;
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
			ss >> obj.origin_rating;

			stream >> sdnv; obj.origin_timestamp.tv_sec = sdnv.getValue();
			stream >> sdnv; obj.origin_timestamp.tv_usec = sdnv.getValue();

			stream >> bs;
			ss.clear();
			ss.str((const std::string&)bs);
			ss >> obj.peer_rating;

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

					// if we are a reference node, we have to watch on our clock
					// do some plausibility checks here
					if (hasReference())
					{
						/**
						 * evaluate the current local time
						 */
						if (dtn::utils::Clock::rating == 0)
						{
							if (t.getTimestamp() > 0)
							{
								dtn::utils::Clock::rating = 1;
								IBRCOMMON_LOGGER(warning) << "The local clock seems to be okay again. Expiration enabled." << IBRCOMMON_LOGGER_ENDL;
							}
						}
					}
					// if we are not a reference then update the local rating if we're not a reference node
					else
					{
						ssize_t now = dtn::utils::Clock::getTime();

						// before we can age our rating we should have been synchronized at least one time
						if ((_last_sync_time.tv_sec > 0) && (_last_sync_time.tv_sec < now)) {
							// calculate the new clock rating
							dtn::utils::Clock::rating = _base_rating * (1.0 / (::pow(_sigma, now - _last_sync_time.tv_sec)));

							// debug quality of time
							IBRCOMMON_LOGGER_DEBUG(25) << "clock rating is " << dtn::utils::Clock::rating << IBRCOMMON_LOGGER_ENDL;
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

					// do not sync if the timestamps are equal in seconds
					if (timestamp == dtn::utils::Clock::getTime()) return;

					// do not sync if the quality is worse than ours
					if ((quality * (1 - _sync_threshold)) <= dtn::utils::Clock::rating) return;

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
			if (!_announce_rating) throw NoServiceHereException("Discovery of time sync mechanisms disabled.");

			std::stringstream ss;
			ss << "version=" << PROTO_VERSION << ";quality=" << dtn::utils::Clock::rating << ";timestamp=" << dtn::utils::Clock::getTime() << ";";
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

		bool DTNTPWorker::hasReference() const {
			return (_sigma == 1.0);
		}

		double DTNTPWorker::toDouble(const timeval &val) const {
			return val.tv_sec + (val.tv_usec / 1000000.0);
		}

		void DTNTPWorker::sync(const TimeSyncMessage &msg, const struct timeval &offset, const struct timeval &local, const struct timeval &remote)
		{
			// do not sync if we are a reference
			if (hasReference()) return;

			ibrcommon::MutexLock l(_sync_lock);

			// if the received quality of time is worse than ours, ignore it
			if (dtn::utils::Clock::rating >= msg.peer_rating) return;

			double local_time = toDouble(local);
			double remote_time = toDouble(remote);
			double lastsync_time = toDouble(_last_sync_time);

			// adjust sigma
			{
				double t_stable = local_time - lastsync_time;
				double sigma_base = (1 / ::pow(_psi, 1/t_stable));

				_sigma = ::abs(remote_time - local_time) / (local_time - lastsync_time) * msg.peer_rating;
				_sigma += sigma_base;
			}

			if (local_time > remote_time) {
				// determine the new base rating
				_base_rating = msg.peer_rating * (remote_time / local_time);
			} else {
				// determine the new base rating
				_base_rating = msg.peer_rating * (local_time / remote_time);
			}

			// trigger time adjustment event
			dtn::core::TimeAdjustmentEvent::raise(offset, _base_rating);

			// store the timestamp of the last synchronization
			dtn::utils::Clock::gettimeofday(&_last_sync_time);
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
							msg.peer_rating = dtn::utils::Clock::rating;
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
						sync(msg, offset, tv_local, peer_timestamp);

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
