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
#include "core/EventDispatcher.h"
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
		const std::string DTNTPWorker::TAG = "DTNTPWorker";

		DTNTPWorker::DTNTPWorker()
		 : _sync_threshold(0.15f), _announce_rating(false), _base_rating(0.0), _psi(0.99), _sigma(1.0), _sync(false)
		{
			AbstractWorker::initialize("/dtntp", 60, true);

			// initialize the last sync time to zero
			timerclear(&_last_sync_time);

			// get global configuration for time synchronization
			const dtn::daemon::Configuration::TimeSync &conf = dtn::daemon::Configuration::getInstance().getTimeSync();

			if (conf.hasReference())
			{
				// set clock rating to 1 since this node has a reference clock
				_base_rating = 1.0;

				// evaluate the current local time
				if (dtn::utils::Clock::getTime() > 0) {
					dtn::utils::Clock::setRating(1.0);
				} else {
					dtn::utils::Clock::setRating(0.0);
					IBRCOMMON_LOGGER_TAG(DTNTPWorker::TAG, warning) << "The local clock seems to be wrong. Expiration disabled." << IBRCOMMON_LOGGER_ENDL;
				}
			} else {
				dtn::utils::Clock::setRating(0.0);
				_sigma = conf.getSigma();
				_psi = conf.getPsi();
			}

			// check if we should announce our own rating via discovery
			_announce_rating = conf.sendDiscoveryAnnouncements();

			// store the sync threshold locally
			_sync_threshold = conf.getSyncLevel();

			// synchronize with other nodes
			_sync  = conf.doSync();

			if (_sync) {
				IBRCOMMON_LOGGER_TAG(DTNTPWorker::TAG, info) << "Time-Synchronization enabled: " << (conf.hasReference() ? "master mode" : "slave mode") << IBRCOMMON_LOGGER_ENDL;
			}

			dtn::core::EventDispatcher<dtn::core::TimeEvent>::add(this);
		}

		DTNTPWorker::~DTNTPWorker()
		{
			dtn::core::EventDispatcher<dtn::core::TimeEvent>::remove(this);
		}

		DTNTPWorker::TimeSyncMessage::TimeSyncMessage()
		 : type(TIMESYNC_REQUEST), origin_rating(dtn::utils::Clock::getRating()), peer_rating(0.0)
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

			stream << dtn::data::Number(obj.origin_timestamp.tv_sec);
			stream << dtn::data::Number(obj.origin_timestamp.tv_usec);

			ss.clear(); ss.str(""); ss << obj.peer_rating;
			stream << dtn::data::BundleString(ss.str());

			stream << dtn::data::Number(obj.peer_timestamp.tv_sec);
			stream << dtn::data::Number(obj.peer_timestamp.tv_usec);

			return stream;
		}

		std::istream &operator>>(std::istream &stream, DTNTPWorker::TimeSyncMessage &obj)
		{
			char type = 0;
			std::stringstream ss;
			dtn::data::BundleString bs;
			dtn::data::Number sdnv;

			stream >> type;
			obj.type = DTNTPWorker::TimeSyncMessage::MSG_TYPE(type);

			stream >> bs;
			ss.clear();
			ss.str((const std::string&)bs);
			ss >> obj.origin_rating;

			stream >> sdnv;
			obj.origin_timestamp.tv_sec = sdnv.get<time_t>();

			stream >> sdnv;
			obj.origin_timestamp.tv_usec = sdnv.get<suseconds_t>();

			stream >> bs;
			ss.clear();
			ss.str((const std::string&)bs);
			ss >> obj.peer_rating;

			stream >> sdnv;
			obj.peer_timestamp.tv_sec = sdnv.get<time_t>();

			stream >> sdnv;
			obj.peer_timestamp.tv_usec = sdnv.get<suseconds_t>();

			return stream;
		}

		void DTNTPWorker::raiseEvent(const dtn::core::Event *evt) throw ()
		{
			try {
				const dtn::core::TimeEvent &t = dynamic_cast<const dtn::core::TimeEvent&>(*evt);

				if (t.getAction() != dtn::core::TIME_SECOND_TICK) return;

				ibrcommon::MutexLock l(_sync_lock);

				// remove outdated blacklist entries
				{
					ibrcommon::MutexLock l(_blacklist_lock);
					for (blacklist_map::iterator iter = _sync_blacklist.begin(); iter != _sync_blacklist.end();)
					{
						const dtn::data::Timestamp &bl_age = (*iter).second;

						// do not query again if the blacklist entry is valid
						if (bl_age < t.getUnixTimestamp()) {
							_sync_blacklist.erase(iter++);
						} else {
							++iter;
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
					if (dtn::utils::Clock::getRating() == 0)
					{
						if (t.getTimestamp() > 0)
						{
							dtn::utils::Clock::setRating(1.0);
							IBRCOMMON_LOGGER_TAG(DTNTPWorker::TAG, warning) << "The local clock seems to be okay again. Expiration enabled." << IBRCOMMON_LOGGER_ENDL;
						}
					}
				}
				// if we are not a reference then update the local rating if we're not a reference node
				else
				{
					// before we can age our rating we should have been synchronized at least one time
					if (timerisset(&_last_sync_time))
					{
						timeval tv_now;
						dtn::utils::Clock::gettimeofday(&tv_now);

						double last_sync = dtn::utils::Clock::toDouble(_last_sync_time);
						double now = dtn::utils::Clock::toDouble(tv_now);

						// the last sync must be in the past
						if (last_sync < now)
						{
							// calculate the new clock rating
							double timediff = now - last_sync;
							dtn::utils::Clock::setRating(_base_rating * (1.0 / (::pow(_sigma, timediff))));
						}
					}
				}

				// if synchronization is enabled
				if (_sync)
				{
					// search for other nodes with better credentials
					const std::set<dtn::core::Node> nodes = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();
					for (std::set<dtn::core::Node>::const_iterator iter = nodes.begin(); iter != nodes.end(); ++iter) {
						if (shouldSyncWith(*iter)) {
							syncWith(*iter);
						}
					}
				}
			} catch (const std::bad_cast&) { };
		}

		bool DTNTPWorker::shouldSyncWith(const dtn::core::Node &node) const
		{
			// only query for time sync if the other node supports this
			if (!node.has("dtntp")) return false;

			// get discovery attribute
			const std::list<dtn::core::Node::Attribute> attrs = node.get("dtntp");

			if (attrs.empty()) return false;

			// decode attribute parameter
			unsigned int version = 0;
			dtn::data::Timestamp timestamp = 0;
			float quality = 0.0;
			decode(attrs.front(), version, timestamp, quality);

			// we do only support version = 1
			if (version != 1) return false;

			// do not sync if the timestamps are equal in seconds
			if (timestamp == dtn::utils::Clock::getTime()) return false;

			// do not sync if the quality is worse than ours
			if ((quality * (1 - _sync_threshold)) <= dtn::utils::Clock::getRating()) return false;

			return true;
		}

		void DTNTPWorker::syncWith(const dtn::core::Node &node)
		{
			// get the EID of the peer
			const dtn::data::EID &peer = node.getEID();

			// check sync blacklist
			{
				ibrcommon::MutexLock l(_blacklist_lock);
				if (_sync_blacklist.find(peer) != _sync_blacklist.end())
				{
					const dtn::data::Timestamp &bl_age = _sync_blacklist[peer];

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

			try {
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

				// set the source
				if (dtn::core::BundleCore::local.isCompressable()) {
					b.source = dtn::core::BundleCore::local.add(dtn::core::BundleCore::local.getDelimiter() + "60");
				} else {
					b.source = dtn::core::BundleCore::local.add(dtn::core::BundleCore::local.getDelimiter() + "dtntp");
				}

				// set the destination
				if (peer.isCompressable()) {
					b.destination = peer.add(peer.getDelimiter() + "60");
				} else {
					b.destination = peer.add(peer.getDelimiter() + "dtntp");
				}

				// set high priority
				b.set(dtn::data::PrimaryBlock::PRIORITY_BIT1, false);
				b.set(dtn::data::PrimaryBlock::PRIORITY_BIT2, true);

				// set the the destination as singleton receiver
				b.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);

				// set the lifetime of the bundle to 60 seconds
				b.lifetime = 60;

				// add a schl block
				dtn::data::ScopeControlHopLimitBlock &schl = b.push_front<dtn::data::ScopeControlHopLimitBlock>();
				schl.setLimit(1);

				transmit(b);
			} catch (const ibrcommon::IOException &ex) {
				IBRCOMMON_LOGGER_TAG(DTNTPWorker::TAG, error) << "error while synchronizing, Exception: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}


		void DTNTPWorker::update(const ibrcommon::vinterface&, DiscoveryAnnouncement &announcement) throw(NoServiceHereException)
		{
			if (!_announce_rating) throw NoServiceHereException("Discovery of time sync mechanisms disabled.");

			std::stringstream ss;
			ss << "version=" << PROTO_VERSION << ";quality=" << dtn::utils::Clock::getRating() << ";timestamp=" << dtn::utils::Clock::getTime().toString() << ";";
			announcement.addService( DiscoveryService("dtntp", ss.str()));
		}

		void DTNTPWorker::decode(const dtn::core::Node::Attribute &attr, unsigned int &version, dtn::data::Timestamp &timestamp, float &quality) const
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
					timestamp.fromString(p[1]);
				}

				if (p[0].compare("quality") == 0)
				{
					std::stringstream ss(p[1]);
					ss >> quality;
				}

				++param_iter;
			}
		}

		bool DTNTPWorker::hasReference() const {
			return (_sigma == 1.0);
		}

		void DTNTPWorker::sync(const TimeSyncMessage &msg, const struct timeval &tv_offset, const struct timeval &tv_local, const struct timeval &tv_remote)
		{
			// do not sync if we are a reference
			if (hasReference()) return;

			ibrcommon::MutexLock l(_sync_lock);

			// if the received quality of time is worse than ours, ignore it
			if (dtn::utils::Clock::getRating() >= msg.peer_rating) return;

			double local_time = dtn::utils::Clock::toDouble(tv_local);
			double remote_time = dtn::utils::Clock::toDouble(tv_remote);

			// adjust sigma if we sync'd at least twice
			if (timerisset(&_last_sync_time))
			{
				double lastsync_time = dtn::utils::Clock::toDouble(_last_sync_time);

				// adjust sigma
				double t_stable = local_time - lastsync_time;

				if (t_stable > 0.0) {
					double sigma_base = (1 / ::pow(_psi, 1/t_stable));
					double sigma_adjustment = ::fabs(remote_time - local_time) / t_stable * msg.peer_rating;
					_sigma = sigma_base + sigma_adjustment;

					IBRCOMMON_LOGGER_DEBUG_TAG(DTNTPWorker::TAG, 25) << "new sigma: " << _sigma << IBRCOMMON_LOGGER_ENDL;
				}
			}

			if (local_time > remote_time) {
				// determine the new base rating
				_base_rating = msg.peer_rating * (remote_time / local_time);
			} else {
				// determine the new base rating
				_base_rating = msg.peer_rating * (local_time / remote_time);
			}

			// trigger time adjustment event
			dtn::core::TimeAdjustmentEvent::raise(tv_offset, _base_rating);

			// store the timestamp of the last synchronization
			dtn::utils::Clock::gettimeofday(&_last_sync_time);
		}

		void DTNTPWorker::callbackBundleReceived(const Bundle &b)
		{
			// do not sync with ourselves
			if (b.source.getNode() == dtn::core::BundleCore::local) return;

			try {
				// read payload block
				const dtn::data::PayloadBlock &p = b.find<dtn::data::PayloadBlock>();

				// read the type of the message
				char type = 0; (*p.getBLOB().iostream()).get(type);

				switch (type)
				{
					case TimeSyncMessage::TIMESYNC_REQUEST:
					{
						dtn::data::Bundle response = b;
						response.relabel();

						// set the lifetime of the bundle to 60 seconds
						response.lifetime = 60;

						// switch the source and destination
						response.source = b.destination;
						response.destination = b.source;
						
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
							msg.peer_rating = dtn::utils::Clock::getRating();
							dtn::utils::Clock::gettimeofday(&msg.peer_timestamp);

							// write the response
							(*stream) << msg;
						}

						// add a second age block
						response.push_front<dtn::data::AgeBlock>();

						// modify the old schl block or add a new one
						try {
							dtn::data::ScopeControlHopLimitBlock &schl = response.find<dtn::data::ScopeControlHopLimitBlock>();
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
						dtn::data::Bundle::const_find_iterator age_it(b.begin(), dtn::data::AgeBlock::BLOCK_TYPE);

						if (!age_it.next(b.end())) throw ibrcommon::Exception("first ageblock missing");
						const dtn::data::AgeBlock &peer_age = dynamic_cast<const dtn::data::AgeBlock&>(**age_it);

						if (!age_it.next(b.end())) throw ibrcommon::Exception("second ageblock missing");
						const dtn::data::AgeBlock &origin_age = dynamic_cast<const dtn::data::AgeBlock&>(**age_it);

						timeval tv_rtt_measured, tv_local_timestamp, tv_rtt, tv_prop_delay, tv_sync_delay, tv_peer_timestamp, tv_offset;

						timerclear(&tv_rtt_measured);
						tv_rtt_measured.tv_sec = origin_age.getSeconds().get<time_t>();
						tv_rtt_measured.tv_usec = origin_age.getMicroseconds().get<suseconds_t>() % 1000000;

						ibrcommon::BLOB::Reference ref = p.getBLOB();
						ibrcommon::BLOB::iostream stream = ref.iostream();

						// parse the received time sync message
						TimeSyncMessage msg; (*stream) >> msg;

						// store the current time in tv_local
						dtn::utils::Clock::gettimeofday(&tv_local_timestamp);

						// determine the RTT of the message exchange
						timersub(&tv_local_timestamp, &msg.origin_timestamp, &tv_rtt);
						double rtt = dtn::utils::Clock::toDouble(tv_rtt);

						// abort here if the rtt is negative or zero!
						if (rtt <= 0.0) {
							IBRCOMMON_LOGGER_TAG(DTNTPWorker::TAG, warning) << "RTT " << rtt << " is too small" << IBRCOMMON_LOGGER_ENDL;
							break;
						}

						double prop_delay = 0.0;

						// assume zero prop. delay if rtt is smaller than the
						// time measured by the age block
						double rtt_measured = dtn::utils::Clock::toDouble(tv_rtt_measured);
						if (rtt <= rtt_measured) {
							timerclear(&tv_prop_delay);
							IBRCOMMON_LOGGER_TAG(DTNTPWorker::TAG, warning) << "Prop. delay " << prop_delay << " is smaller than the tracked time (" << rtt_measured << ")" << IBRCOMMON_LOGGER_ENDL;
						} else {
							timersub(&tv_rtt, &tv_rtt_measured, &tv_prop_delay);
							prop_delay = dtn::utils::Clock::toDouble(tv_prop_delay);
						}

						// half the prop delay
						tv_prop_delay.tv_sec /= 2;
						tv_prop_delay.tv_usec /= 2;

						// copy time interval tracked with the ageblock of the peer
						timerclear(&tv_sync_delay);
						tv_sync_delay.tv_sec = peer_age.getSeconds().get<time_t>();
						tv_sync_delay.tv_usec = peer_age.getMicroseconds().get<suseconds_t>() % 1000000;

						// add sync delay to the peer timestamp
						timeradd(&msg.peer_timestamp, &tv_sync_delay, &tv_peer_timestamp);

						// add propagation delay to the peer timestamp
						timeradd(&msg.peer_timestamp, &tv_prop_delay, &tv_peer_timestamp);

						// calculate offset
						timersub(&tv_local_timestamp, &tv_peer_timestamp, &tv_offset);

						// print out offset to the local clock
						IBRCOMMON_LOGGER_TAG(DTNTPWorker::TAG, info) << "DT-NTP bundle received; rtt = " << rtt << "s; prop. delay = " << prop_delay << "s; clock of " << b.source.getNode().getString() << " has a offset of " << dtn::utils::Clock::toDouble(tv_offset) << "s" << IBRCOMMON_LOGGER_ENDL;

						// sync to this time message
						sync(msg, tv_offset, tv_local_timestamp, tv_peer_timestamp);

						// remove the blacklist entry
						ibrcommon::MutexLock l(_blacklist_lock);
						_sync_blacklist.erase(b.source.getNode());

						break;
					}
				}
			} catch (const ibrcommon::Exception&) { };
		}
	}
}
