/*
 * ProphetRoutingExtension.cpp
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

#include "routing/prophet/ProphetRoutingExtension.h"
#include "routing/prophet/DeliveryPredictabilityMap.h"

#include "core/BundleCore.h"

#include <algorithm>
#include <memory>

#include "routing/QueueBundleEvent.h"
#include "routing/NodeHandshakeEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "net/ConnectionEvent.h"
#include "core/TimeEvent.h"
#include "core/NodeEvent.h"
#include "core/BundlePurgeEvent.h"
#include "core/BundleEvent.h"

#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/ThreadsafeReference.h>

#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/Exceptions.h>
#include <ibrdtn/utils/Clock.h>

namespace dtn
{
	namespace routing
	{
		const std::string ProphetRoutingExtension::TAG = "ProphetRoutingExtension";

		ProphetRoutingExtension::ProphetRoutingExtension(ForwardingStrategy *strategy, float p_encounter_max, float p_encounter_first, float p_first_threshold,
								 float beta, float gamma, float delta, ibrcommon::Timer::time_t time_unit, ibrcommon::Timer::time_t i_typ,
								 dtn::data::Timestamp next_exchange_timeout)
			: _deliveryPredictabilityMap(time_unit, beta, gamma),
			  _forwardingStrategy(strategy), _next_exchange_timeout(next_exchange_timeout), _next_exchange_timestamp(0),
			  _p_encounter_max(p_encounter_max), _p_encounter_first(p_encounter_first),
			  _p_first_threshold(p_first_threshold), _delta(delta), _i_typ(i_typ)
		{
			// assign myself to the forwarding strategy
			strategy->setProphetRouter(this);

			// set value for local EID to 1.0
			_deliveryPredictabilityMap.set(core::BundleCore::local, 1.0);

			// define the first exchange timestamp
			_next_exchange_timestamp = dtn::utils::Clock::getUnixTimestamp() + _next_exchange_timeout;

			// write something to the syslog
			IBRCOMMON_LOGGER_TAG(ProphetRoutingExtension::TAG, info) << "Initializing PRoPHET routing module" << IBRCOMMON_LOGGER_ENDL;
		}

		ProphetRoutingExtension::~ProphetRoutingExtension()
		{
			stop();
			join();
			delete _forwardingStrategy;
		}

		void ProphetRoutingExtension::requestHandshake(const dtn::data::EID&, NodeHandshake& handshake) const
		{
			handshake.addRequest(DeliveryPredictabilityMap::identifier);
			handshake.addRequest(AcknowledgementSet::identifier);

			// request summary vector to exclude bundles known by the peer
			handshake.addRequest(BloomFilterSummaryVector::identifier);
		}

		void ProphetRoutingExtension::responseHandshake(const dtn::data::EID& neighbor, const NodeHandshake& request, NodeHandshake& response)
		{
			const dtn::data::EID neighbor_node = neighbor.getNode();
			if (request.hasRequest(DeliveryPredictabilityMap::identifier))
			{
				ibrcommon::MutexLock l(_deliveryPredictabilityMap);
				age();
				response.addItem(new DeliveryPredictabilityMap(_deliveryPredictabilityMap));
			}
			if (request.hasRequest(AcknowledgementSet::identifier))
			{
				ibrcommon::MutexLock l(_acknowledgementSet);
				response.addItem(new AcknowledgementSet(_acknowledgementSet));
			}
		}

		void ProphetRoutingExtension::processHandshake(const dtn::data::EID& neighbor, NodeHandshake& response)
		{
			const dtn::data::EID neighbor_node = neighbor.getNode();

			/* ignore neighbors, that have our EID */
			if (neighbor_node == dtn::core::BundleCore::local) return;

			try {
				const DeliveryPredictabilityMap& neighbor_dp_map = response.get<DeliveryPredictabilityMap>();

				IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 10) << "delivery predictability map received from " << neighbor.getString() << IBRCOMMON_LOGGER_ENDL;

				// update the encounter on every routing handshake
				{
					ibrcommon::MutexLock l(_deliveryPredictabilityMap);

					age();

					/* update predictability for this neighbor */
					updateNeighbor(neighbor_node);
				}

				// store a copy of the map in the neighbor database
				{
					NeighborDatabase &db = (**this).getNeighborDB();
					NeighborDataset ds(new DeliveryPredictabilityMap(neighbor_dp_map));

					ibrcommon::MutexLock l(db);
					db.create(neighbor_node).putDataset(ds);
				}

				ibrcommon::MutexLock l(_deliveryPredictabilityMap);

				/* update the dp_map */
				_deliveryPredictabilityMap.update(neighbor_node, neighbor_dp_map, _p_encounter_first);

			} catch (std::exception&) { }

			try {
				const AcknowledgementSet& neighbor_ack_set = response.get<AcknowledgementSet>();

				IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 10) << "ack'set received from " << neighbor.getString() << IBRCOMMON_LOGGER_ENDL;

				// merge ack'set into the known bundles
				for (AcknowledgementSet::const_iterator it = _acknowledgementSet.begin(); it != _acknowledgementSet.end(); ++it) {
					(**this).setKnown(*it);
				}

				// merge the received ack set with the local one
				{
					ibrcommon::MutexLock l(_acknowledgementSet);
					_acknowledgementSet.merge(neighbor_ack_set);
				}

				/* remove acknowledged bundles from bundle store if we do not have custody */
				dtn::storage::BundleStorage &storage = (**this).getStorage();

				class BundleFilter : public dtn::storage::BundleSelector
				{
				public:
					BundleFilter(const AcknowledgementSet& neighbor_ack_set)
					 : _ackset(neighbor_ack_set)
					{}

					virtual ~BundleFilter() {}

					virtual dtn::data::Size limit() const throw () { return 0; }

					virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const throw (dtn::storage::BundleSelectorException)
					{
						// do not delete any bundles with
						if (meta.destination.getNode() == dtn::core::BundleCore::local)
							return false;

						if(!_ackset.has(meta))
							return false;

						return true;
					}

				private:
					const AcknowledgementSet& _ackset;
				} filter(neighbor_ack_set);

				dtn::storage::BundleResultList removeList;
				storage.get(filter, removeList);

				for (std::list<dtn::data::MetaBundle>::const_iterator it = removeList.begin(); it != removeList.end(); ++it)
				{
					const dtn::data::MetaBundle &meta = (*it);

					if (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
					{
						dtn::core::BundlePurgeEvent::raise(meta);
						IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 10) << "Bundle removed due to prophet ack: " << meta.toString() << IBRCOMMON_LOGGER_ENDL;
					}
					else
					{
						IBRCOMMON_LOGGER_TAG(ProphetRoutingExtension::TAG, warning) << neighbor.getString() << " requested to purge a bundle with a non-singleton destination: " << meta.toString() << IBRCOMMON_LOGGER_ENDL;
					}

					/* generate a report */
					dtn::core::BundleEvent::raise(meta, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::NO_ADDITIONAL_INFORMATION);
				}
			} catch (const dtn::storage::NoBundleFoundException&) {
			} catch (std::exception&) { }
		}

		void ProphetRoutingExtension::notify(const dtn::core::Event *evt) throw ()
		{
			try {
				const dtn::core::TimeEvent &time = dynamic_cast<const dtn::core::TimeEvent&>(*evt);

				// expire bundles in the acknowledgement set
				{
					ibrcommon::MutexLock l(_acknowledgementSet);
					_acknowledgementSet.expire(time.getUnixTimestamp());
				}

				ibrcommon::MutexLock l(_next_exchange_mutex);
				if ((_next_exchange_timestamp > 0) && (_next_exchange_timestamp < time.getUnixTimestamp()))
				{
					_taskqueue.push( new NextExchangeTask() );

					// define the next exchange timestamp
					_next_exchange_timestamp = time.getUnixTimestamp() + _next_exchange_timeout;
				}
				return;
			} catch (const std::bad_cast&) { };

			// If an incoming bundle is received, forward it to all connected neighbors
			try {
				const QueueBundleEvent &queued = dynamic_cast<const QueueBundleEvent&>(*evt);

				// new bundles trigger a recheck for all neighbors
				const std::set<dtn::core::Node> nl = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();

				for (std::set<dtn::core::Node>::const_iterator iter = nl.begin(); iter != nl.end(); ++iter)
				{
					const dtn::core::Node &n = (*iter);

					if (n.getEID() != queued.origin) {
						// transfer the next bundle to this destination
						_taskqueue.push( new SearchNextBundleTask( n.getEID() ) );
					}
				}
				return;
			} catch (const std::bad_cast&) { };

			// If a new neighbor comes available search for bundles
			try {
				const dtn::core::NodeEvent &nodeevent = dynamic_cast<const dtn::core::NodeEvent&>(*evt);
				const dtn::core::Node &n = nodeevent.getNode();

				if (nodeevent.getAction() == NODE_AVAILABLE)
				{
					_taskqueue.push( new SearchNextBundleTask( n.getEID() ) );
				}
				else if (nodeevent.getAction() == NODE_DATA_ADDED)
				{
					_taskqueue.push( new SearchNextBundleTask( n.getEID() ) );
				}

				return;
			} catch (const std::bad_cast&) { };

			try {
				const dtn::net::ConnectionEvent &ce = dynamic_cast<const dtn::net::ConnectionEvent&>(*evt);

				if (ce.state == dtn::net::ConnectionEvent::CONNECTION_UP)
				{
					// send all (multi-hop) bundles in the storage to the neighbor
					_taskqueue.push( new SearchNextBundleTask(ce.peer) );
				}
				return;
			} catch (const std::bad_cast&) { };

			try {
				const NodeHandshakeEvent &handshake = dynamic_cast<const NodeHandshakeEvent&>(*evt);

				if (handshake.state == NodeHandshakeEvent::HANDSHAKE_UPDATED)
				{
					// transfer the next bundle to this destination
					_taskqueue.push( new SearchNextBundleTask( handshake.peer ) );
				}
				else if (handshake.state == NodeHandshakeEvent::HANDSHAKE_COMPLETED)
				{
					// transfer the next bundle to this destination
					_taskqueue.push( new SearchNextBundleTask( handshake.peer ) );
				}
				return;
			} catch (const std::bad_cast&) { };

			// The bundle transfer has been aborted
			try {
				const dtn::net::TransferAbortedEvent &aborted = dynamic_cast<const dtn::net::TransferAbortedEvent&>(*evt);

				// transfer the next bundle to this destination
				_taskqueue.push( new SearchNextBundleTask( aborted.getPeer() ) );

				return;
			} catch (const std::bad_cast&) { };

			// A bundle transfer was successful
			try {
				const dtn::net::TransferCompletedEvent &completed = dynamic_cast<const dtn::net::TransferCompletedEvent&>(*evt);
				const dtn::data::MetaBundle &meta = completed.getBundle();
				const dtn::data::EID &peer = completed.getPeer();

				if ((meta.destination.getNode() == peer.getNode())
						/* send prophet ack only for singleton */
						&& (meta.procflags & dtn::data::Bundle::DESTINATION_IS_SINGLETON))
				{
					/* the bundle was transferred, mark it as acknowledged */
					ibrcommon::MutexLock l(_acknowledgementSet);
					_acknowledgementSet.add(meta);
				}

				// add forwarded entry to GTMX strategy
				try {
					GTMX_Strategy &gtmx = dynamic_cast<GTMX_Strategy&>(*_forwardingStrategy);
					gtmx.addForward(meta);
				} catch (const std::bad_cast &ex) { };

				// search for the next bundle
				_taskqueue.push( new SearchNextBundleTask( completed.getPeer() ) );
				return;
			} catch (const std::bad_cast&) { };
		}

		void ProphetRoutingExtension::componentUp() throw ()
		{
			// reset task queue
			_taskqueue.reset();

			// routine checked for throw() on 15.02.2013
			try {
				// run the thread
				start();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_TAG(ProphetRoutingExtension::TAG, error) << "componentUp failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void ProphetRoutingExtension::componentDown() throw ()
		{
			try {
				// stop the thread
				stop();
				join();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_TAG(ProphetRoutingExtension::TAG, error) << "componentDown failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		ibrcommon::ThreadsafeReference<DeliveryPredictabilityMap> ProphetRoutingExtension::getDeliveryPredictabilityMap()
		{
			{
				ibrcommon::MutexLock l(_deliveryPredictabilityMap);
				age();
			}
			return ibrcommon::ThreadsafeReference<DeliveryPredictabilityMap>(_deliveryPredictabilityMap, _deliveryPredictabilityMap);
		}

		ibrcommon::ThreadsafeReference<const DeliveryPredictabilityMap> ProphetRoutingExtension::getDeliveryPredictabilityMap() const
		{
			return ibrcommon::ThreadsafeReference<const DeliveryPredictabilityMap>(_deliveryPredictabilityMap, const_cast<DeliveryPredictabilityMap&>(_deliveryPredictabilityMap));
		}

		ibrcommon::ThreadsafeReference<const AcknowledgementSet> ProphetRoutingExtension::getAcknowledgementSet() const
		{
			return ibrcommon::ThreadsafeReference<const AcknowledgementSet>(_acknowledgementSet, const_cast<AcknowledgementSet&>(_acknowledgementSet));
		}

		void ProphetRoutingExtension::ProphetRoutingExtension::run() throw ()
		{
			class BundleFilter : public dtn::storage::BundleSelector
			{
			public:
				BundleFilter(const NeighborDatabase::NeighborEntry &entry, ForwardingStrategy &strategy, const DeliveryPredictabilityMap &dpm)
				 : _entry(entry), _strategy(strategy), _dpm(dpm)
				{ };

				virtual ~BundleFilter() {};

				virtual dtn::data::Size limit() const throw () { return _entry.getFreeTransferSlots(); };

				virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const throw (dtn::storage::BundleSelectorException)
				{
					// check Scope Control Block - do not forward bundles with hop limit == 0
					if (meta.hopcount == 0)
					{
						return false;
					}

					// do not forward any routing control message
					// this is done by the neighbor routing module
					if (isRouting(meta.source))
					{
						return false;
					}

					// do not forward local bundles
					if ((meta.destination.getNode() == dtn::core::BundleCore::local)
							&& meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON)
						)
					{
						return false;
					}

					// check Scope Control Block - do not forward non-group bundles with hop limit <= 1
					if ((meta.hopcount <= 1) && (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON)))
					{
						return false;
					}

					// do not forward bundles addressed to this neighbor,
					// because this is handled by neighbor routing extension
					if (_entry.eid == meta.destination.getNode())
					{
						return false;
					}

					// do not forward bundles already known by the destination
					// throws BloomfilterNotAvailableException if no filter is available or it is expired
					try {
						if (_entry.has(meta, true))
						{
							return false;
						}
					} catch (const dtn::routing::NeighborDatabase::BloomfilterNotAvailableException&) {
						throw dtn::storage::BundleSelectorException();
					}

					// ask the routing strategy if this bundle should be selected
					if (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
					{
						return _strategy.shallForward(_dpm, meta);
					}

					return true;
				}

			private:
				const NeighborDatabase::NeighborEntry &_entry;
				const ForwardingStrategy &_strategy;
				const DeliveryPredictabilityMap &_dpm;
			};

			dtn::storage::BundleResultList list;

			while (true)
			{
				try {
					Task *t = _taskqueue.getnpop(true);
					std::auto_ptr<Task> killer(t);

					IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 50) << "processing task " << t->toString() << IBRCOMMON_LOGGER_ENDL;

					try {
						/**
						 * SearchNextBundleTask triggers a search for a bundle to transfer
						 * to another host. This Task is generated by TransferCompleted, TransferAborted
						 * and node events.
						 */
						try {
							SearchNextBundleTask &task = dynamic_cast<SearchNextBundleTask&>(*t);

							// lock the neighbor database while searching for bundles
							try {
								NeighborDatabase &db = (**this).getNeighborDB();

								ibrcommon::MutexLock l(db);
								NeighborDatabase::NeighborEntry &entry = db.get(task.eid);

								// check if enough transfer slots available (threshold reached)
								if (!entry.isTransferThresholdReached())
									throw NeighborDatabase::NoMoreTransfersAvailable();

								// get the DeliveryPredictabilityMap of the potentially next hop
								const DeliveryPredictabilityMap &dpm = entry.getDataset<DeliveryPredictabilityMap>();

								// get the bundle filter of the neighbor
								BundleFilter filter(entry, *_forwardingStrategy, dpm);

								// some debug output
								IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 40) << "search some bundles not known by " << task.eid.getString() << IBRCOMMON_LOGGER_ENDL;

								// query some unknown bundle from the storage, the list contains max. 10 items.
								list.clear();
								(**this).getSeeker().get(filter, list);
							} catch (const NeighborDatabase::DatasetNotAvailableException&) {
								// if there is no DeliveryPredictabilityMap for the next hop
								// perform a routing handshake with the peer
								(**this).doHandshake(task.eid);
							} catch (const dtn::storage::BundleSelectorException&) {
								// query a new summary vector from this neighbor
								(**this).doHandshake(task.eid);
							}

							// send the bundles as long as we have resources
							for (std::list<dtn::data::MetaBundle>::const_iterator iter = list.begin(); iter != list.end(); ++iter)
							{
								const dtn::data::MetaBundle &meta = (*iter);

								try {
									transferTo(task.eid, meta);
								} catch (const NeighborDatabase::AlreadyInTransitException&) { };
							}
						} catch (const NeighborDatabase::NoMoreTransfersAvailable &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const NeighborDatabase::NeighborNotAvailableException &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const dtn::storage::NoBundleFoundException &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const std::bad_cast&) { }

						/**
						 * NextExchangeTask is a timer based event, that triggers
						 * a new dp_map exchange for every connected node
						 */
						try {
							dynamic_cast<NextExchangeTask&>(*t);

							std::set<dtn::core::Node> neighbors = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();
							std::set<dtn::core::Node>::const_iterator it;
							for(it = neighbors.begin(); it != neighbors.end(); ++it)
							{
								try{
									(**this).doHandshake(it->getEID());
								} catch (const ibrcommon::Exception &ex) { }
							}
						} catch (const std::bad_cast&) { }

					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 20) << "task failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const std::exception &ex) {
					IBRCOMMON_LOGGER_DEBUG_TAG(ProphetRoutingExtension::TAG, 15) << "terminated due to " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					return;
				}

				yield();
			}
		}

		void ProphetRoutingExtension::__cancellation() throw ()
		{
			_taskqueue.abort();
		}

		float ProphetRoutingExtension::p_encounter(const dtn::data::EID &neighbor) const
		{
			age_map::const_iterator it = _ageMap.find(neighbor);
			if(it == _ageMap.end())
			{
				/* In this case, we got a transitive update for the node earlier but havent encountered it ourselves */
				return _p_encounter_max;
			}

			const dtn::data::Timestamp currentTime = dtn::utils::Clock::getUnixTimestamp();
			const dtn::data::Timestamp time_diff = currentTime - it->second;
#ifdef __DEVELOPMENT_ASSERTIONS__
			assert(currentTime >= it->second && "the ageMap timestamp should be smaller than the current timestamp");
#endif
			if(time_diff > _i_typ)
			{
				return _p_encounter_max;
			}
			else
			{
				return _p_encounter_max * static_cast<float>(time_diff.get<size_t>() / _i_typ);
			}
		}

		void ProphetRoutingExtension::updateNeighbor(const dtn::data::EID &neighbor)
		{
			/**
			 * Calculate new value for this encounter
			 */
			try {
				float neighbor_dp = _deliveryPredictabilityMap.get(neighbor);

				if (neighbor_dp < _p_first_threshold)
				{
					neighbor_dp = _p_encounter_first;
				}
				else
				{
					neighbor_dp += (1 - _delta - neighbor_dp) * p_encounter(neighbor);
				}

				_deliveryPredictabilityMap.set(neighbor, neighbor_dp);
			} catch (const DeliveryPredictabilityMap::ValueNotFoundException&) {
				_deliveryPredictabilityMap.set(neighbor, _p_encounter_first);
			}

			_ageMap[neighbor] = dtn::utils::Clock::getUnixTimestamp();
		}

		void ProphetRoutingExtension::age()
		{
			_deliveryPredictabilityMap.age(_p_first_threshold);
		}

		ProphetRoutingExtension::SearchNextBundleTask::SearchNextBundleTask(const dtn::data::EID &eid)
			: eid(eid)
		{
		}

		ProphetRoutingExtension::SearchNextBundleTask::~SearchNextBundleTask()
		{
		}

		std::string ProphetRoutingExtension::SearchNextBundleTask::toString() const
		{
			return "SearchNextBundleTask: " + eid.getString();
		}

		ProphetRoutingExtension::NextExchangeTask::NextExchangeTask()
		{
		}

		ProphetRoutingExtension::NextExchangeTask::~NextExchangeTask()
		{
		}

		std::string ProphetRoutingExtension::NextExchangeTask::toString() const
		{
			return "NextExchangeTask";
		}

		ProphetRoutingExtension::GRTR_Strategy::GRTR_Strategy()
		{
		}

		ProphetRoutingExtension::GRTR_Strategy::~GRTR_Strategy()
		{
		}

		bool ProphetRoutingExtension::GRTR_Strategy::shallForward(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::MetaBundle& bundle) const
		{
			return neighborDPIsGreater(neighbor_dpm, bundle.destination);
		}

		ProphetRoutingExtension::GTMX_Strategy::GTMX_Strategy(unsigned int NF_max)
		 : _NF_max(NF_max)
		{
		}

		ProphetRoutingExtension::GTMX_Strategy::~GTMX_Strategy()
		{
		}

		void ProphetRoutingExtension::GTMX_Strategy::addForward(const dtn::data::BundleID &id)
		{
			nf_map::iterator nf_it = _NF_map.find(id);

			if (nf_it == _NF_map.end()) {
				nf_it = _NF_map.insert(std::make_pair(id, 0)).first;
			}

			++nf_it->second;
		}

		bool ProphetRoutingExtension::GTMX_Strategy::shallForward(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::MetaBundle& bundle) const
		{
			unsigned int NF = 0;

			nf_map::const_iterator nf_it = _NF_map.find(bundle);
			if(nf_it != _NF_map.end()) {
				NF = nf_it->second;
			}

			if (NF > _NF_max) return false;

			return neighborDPIsGreater(neighbor_dpm, bundle.destination);
		}

	} // namespace routing
} // namespace dtn
