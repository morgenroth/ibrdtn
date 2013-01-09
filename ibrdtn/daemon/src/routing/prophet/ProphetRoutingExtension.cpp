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
#include "core/TimeEvent.h"
#include "core/NodeEvent.h"
#include "core/BundlePurgeEvent.h"
#include "core/BundleEvent.h"

#include <ibrcommon/Logger.h>

#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/Exceptions.h>
#include <ibrdtn/utils/Clock.h>

namespace dtn
{
	namespace routing
	{
		ProphetRoutingExtension::ProphetRoutingExtension(ForwardingStrategy *strategy, float p_encounter_max, float p_encounter_first, float p_first_threshold,
								 float beta, float gamma, float delta, ibrcommon::Timer::time_t time_unit, ibrcommon::Timer::time_t i_typ,
								 ibrcommon::Timer::time_t next_exchange_timeout)
			: _deliveryPredictabilityMap(time_unit, beta, gamma),
			  _forwardingStrategy(strategy), _next_exchange_timeout(next_exchange_timeout), _next_exchange_timestamp(0),
			  _p_encounter_max(p_encounter_max), _p_encounter_first(p_encounter_first),
			  _p_first_threshold(p_first_threshold), _delta(delta), _i_typ(i_typ)
		{
			// assign myself to the forwarding strategy
			strategy->setProphetRouter(this);

			// set value for local EID to 1.0
			_deliveryPredictabilityMap.set(core::BundleCore::local, 1.0);

			// write something to the syslog
			IBRCOMMON_LOGGER(info) << "Initializing PRoPHET routing module" << IBRCOMMON_LOGGER_ENDL;
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

#ifndef DISABLE_MAP_STORE
				/* check if a saved predictablity map exists */
				map_store::iterator it;
				if((it = _mapStore.find(neighbor_node)) != _mapStore.end())
				{
					/* the map has already been aged by processHandshake */
					response.addItem(new DeliveryPredictabilityMap(it->second));
				}
				else
				{
					age();

					/* copy the map to the map_store so that processHandshakes knows that it was already aged */
					_mapStore[neighbor_node] = _deliveryPredictabilityMap;

					response.addItem(new DeliveryPredictabilityMap(_deliveryPredictabilityMap));
				}
#else
				age();
				response.addItem(new DeliveryPredictabilityMap(_deliveryPredictabilityMap));
#endif
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
			if(neighbor_node == dtn::core::BundleCore::local)
				return;

			// update the encounter on every routing handshake
			{
				ibrcommon::MutexLock l(_deliveryPredictabilityMap);

				age();

				/* update predictability for this neighbor */
				updateNeighbor(neighbor_node);
			}

			try {
				const DeliveryPredictabilityMap& neighbor_dp_map = response.get<DeliveryPredictabilityMap>();

				// store a copy of the map in the neighbor database
				{
					NeighborDatabase &db = (**this).getNeighborDB();
					DeliveryPredictabilityMap *dpm = new DeliveryPredictabilityMap(neighbor_dp_map);

					ibrcommon::MutexLock l(db);
					db.create(neighbor_node).putDataset(dpm);
				}

				ibrcommon::MutexLock l(_deliveryPredictabilityMap);

#ifndef DISABLE_MAP_STORE
				/* save the current map for this neighbor */
				map_store::iterator it;
				if((it = _mapStore.find(neighbor_node)) == _mapStore.end())
				{
					/* the map has not been aged yet */
					age();
				}
				_mapStore[neighbor_node] = _deliveryPredictabilityMap;
#endif

				/* update the dp_map */
				_deliveryPredictabilityMap.update(neighbor_node, neighbor_dp_map, _p_encounter_first);

			} catch (std::exception&) { }

			try {
				const AcknowledgementSet& neighbor_ack_set = response.get<AcknowledgementSet>();

				ibrcommon::MutexLock l(_acknowledgementSet);

				_acknowledgementSet.merge(neighbor_ack_set);

				/* remove acknowledged bundles from bundle store if we do not have custody */
				dtn::storage::BundleStorage &storage = (**this).getStorage();

				class BundleFilter : public dtn::storage::BundleStorage::BundleFilterCallback
				{
				public:
					BundleFilter(const AcknowledgementSet& entry)
					 : _entry(entry)
					{}

					virtual ~BundleFilter() {}

					virtual size_t limit() const { return 0; }

					virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const throw (dtn::storage::BundleStorage::BundleFilterException)
					{
						// do not delete any bundles with
						if ((meta.destination.getNode() == dtn::core::BundleCore::local) || (meta.custodian.getNode() == dtn::core::BundleCore::local))
							return false;

						if(!_entry.has(meta))
							return false;

						return true;
					}

				private:
					const AcknowledgementSet& _entry;
				} filter(_acknowledgementSet);

				dtn::storage::BundleResultList removeList;
				storage.get(filter, removeList);

				for (std::list<dtn::data::MetaBundle>::const_iterator it = removeList.begin(); it != removeList.end(); ++it)
				{
					const dtn::data::MetaBundle &meta = (*it);

					dtn::core::BundlePurgeEvent::raise(meta);

					IBRCOMMON_LOGGER(notice) << "Bundle removed due to prophet ack: " << meta.toString() << IBRCOMMON_LOGGER_ENDL;

					/* generate a report */
					dtn::core::BundleEvent::raise(meta, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::DEPLETED_STORAGE);
				}
			} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
			} catch (std::exception&) { }
		}

