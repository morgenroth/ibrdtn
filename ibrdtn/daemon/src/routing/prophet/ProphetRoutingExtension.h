/*
 * ProphetRoutingExtension.h
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

#ifndef PROPHETROUTINGEXTENSION_H_
#define PROPHETROUTINGEXTENSION_H_

#include "routing/BaseRouter.h"
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/ThreadsafeReference.h>

#include <map>
#include <list>

#define DISABLE_MAP_STORE 1

namespace dtn
{
	namespace routing
	{
		/*!
		 * \brief Routing extension for PRoPHET routing
		 *
		 * PRoPHET keeps track of predictabilities to see specific other nodes.
		 * i.e. Nodes that are seen often will have a high predictabilites attached
		 * and forwarding decisions can be made based on these predictabilites.
		 * In addition, these predictabilites are updated transitively by exchanging
		 * predictabilityMaps with neighbors.
		 * For a detailed description of the protocol, see draft-irtf-dtnrg-prophet-09
		 */
		class ProphetRoutingExtension : public BaseRouter::ThreadedExtension
		{
		public:

			/*!
			 * \brief This class keeps track of the predictablities to see a specific EID
			 *
			 * This class can be used as a map from EID to float.
			 * Also, it can be serialized as a NodeHandshakeItem to be exchanged with neighbors.
			 */
			class DeliveryPredictabilityMap : public std::map<dtn::data::EID, float>, public NodeHandshakeItem, public ibrcommon::Mutex
			{
			public:
				/* virtual methods from NodeHandshakeItem */

				virtual size_t getIdentifier() const; ///< \see NodeHandshakeItem::getIdentifier
				virtual size_t getLength() const; ///< \see NodeHandshakeItem::getLength
				virtual std::ostream& serialize(std::ostream& stream) const; ///< \see NodeHandshakeItem::serialize
				virtual std::istream& deserialize(std::istream& stream); ///< \see NodeHandshakeItem::deserialize
				static const size_t identifier;

				/* TODO: move this to a deserializer or toString? */
				friend std::ostream& operator<<(ostream&, const DeliveryPredictabilityMap&);
			};

			/*!
			 * \brief This class is a abstract base class for all prophet forwarding strategies.
			 */
			class ForwardingStrategy
			{
			public:
				ForwardingStrategy();
				virtual ~ForwardingStrategy() = 0;
				/*!
				 * The prophetRoutingExtension calls this function for every bundle that can be forwarded to a neighbor
				 * and forwards it depending on the return value.
				 * \param neighbor the neighbor to forward to
				 * \param bundle the bundle that can be forwarded
				 * \param prophet_router Reference to the ProphetRoutingExtension to access its parameters
				 * \return true if the bundle should be forwarded
				 */
				virtual bool shallForward(const dtn::data::EID& neighbor, const dtn::data::MetaBundle& bundle) const = 0;
				/*!
				 * checks if the deliveryPredictability of the neighbor is higher than that of the destination of the bundle.
				 */
				bool neighborDPIsGreater(const dtn::data::EID& neighbor, const dtn::data::EID& destination) const;

				/**
				 * Set back-reference to the prophet router
				 */
				void setProphetRouter(ProphetRoutingExtension *router);

			protected:
				ProphetRoutingExtension *_prophet_router;
			};

			/*!
			 * \brief Represents an Acknowledgement, i.e. the bundleID that is acknowledged
			 * and the lifetime of the acknowledgement
			 */
			class Acknowledgement
			{
			public:
				Acknowledgement();
				Acknowledgement(const dtn::data::BundleID&, size_t expire_time);
				virtual ~Acknowledgement();
				std::ostream& serialize(std::ostream& stream) const;
				std::istream& deserialize(std::istream& stream);

				bool operator<(const Acknowledgement &other) const;

				dtn::data::BundleID bundleID; ///< The BundleID that is acknowledged
				size_t expire_time; ///< The expire time of the acknowledgement, given in the same format as the bundle timestamps
			};

			/*!
			 * \brief Set of Acknowledgements, that can be serialized in node handshakes.
			 */
			class AcknowledgementSet : public NodeHandshakeItem, public ibrcommon::Mutex
			{
			public:
				AcknowledgementSet();
				AcknowledgementSet(const AcknowledgementSet&);

				void insert(const Acknowledgement&); ///< Insert an Acknowledgement into the set
				void purge(); ///< purge the set, i.e. remove acknowledgements that have expired
				void merge(const AcknowledgementSet&); ///< merge the set with a second AcknowledgementSet
				bool has(const dtn::data::BundleID &bundle) const; ///< check if an acknowledgement exists for the bundle

				/* virtual methods from NodeHandshakeItem */
				virtual size_t getIdentifier() const; ///< \see NodeHandshakeItem::getIdentifier
				virtual size_t getLength() const; ///< \see NodeHandshakeItem::getLength
				virtual std::ostream& serialize(std::ostream& stream) const; ///< \see NodeHandshakeItem::serialize
				virtual std::istream& deserialize(std::istream& stream); ///< \see NodeHandshakeItem::deserialize
				static const size_t identifier;

				/* TODO: move this to a deserializer or toString? */
				friend std::ostream& operator<<(ostream&, const AcknowledgementSet&);
			private:
				std::set<Acknowledgement> _ackSet;
			};

			ProphetRoutingExtension(ForwardingStrategy *strategy, float p_encounter_max, float p_encounter_first,
						float p_first_threshold, float beta, float gamma, float delta,
						size_t time_unit, size_t i_typ,
						size_t next_exchange_timeout);
			virtual ~ProphetRoutingExtension();

			/* virtual methods from BaseRouter::Extension */
			virtual void requestHandshake(const dtn::data::EID&, NodeHandshake&) const; ///< \see BaseRouter::Extension::requestHandshake
			virtual void responseHandshake(const dtn::data::EID&, const NodeHandshake&, NodeHandshake&); ///< \see BaseRouter::Extension::responseHandshake
			virtual void processHandshake(const dtn::data::EID&, NodeHandshake&); ///< \see BaseRouter::Extension::processHandshake
			virtual void notify(const dtn::core::Event *evt);///< \see BaseRouter::Extension::notifiy

			/*!
			 * Returns a threadsafe reference to the DeliveryPredictabilityMap. I.e. the corresponding
			 * Mutex is locked while this object exists.
			 * In addition, the map is aged before it is returned.
			 */
			ibrcommon::ThreadsafeReference<DeliveryPredictabilityMap> getDeliveryPredictabilityMap();
			/*!
			 * Returns a threadsafe reference to the DeliveryPredictabilityMap. I.e. the corresponding
			 * Mutex is locked while this object exists.
			 * This const version does not age the map.
			 */
			ibrcommon::ThreadsafeReference<const DeliveryPredictabilityMap> getDeliveryPredictabilityMap() const;

			/*!
			 * Returns a threadsafe reference to the AcknowledgementSet. I.e. the corresponding
			 * Mutex is locked while this object exists.
			 */
			ibrcommon::ThreadsafeReference<const AcknowledgementSet> getAcknowledgementSet() const;
		protected:
			virtual void run() throw ();
			void __cancellation() throw ();
		private:
			/*!
			 * Updates the DeliveryPredictabilityMap with one recieved by a neighbor.
			 * \param neighbor_dp_map the DeliveryPredictabilityMap recieved from the neighbor
			 * \param neighbor EID of the neighbor
			 * \warning The _deliveryPredictabilityMap has to be locked before calling this function
			 */
			void update(const DeliveryPredictabilityMap& neighbor_dp_map, const dtn::data::EID& neighbor);

			/*!
			 * Updates the DeliveryPredictabilityMap in the event that a neighbor has been encountered.
			 * \warning The _deliveryPredictabilityMap has to be locked before calling this function
			 */
			void updateNeighbor(const dtn::data::EID& neighbor);

			/*!
			 * Age all entries in the DeliveryPredictabilityMap.
			 * \warning The _deliveryPredictabilityMap has to be locked before calling this function
			 */
			void age();

			DeliveryPredictabilityMap _deliveryPredictabilityMap;
			ForwardingStrategy *_forwardingStrategy;
			AcknowledgementSet _acknowledgementSet;

#ifndef DISABLE_MAP_STORE
			typedef std::map<dtn::data::EID, DeliveryPredictabilityMap> map_store;
			/*!
			 * Store for old DeliveryPredictabilityMaps.
			 * This is needed in the two-way-handshake, to guarantee that both nodes receive the
			 * versions of the maps before they have been updated in this encounter.
			 */
			map_store _mapStore;
#endif

			ibrcommon::Mutex _next_exchange_mutex; ///< Mutex for the _next_exchange_timestamp.
			size_t _next_exchange_timeout; ///< Interval in seconds how often Handshakes should be executed on longer connections.
			size_t _next_exchange_timestamp; ///< Unix timestamp, when the next handshake is due.

			/*!
			 * Calculates the p_encounter that rises linearly with the time since the encounter, up to p_encounter_max.
			 */
			float p_encounter(const dtn::data::EID &neighbor) const;

			float _p_encounter_max; ///< Maximum value returned by p_encounter. \see p_encounter
			float _p_encounter_first; ///< Value to which the predictability is set on the first encounter.
			float _p_first_threshold; ///< If the predictability falls below this value, it is erased from the map and _p_encounter_first will be used on the next encounter.
			float _beta; ///< Weight of the transitive property of prophet.
			float _gamma; ///< Determines how quickly predictabilities age.
			float _delta; ///< Maximum predictability is (1-delta).
			size_t _lastAgingTime; ///< Timestamp when the map has been aged the last time.
			size_t _time_unit; ///< time unit to be used in the network
			size_t _i_typ; ///< time interval that is characteristic for the network

			typedef std::map<dtn::data::EID, size_t> age_map;
			age_map _ageMap; ///< map with time for each neighbor, when the last encounter happened

			class Task
			{
			public:
				virtual ~Task() {};
				virtual std::string toString() const = 0;
			};

			class SearchNextBundleTask : public Task
			{
			public:
				SearchNextBundleTask(const dtn::data::EID &eid);
				virtual ~SearchNextBundleTask();

				virtual std::string toString() const;

				const dtn::data::EID eid;
			};

			class NextExchangeTask : public Task
			{
			public:
				NextExchangeTask();
				virtual ~NextExchangeTask();

				virtual std::string toString() const;
			};

			ibrcommon::Queue<Task* > _taskqueue;

		public:
			/*!
			 * \brief The GRTR forwarding strategy.
			 * Using this strategy, packets are forwarding, if the neighbor
			 * has a higher predictability then the destination.
			 */
			class GRTR_Strategy : public ForwardingStrategy
			{
			public:
				explicit GRTR_Strategy();
				virtual ~GRTR_Strategy();
				virtual bool shallForward(const dtn::data::EID& neighbor, const dtn::data::MetaBundle& bundle) const;
			};

			/*!
			 * \brief The GTMX forwarding strategy.
			 * Using this strategy, packets are forwarding, if the neighbor
			 * has a higher predictability then the destination, but at most
			 * NF_max times.
			 */
			class GTMX_Strategy : public ForwardingStrategy
			{
			public:
				explicit GTMX_Strategy(unsigned int NF_max);
				virtual ~GTMX_Strategy();
				virtual bool shallForward(const dtn::data::EID& neighbor, const dtn::data::MetaBundle& bundle) const;

				void addForward(const dtn::data::BundleID &id);

			private:
				unsigned int _NF_max; ///< Number how often a bundle should be forwarded at most.

				typedef std::map<dtn::data::BundleID, unsigned int> nf_map;
				nf_map _NF_map; ///< Map where the number of forwards are saved.
			};
		};

		std::ostream& operator<<(std::ostream&, const ProphetRoutingExtension::AcknowledgementSet&);

	} // namespace routing
} // namespace dtn

#endif // PROPHETROUTINGEXTENSION_H_
