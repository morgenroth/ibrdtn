#include "ProphetRoutingExtension.h"

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
			: _forwardingStrategy(strategy), _next_exchange_timeout(next_exchange_timeout), _p_encounter_max(p_encounter_max), _p_encounter_first(p_encounter_first),
			  _p_first_threshold(p_first_threshold), _beta(beta), _gamma(gamma), _delta(delta), _time_unit(time_unit), _i_typ(i_typ)
		{
			// assign myself to the forwarding strategy
			strategy->setProphetRouter(this);

			_deliveryPredictabilityMap[core::BundleCore::local] = 1.0;
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

				_acknowledgementSet.purge();
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
				update(neighbor_dp_map, neighbor_node);

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

					virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const
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

				const std::list<dtn::data::MetaBundle> removeList = storage.get(filter);

				for (std::list<dtn::data::MetaBundle>::const_iterator it = removeList.begin(); it != removeList.end(); ++it)
				{
					const dtn::data::MetaBundle &meta = (*it);

					dtn::core::BundlePurgeEvent::raise(meta);

					IBRCOMMON_LOGGER(notice) << "Bundle removed due to prophet ack: " << meta.toString() << IBRCOMMON_LOGGER_ENDL;

					/* generate a report */
					dtn::core::BundleEvent::raise(meta, dtn::core::BUNDLE_DELETED, dtn::data::StatusReportBlock::DEPLETED_STORAGE);
				}
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
				const std::set<dtn::core::Node> nl = dtn::core::BundleCore::getInstance().getNeighbors();

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

		ibrcommon::ThreadsafeReference<ProphetRoutingExtension::DeliveryPredictabilityMap> ProphetRoutingExtension::getDeliveryPredictabilityMap()
		{
			{
				ibrcommon::MutexLock l(_deliveryPredictabilityMap);
				age();
			}
			return ibrcommon::ThreadsafeReference<DeliveryPredictabilityMap>(_deliveryPredictabilityMap, _deliveryPredictabilityMap);
		}

		ibrcommon::ThreadsafeReference<const ProphetRoutingExtension::DeliveryPredictabilityMap> ProphetRoutingExtension::getDeliveryPredictabilityMap() const
		{
			return ibrcommon::ThreadsafeReference<const DeliveryPredictabilityMap>(_deliveryPredictabilityMap, const_cast<DeliveryPredictabilityMap&>(_deliveryPredictabilityMap));
		}

		ibrcommon::ThreadsafeReference<const ProphetRoutingExtension::AcknowledgementSet> ProphetRoutingExtension::getAcknowledgementSet() const
		{
			return ibrcommon::ThreadsafeReference<const AcknowledgementSet>(_acknowledgementSet, const_cast<AcknowledgementSet&>(_acknowledgementSet));
		}

		void ProphetRoutingExtension::ProphetRoutingExtension::run()
		{
			class BundleFilter : public dtn::storage::BundleStorage::BundleFilterCallback
			{
			public:
				BundleFilter(const NeighborDatabase::NeighborEntry &entry, ForwardingStrategy &strategy)
				 : _entry(entry), _strategy(strategy)
				{};

				virtual ~BundleFilter() {};

				virtual size_t limit() const { return 10; };

				virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const
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

					// do not forward to any blacklisted destination
					const dtn::data::EID dest = meta.destination.getNode();
					if (_blacklist.find(dest) != _blacklist.end())
					{
						return false;
					}

					// do not forward bundles already known by the destination
					// throws BloomfilterNotAvailableException if no filter is available or it is expired
					if (_entry.has(meta, true))
					{
						return false;
					}

					// ask the routing strategy if this bundle should be selected
					if (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
					{
						return _strategy.shallForward(_entry.eid, meta);
					}

					return true;
				}

				void blacklist(const dtn::data::EID& id)
				{
					_blacklist.insert(id);
				}

			private:
				std::set<dtn::data::EID> _blacklist;
				const NeighborDatabase::NeighborEntry &_entry;
				const ForwardingStrategy &_strategy;
			};

			dtn::storage::BundleStorage &storage = (**this).getStorage();

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

							try {
								// get the bundle filter of the neighbor
								BundleFilter filter(entry, *_forwardingStrategy);

								// some debug output
								IBRCOMMON_LOGGER_DEBUG(40) << "search some bundles not known by " << task.eid.getString() << IBRCOMMON_LOGGER_ENDL;

								// blacklist the neighbor itself, because this is handled by neighbor routing extension
								filter.blacklist(task.eid);

								// query some unknown bundle from the storage, the list contains max. 10 items.
								const std::list<dtn::data::MetaBundle> list = storage.get(filter);

								// send the bundles as long as we have resources
								for (std::list<dtn::data::MetaBundle>::const_iterator iter = list.begin(); iter != list.end(); iter++)
								{
									const dtn::data::MetaBundle &meta = (*iter);

									try {
										transferTo(entry, meta);
									} catch (const NeighborDatabase::AlreadyInTransitException&) { };
								}
							} catch (const NeighborDatabase::BloomfilterNotAvailableException&) {
								// query a new summary vector from this neighbor
								(**this).doHandshake(task.eid);
							}
						} catch (const NeighborDatabase::NoMoreTransfersAvailable&) {
						} catch (const NeighborDatabase::NeighborNotAvailableException&) {
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

							std::set<dtn::core::Node> neighbors = dtn::core::BundleCore::getInstance().getNeighbors();
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

		void ProphetRoutingExtension::__cancellation()
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

		const size_t ProphetRoutingExtension::DeliveryPredictabilityMap::identifier = NodeHandshakeItem::DELIVERY_PREDICTABILITY_MAP;
		const size_t ProphetRoutingExtension::AcknowledgementSet::identifier = NodeHandshakeItem::PROPHET_ACKNOWLEDGEMENT_SET;

		size_t ProphetRoutingExtension::DeliveryPredictabilityMap::DeliveryPredictabilityMap::getIdentifier() const
		{
			return identifier;
		}

		size_t ProphetRoutingExtension::DeliveryPredictabilityMap::getLength() const
		{
			size_t len = 0;
			for(const_iterator it = begin(); it != end(); ++it)
			{
				/* calculate length of the EID */
				const std::string eid = it->first.getString();
				size_t eid_len = eid.length();
				len += data::SDNV(eid_len).getLength() + eid_len;

				/* calculate length of the float in fixed notation */
				const float& f = it->second;
				std::stringstream ss;
				ss << f << std::flush;

				size_t float_len = ss.str().length();
				len += data::SDNV(float_len).getLength() + float_len;
			}
			return data::SDNV(size()).getLength() + len;
		}

		std::ostream& ProphetRoutingExtension::DeliveryPredictabilityMap::serialize(std::ostream& stream) const
		{
			stream << data::SDNV(size());
			for(const_iterator it = begin(); it != end(); ++it)
			{
				const std::string eid = it->first.getString();
				stream << data::SDNV(eid.length()) << eid;

				const float& f = it->second;
				/* write f into a stringstream to get final length */
				std::stringstream ss;
				ss << f << std::flush;

				stream << data::SDNV(ss.str().length());
				stream << ss.str();
			}
			IBRCOMMON_LOGGER_DEBUG(20) << "ProphetRouting: Serialized DeliveryPredictabilityMap with " << size() << " items." << IBRCOMMON_LOGGER_ENDL;
			IBRCOMMON_LOGGER_DEBUG(60) << *this << IBRCOMMON_LOGGER_ENDL;
			return stream;
		}

		std::istream& ProphetRoutingExtension::DeliveryPredictabilityMap::deserialize(std::istream& stream)
		{
			data::SDNV elements_read(0);
			data::SDNV map_size;
			stream >> map_size;

			while(elements_read < map_size)
			{
				/* read the EID */
				data::SDNV eid_len;
				stream >> eid_len;
				char eid_cstr[eid_len.getValue()+1];
				stream.read(eid_cstr, sizeof(eid_cstr)-1);
				eid_cstr[sizeof(eid_cstr)-1] = 0;
				data::EID eid(eid_cstr);
				if(eid == data::EID())
					throw dtn::InvalidDataException("EID could not be casted, while parsing a dp_map.");

				/* read the probability (float) */
				float f;
				data::SDNV float_len;
				stream >> float_len;
				char f_cstr[float_len.getValue()+1];
				stream.read(f_cstr, sizeof(f_cstr)-1);
				f_cstr[sizeof(f_cstr)-1] = 0;

				std::stringstream ss(f_cstr);
				ss >> f;
				if(ss.fail())
					throw dtn::InvalidDataException("Float could not be casted, while parsing a dp_map.");

				/* check if f is in a proper range */
				if(f < 0 || f > 1)
					continue;

				/* insert the data into the map */
				(*this)[eid] = f;

				elements_read += 1;
			}

			IBRCOMMON_LOGGER_DEBUG(20) << "ProphetRouting: Deserialized DeliveryPredictabilityMap with " << size() << " items." << IBRCOMMON_LOGGER_ENDL;
			IBRCOMMON_LOGGER_DEBUG(60) << *this << IBRCOMMON_LOGGER_ENDL;
			return stream;
		}

		void ProphetRoutingExtension::updateNeighbor(const dtn::data::EID &neighbor)
		{
			/**
			 * Calculate new value for this encounter
			 */
			DeliveryPredictabilityMap::const_iterator it;
			float neighbor_dp = _p_encounter_first;

			if ((it = _deliveryPredictabilityMap.find(neighbor)) != _deliveryPredictabilityMap.end())
			{
				neighbor_dp = it->second;

				if(it->second < _p_first_threshold)
				{
					neighbor_dp = _p_encounter_first;
				}
				else
				{
					neighbor_dp += (1 - _delta - it->second) * p_encounter(neighbor);
				}
			}

			_deliveryPredictabilityMap[neighbor] = neighbor_dp;
			_ageMap[neighbor] = dtn::utils::Clock::getUnixTimestamp();
		}

		void ProphetRoutingExtension::update(const DeliveryPredictabilityMap& neighbor_dp_map, const dtn::data::EID& neighbor)
		{
			DeliveryPredictabilityMap::const_iterator it;
			float neighbor_dp = _p_encounter_first;

			if ((it = _deliveryPredictabilityMap.find(neighbor)) != _deliveryPredictabilityMap.end())
			{
				neighbor_dp = it->second;
			}

			/**
			 * Calculate transitive values
			 */
			for (it = neighbor_dp_map.begin(); it != neighbor_dp_map.end(); ++it)
			{
				if ((it->first != neighbor) && (it->first != dtn::core::BundleCore::local))
				{
					float dp = 0;

					DeliveryPredictabilityMap::iterator dp_it;
					if((dp_it = _deliveryPredictabilityMap.find(it->first)) != _deliveryPredictabilityMap.end())
						dp = dp_it->second;

					dp = max(dp, neighbor_dp * it->second * _beta);

					if(dp_it != _deliveryPredictabilityMap.end())
						dp_it->second = dp;
					else
						_deliveryPredictabilityMap[it->first] = dp;
				}
			}
		}

		void ProphetRoutingExtension::age()
		{
			size_t current_time = dtn::utils::Clock::getUnixTimestamp();

			// prevent double aging
			if (current_time <= _lastAgingTime) return;

			unsigned int k = (current_time - _lastAgingTime) / _time_unit;

			DeliveryPredictabilityMap::iterator it;
			for(it = _deliveryPredictabilityMap.begin(); it != _deliveryPredictabilityMap.end();)
			{
				if(it->first == dtn::core::BundleCore::local)
				{
					++it;
					continue;
				}

				it->second *= pow(_gamma, (int)k);

				if(it->second < _p_first_threshold)
				{
					_deliveryPredictabilityMap.erase(it++);
				} else {
					++it;
				}
			}

			_lastAgingTime = current_time;
		}

		std::ostream& operator<<(std::ostream& stream, const ProphetRoutingExtension::DeliveryPredictabilityMap& map)
		{
			ProphetRoutingExtension::DeliveryPredictabilityMap::const_iterator it;
			for(it = map.begin(); it != map.end(); ++it)
			{
				stream << it->first.getString() << ": " << it->second << std::endl;
			}

			return  stream;
		}

		std::ostream& operator<<(std::ostream& stream, const ProphetRoutingExtension::AcknowledgementSet& ack_set)
		{
			std::set<ProphetRoutingExtension::Acknowledgement>::const_iterator it;
			for(it = ack_set._ackSet.begin(); it != ack_set._ackSet.end(); ++it)
			{
				stream << it->bundleID.toString() << std::endl;
				stream << it->expire_time << std::endl;
			}

			return stream;
		}

		ProphetRoutingExtension::Acknowledgement::Acknowledgement()
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
				_ackSet.insert(*it);
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
		size_t ProphetRoutingExtension::AcknowledgementSet::getLength() const
		{
			std::stringstream ss;
			serialize(ss);
			return ss.str().length();
		}

		std::ostream& ProphetRoutingExtension::AcknowledgementSet::serialize(std::ostream& stream) const
		{
			stream << dtn::data::SDNV(_ackSet.size());
			std::set<Acknowledgement>::const_iterator it;
			for(it = _ackSet.begin(); it != _ackSet.end(); ++it)
			{
				it->serialize(stream);
			}
			return stream;
		}

		std::istream& ProphetRoutingExtension::AcknowledgementSet::deserialize(std::istream& stream)
		{
			dtn::data::SDNV size;
			stream >> size;

			for(dtn::data::SDNV i = 0; i < size; i += 1)
			{
				Acknowledgement ack;
				ack.deserialize(stream);
				_ackSet.insert(ack);
			}
			return stream;
		}

		std::ostream& ProphetRoutingExtension::Acknowledgement::serialize(std::ostream& stream) const
		{
			stream << bundleID;
			stream << dtn::data::SDNV(expire_time);
			return stream;
		}

		std::istream& ProphetRoutingExtension::Acknowledgement::deserialize(std::istream& stream)
		{
			dtn::data::SDNV expire_time;
			stream >> bundleID;
			stream >> expire_time;
			this->expire_time = expire_time.getValue();
			/* TODO only accept entries with bundles that we routed ourselves? */

			if (BundleCore::max_lifetime > 0 && this->expire_time > BundleCore::max_lifetime)
				this->expire_time = BundleCore::max_lifetime;

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

		bool ProphetRoutingExtension::ForwardingStrategy::neighborDPIsGreater(const dtn::data::EID& neighbor, const dtn::data::EID& destination) const
		{
			bool ret = false;
			const DeliveryPredictabilityMap& dp_map = _prophet_router->_deliveryPredictabilityMap;

			DeliveryPredictabilityMap::const_iterator neighborIT = dp_map.find(neighbor);
			DeliveryPredictabilityMap::const_iterator destinationIT = dp_map.find(destination);

			if(destinationIT == dp_map.end()) {
				ret = true;
			} else {
				float neighbor_dp = _prophet_router->_p_first_threshold;
				if(neighborIT != dp_map.end()) {
					neighbor_dp = neighborIT->second;
				}
				ret = (neighbor_dp > destinationIT->second);
			}

			return ret;
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

		bool ProphetRoutingExtension::GRTR_Strategy::shallForward(const dtn::data::EID& neighbor, const dtn::data::MetaBundle& bundle) const
		{
			return neighborDPIsGreater(neighbor, bundle.destination);
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

		bool ProphetRoutingExtension::GTMX_Strategy::shallForward(const dtn::data::EID &neighbor, const dtn::data::MetaBundle& bundle) const
		{
			unsigned int NF = 0;

			nf_map::const_iterator nf_it = _NF_map.find(bundle);
			if(nf_it != _NF_map.end()) {
				NF = nf_it->second;
			}

			if (NF > _NF_max) return false;

			return neighborDPIsGreater(neighbor, bundle.destination);
		}

	} // namespace routing
} // namespace dtn