		void ProphetRoutingExtension::notify(const dtn::core::Event *evt)
		{
			try {
				const dtn::core::TimeEvent &time = dynamic_cast<const dtn::core::TimeEvent&>(*evt);

				ibrcommon::MutexLock l(_next_exchange_mutex);
				if ((_next_exchange_timestamp > 0) && (_next_exchange_timestamp < time.getUnixTimestamp()))
				{
					_taskqueue.push( new NextExchangeTask() );

					// define the next exchange timestamp
					_next_exchange_timestamp = time.getUnixTimestamp() + _next_exchange_timeout;

					ibrcommon::MutexLock l(_acknowledgementSet);
					_acknowledgementSet.purge();
				}
				return;
			} catch (const std::bad_cast&) { };

#ifndef DISABLE_MAP_STORE
			/* in case of timer or node disconnect event, remove corresponding _mapStore items */
			try {
				const dtn::core::NodeEvent &nodeevent = dynamic_cast<const dtn::core::NodeEvent&>(*evt);
				const dtn::data::EID &neighbor = nodeevent.getNode().getEID().getNode();

				if (nodeevent.getAction() == NODE_UNAVAILABLE)
				{
					ibrcommon::MutexLock l(_deliveryPredictabilityMap);
					_mapStore.erase(neighbor);
				}
				return;
			} catch (const std::bad_cast&) { };
#endif

			// If an incoming bundle is received, forward it to all connected neighbors
			try {
				dynamic_cast<const QueueBundleEvent&>(*evt);

				// new bundles trigger a recheck for all neighbors
				const std::set<dtn::core::Node> nl = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();

				for (std::set<dtn::core::Node>::const_iterator iter = nl.begin(); iter != nl.end(); iter++)
				{
					const dtn::core::Node &n = (*iter);

					// transfer the next bundle to this destination
					_taskqueue.push( new SearchNextBundleTask( n.getEID() ) );
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
					_acknowledgementSet.insert(Acknowledgement(meta, meta.expiretime));
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

		ibrcommon::ThreadsafeReference<const ProphetRoutingExtension::AcknowledgementSet> ProphetRoutingExtension::getAcknowledgementSet() const
		{
			return ibrcommon::ThreadsafeReference<const AcknowledgementSet>(_acknowledgementSet, const_cast<AcknowledgementSet&>(_acknowledgementSet));
		}

		void ProphetRoutingExtension::ProphetRoutingExtension::run() throw ()
		{
			class BundleFilter : public dtn::storage::BundleStorage::BundleFilterCallback
			{
			public:
				BundleFilter(const NeighborDatabase::NeighborEntry &entry, ForwardingStrategy &strategy, const DeliveryPredictabilityMap &dpm)
				 : _entry(entry), _strategy(strategy), _dpm(dpm)
				{ };

				virtual ~BundleFilter() {};

				virtual size_t limit() const { return _entry.getFreeTransferSlots(); };

				virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const throw (dtn::storage::BundleStorage::BundleFilterException)
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
						throw dtn::storage::BundleStorage::BundleFilterException();
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

			dtn::storage::BundleStorage &storage = (**this).getStorage();
			dtn::storage::BundleResultList list;

			{
				ibrcommon::MutexLock l(_next_exchange_mutex);
				// define the next exchange timestamp
				_next_exchange_timestamp = dtn::utils::Clock::getUnixTimestamp() + _next_exchange_timeout;
			}

			while (true)
			{
				try {
					Task *t = _taskqueue.getnpop(true);
					std::auto_ptr<Task> killer(t);

					IBRCOMMON_LOGGER_DEBUG(50) << "processing prophet task " << t->toString() << IBRCOMMON_LOGGER_ENDL;

					try {
						/**
						 * SearchNextBundleTask triggers a search for a bundle to transfer
						 * to another host. This Task is generated by TransferCompleted, TransferAborted
						 * and node events.
						 */
						try {
							SearchNextBundleTask &task = dynamic_cast<SearchNextBundleTask&>(*t);
							NeighborDatabase &db = (**this).getNeighborDB();

							ibrcommon::MutexLock l(db);
							NeighborDatabase::NeighborEntry &entry = db.get(task.eid);

							// check if enough transfer slots available (threshold reached)
							if (!entry.isTransferThresholdReached())
								throw NeighborDatabase::NoMoreTransfersAvailable();

							try {
								// get the DeliveryPredictabilityMap of the potentially next hop
								DeliveryPredictabilityMap &dpm = entry.getDataset<DeliveryPredictabilityMap>();

								// get the bundle filter of the neighbor
								BundleFilter filter(entry, *_forwardingStrategy, dpm);

								// some debug output
								IBRCOMMON_LOGGER_DEBUG(40) << "search some bundles not known by " << task.eid.getString() << IBRCOMMON_LOGGER_ENDL;

								// query some unknown bundle from the storage, the list contains max. 10 items.
								list.clear();
								storage.get(filter, list);

								// send the bundles as long as we have resources
								for (std::list<dtn::data::MetaBundle>::const_iterator iter = list.begin(); iter != list.end(); iter++)
								{
									const dtn::data::MetaBundle &meta = (*iter);

									try {
										transferTo(entry, meta);
									} catch (const NeighborDatabase::AlreadyInTransitException&) { };
								}
							} catch (const NeighborDatabase::DatasetNotAvailableException&) {
								// if there is no DeliveryPredictabilityMap for the next hop
								// perform a routing handshake with the peer
								(**this).doHandshake(task.eid);
							} catch (const dtn::storage::BundleStorage::BundleFilterException&) {
								// query a new summary vector from this neighbor
								(**this).doHandshake(task.eid);
							}
						} catch (const NeighborDatabase::NoMoreTransfersAvailable&) {
						} catch (const NeighborDatabase::NeighborNotAvailableException&) {
						} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
						} catch (const std::bad_cast&) { }

						/**
						 * NextExchangeTask is a timer based event, that triggers
						 * a new dp_map exchange for every connected node
						 */
						try {
							dynamic_cast<NextExchangeTask&>(*t);

#ifndef DISABLE_MAP_STORE
							{
								/* lock the dp map when accessing the mapStore */
								ibrcommon::MutexLock l(_deliveryPredictabilityMap);
								_mapStore.clear();
							}
#endif

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
						IBRCOMMON_LOGGER_DEBUG(20) << "Exception occurred in ProphetRoutingExtension: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const std::exception&) {
					return;
				}

				yield();
			}
		}

		void ProphetRoutingExtension::__cancellation() throw ()
		{
			{
				ibrcommon::MutexLock l(_next_exchange_mutex);
				_next_exchange_timestamp = 0;
			}
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

			size_t currentTime = dtn::utils::Clock::getUnixTimestamp();
			size_t time_diff = currentTime - it->second;
#ifdef __DEVELOPMENT_ASSERTIONS__
			assert(currentTime >= it->second && "the ageMap timestamp should be smaller than the current timestamp");
#endif
			if(time_diff > _i_typ)
			{
				return _p_encounter_max;
			}
			else
			{
				return _p_encounter_max * ((float) time_diff / _i_typ);
			}
		}

		const size_t ProphetRoutingExtension::AcknowledgementSet::identifier = NodeHandshakeItem::PROPHET_ACKNOWLEDGEMENT_SET;

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

		ProphetRoutingExtension::Acknowledgement::Acknowledgement()
		 : expire_time(0)
		{
		}

		ProphetRoutingExtension::Acknowledgement::Acknowledgement(const dtn::data::BundleID &bundleID, size_t expire_time)
			: bundleID(bundleID), expire_time(expire_time)
		{
		}

		ProphetRoutingExtension::Acknowledgement::~Acknowledgement()
		{
		}

		bool ProphetRoutingExtension::Acknowledgement::operator<(const Acknowledgement &other) const
		{
			return (bundleID < other.bundleID);
		}

		ProphetRoutingExtension::AcknowledgementSet::AcknowledgementSet()
		{
		}

		ProphetRoutingExtension::AcknowledgementSet::AcknowledgementSet(const AcknowledgementSet &other)
			: ibrcommon::Mutex(), _ackSet(other._ackSet)
		{
		}

		void ProphetRoutingExtension::AcknowledgementSet::insert(const Acknowledgement& ack)
		{
			_ackSet.insert(ack);
		}

		void ProphetRoutingExtension::AcknowledgementSet::clear()
		{
			_ackSet.clear();
		}

		void ProphetRoutingExtension::AcknowledgementSet::purge()
		{
			std::set<Acknowledgement>::iterator it;
			for(it = _ackSet.begin(); it != _ackSet.end();)
			{
				if(dtn::utils::Clock::isExpired(it->expire_time))
					_ackSet.erase(it++);
				else
					++it;
			}
		}

		void ProphetRoutingExtension::AcknowledgementSet::merge(const AcknowledgementSet &other)
		{
			std::set<Acknowledgement>::iterator it;
			for(it = other._ackSet.begin(); it != other._ackSet.end(); ++it)
			{
				const Acknowledgement &ack = (*it);
				if (!dtn::utils::Clock::isExpired(ack.expire_time)) {
					_ackSet.insert(ack);
				}
			}
		}

		bool ProphetRoutingExtension::AcknowledgementSet::has(const dtn::data::BundleID &bundle) const
		{
			return _ackSet.find(Acknowledgement(bundle, 0)) != _ackSet.end();
		}

		size_t ProphetRoutingExtension::AcknowledgementSet::getIdentifier() const
		{
			return identifier;
		}

		size_t ProphetRoutingExtension::AcknowledgementSet::size() const
		{
			return _ackSet.size();
		}

		size_t ProphetRoutingExtension::AcknowledgementSet::getLength() const
		{
			std::stringstream ss;
			serialize(ss);
			return ss.str().length();
		}

		std::ostream& ProphetRoutingExtension::AcknowledgementSet::serialize(std::ostream& stream) const
		{
			stream << (*this);
			return stream;
		}

		std::istream& ProphetRoutingExtension::AcknowledgementSet::deserialize(std::istream& stream)
		{
			stream >> (*this);
			return stream;
		}

		const std::set<ProphetRoutingExtension::Acknowledgement>& ProphetRoutingExtension::AcknowledgementSet::operator*() const
		{
			return _ackSet;
		}

		std::ostream& operator<<(std::ostream& stream, const ProphetRoutingExtension::AcknowledgementSet& ack_set)
		{
			stream << dtn::data::SDNV(ack_set.size());
			for (std::set<ProphetRoutingExtension::Acknowledgement>::const_iterator it = ack_set._ackSet.begin(); it != ack_set._ackSet.end(); ++it)
			{
				const ProphetRoutingExtension::Acknowledgement &ack = (*it);
				stream << ack;
			}

			return stream;
		}

		std::istream& operator>>(std::istream &stream, ProphetRoutingExtension::AcknowledgementSet &ack_set)
		{
			// clear the ack set first
			ack_set.clear();
						
			dtn::data::SDNV size;
			stream >> size;

			for(size_t i = 0; i < size.getValue(); i++)
			{
				ProphetRoutingExtension::Acknowledgement ack;
				stream >> ack;
				ack_set.insert(ack);
			}
			return stream;
		}

		std::ostream& operator<<(std::ostream &stream, const ProphetRoutingExtension::Acknowledgement &ack) {
			stream << ack.bundleID;
			stream << dtn::data::SDNV(ack.expire_time);
			return stream;
		}

		std::istream& operator>>(std::istream &stream, ProphetRoutingExtension::Acknowledgement &ack) {
			dtn::data::SDNV expire_time;
			stream >> ack.bundleID;
			stream >> expire_time;
			ack.expire_time = expire_time.getValue();
			return stream;
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

		ProphetRoutingExtension::ForwardingStrategy::ForwardingStrategy()
		 : _prophet_router(NULL)
		{}

		ProphetRoutingExtension::ForwardingStrategy::~ForwardingStrategy()
		{}

		bool ProphetRoutingExtension::ForwardingStrategy::neighborDPIsGreater(const DeliveryPredictabilityMap& neighbor_dpm, const dtn::data::EID& destination) const
		{
			const DeliveryPredictabilityMap& dp_map = _prophet_router->_deliveryPredictabilityMap;
			const dtn::data::EID destnode = destination.getNode();

			try {
				float local_pv = dp_map.get(destnode);

				try {
					// retrieve the value from the DeliveryPredictabilityMap of the neighbor
					float foreign_pv = neighbor_dpm.get(destnode);

					return (foreign_pv > local_pv);
				} catch (const DeliveryPredictabilityMap::ValueNotFoundException&) {
					// if the foreign router has no entry for the destination
					// then compare the local value with a fictitious initial value
					return (_prophet_router->_p_first_threshold > local_pv);
				}
			} catch (const DeliveryPredictabilityMap::ValueNotFoundException&) {
				// always forward if the destination is not in our own predictability map
			}

			return false;
		}

		void ProphetRoutingExtension::ForwardingStrategy::setProphetRouter(ProphetRoutingExtension *router)
		{
			_prophet_router = router;
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
