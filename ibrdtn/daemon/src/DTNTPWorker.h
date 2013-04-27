/*
 * DTNTPWorker.h
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

#ifndef DTNTPWORKER_H_
#define DTNTPWORKER_H_

#include "core/AbstractWorker.h"
#include "core/EventReceiver.h"
#include "net/DiscoveryServiceProvider.h"
#include "Configuration.h"

namespace dtn
{
	namespace daemon
	{
		class DTNTPWorker : public dtn::core::AbstractWorker, public dtn::core::EventReceiver, public dtn::net::DiscoveryServiceProvider
		{
		public:
			/**
			 * Constructor
			 */
			DTNTPWorker();

			/**
			 * Destructor
			 */
			virtual ~DTNTPWorker();

			/**
			 * This method is called every time a bundles is received for this endpoint.
			 * @param b
			 */
			void callbackBundleReceived(const Bundle &b);

			/**
			 * This method is called by the EventSwitch.
			 * @param evt
			 */
			void raiseEvent(const dtn::core::Event *evt) throw ();

			/**
			 * This message is called by the discovery module.
			 * @param iface
			 * @param name
			 * @param data
			 */
			void update(const ibrcommon::vinterface &iface, DiscoveryAnnouncement &announcement)
				throw(NoServiceHereException);

//			/**
//			 * Determine the current local clock rating
//			 * @return The rating as double value.
//			 */
//			double getClockRating() const;

			/**
			 * TimeSyncMessage
			 * This class represent a sync message which is used to exchange data about
			 * the current clock state. During a short request-response contact the offset
			 * between two clock can be determined.
			 */
			class TimeSyncMessage
			{
			public:
				enum MSG_TYPE
				{
					TIMESYNC_REQUEST = 1,
					TIMESYNC_RESPONSE = 2
				};

				TimeSyncMessage();
				~TimeSyncMessage();

				MSG_TYPE type;

				timeval origin_timestamp;
				float origin_rating;

				timeval peer_timestamp;
				float peer_rating;

				friend std::ostream &operator<<(std::ostream &stream, const DTNTPWorker::TimeSyncMessage &obj);
				friend std::istream &operator>>(std::istream &stream, DTNTPWorker::TimeSyncMessage &obj);
			};

		private:
			static const unsigned int PROTO_VERSION;
			static const std::string TAG;

			/**
			 * check if we should sync with another node
			 * @param node
			 */
			bool shouldSyncWith(const dtn::core::Node &node) const;

			/**
			 * Start sync'ing with another node
			 * @param node
			 */
			void syncWith(const dtn::core::Node &node);

			/**
			 * Determine if this node is configured as reference or not
			 * @return
			 */
			bool hasReference() const;

			/**
			 * this method decode neighbor data
			 * @param attr
			 * @param version
			 * @param timestamp
			 * @param quality
			 */
			void decode(const dtn::core::Node::Attribute &attr, unsigned int &version, size_t &timestamp, float &quality) const;

			/**
			 * Synchronize this clock with another one
			 * @param msg
			 * @param tv
			 */
			void sync(const TimeSyncMessage &msg, const struct timeval &tv, const struct timeval &local, const struct timeval &remote);

			// sync threshold
			float _sync_threshold;

			// send discovery announcements with the local clock rating
			bool _announce_rating;

			// the base rating used to determine the current clock rating
			double _base_rating;

			// the local rating is at least decremented by this value between each synchronization
			double _psi;

			// current value for sigma
			double _sigma;

			// synchronize with other nodes
			bool _sync;

			// timestamp of the last synchronization with another (better) clock
			timeval _last_sync_time;

			// Mutex to lock the synchronization process
			ibrcommon::Mutex _sync_lock;

			// manage a list of recently sync'd nodes
			ibrcommon::Mutex _blacklist_lock;
			typedef std::map<EID, uint64_t> blacklist_map;
			blacklist_map _sync_blacklist;
		};
	}
}

#endif /* DTNTPWORKER_H_ */
