/*
 * MulticastRoutingExtension.cpp
 *
 * Copyright (C) 2016 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <jm@m-network.de>
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

#include "MulticastRoutingExtension.h"

#include "core/BundleCore.h"
#include "core/EventDispatcher.h"

#include <ibrcommon/Logger.h>

#include <memory>

namespace dtn
{
	namespace routing
	{
		const std::string MulticastRoutingExtension::TAG = "MulticastRoutingExtension";

		MulticastRoutingExtension::MulticastRoutingExtension()
		{
			try {
				// set file to store prophet data
				ibrcommon::File routing_d = dtn::daemon::Configuration::getInstance().getPath("storage").get("routing");

				// create directory if necessary
				if (!routing_d.isDirectory()) ibrcommon::File::createDirectory(routing_d);

				// assign file within the routing data directory
				_persistent_file = routing_d.get("multicast.dat");
			} catch (const dtn::daemon::Configuration::ParameterNotSetException&) {
				// no path set
			}

			// write something to the syslog
			IBRCOMMON_LOGGER_TAG(MulticastRoutingExtension::TAG, info) << "Initializing multicast routing module" << IBRCOMMON_LOGGER_ENDL;
		}

		MulticastRoutingExtension::~MulticastRoutingExtension()
		{
			stop();
			join();
		}

		void MulticastRoutingExtension::requestHandshake(const dtn::data::EID&, NodeHandshake& handshake) const
		{
			handshake.addRequest(MulticastSubscriptionList::identifier);

			// request summary vector to exclude bundles known by the peer
			handshake.addRequest(BloomFilterSummaryVector::identifier);
		}

		void MulticastRoutingExtension::responseHandshake(const dtn::data::EID& neighbor, const NodeHandshake& request, NodeHandshake& response)
		{
			if (request.hasRequest(MulticastSubscriptionList::identifier))
			{
				ibrcommon::MutexLock l(_subscriptions);
				response.addItem(new MulticastSubscriptionList(_subscriptions));
			}
		}

		void MulticastRoutingExtension::processHandshake(const dtn::data::EID& neighbor, NodeHandshake& response)
		{
			try {
				const MulticastSubscriptionList& subscriptions = response.get<MulticastSubscriptionList>();

				// strip possible application part off the neighbor EID
				const dtn::data::EID neighbor_node = neighbor.getNode();

				IBRCOMMON_LOGGER_DEBUG_TAG(MulticastRoutingExtension::TAG, 10) << "subscriptions received from " << neighbor_node.getString() << IBRCOMMON_LOGGER_ENDL;

				// store a copy of the map in the neighbor database
				try {
					NeighborDatabase &db = (**this).getNeighborDB();
					NeighborDataset ds(new MulticastSubscriptionList(subscriptions));

					ibrcommon::MutexLock l(db);
					db.get(neighbor_node).putDataset(ds);
				} catch (const NeighborDatabase::EntryNotFoundException&) { };

				/* merge received subscription map */
				ibrcommon::MutexLock l(_subscriptions);
				_subscriptions.merge(subscriptions);
			} catch (std::exception&) { }
		}

		void MulticastRoutingExtension::eventDataChanged(const dtn::data::EID &peer) throw ()
		{
			// transfer the next bundle to this destination
			_taskqueue.push( new SearchNextBundleTask( peer ) );
		}

		void MulticastRoutingExtension::eventTransferSlotChanged(const dtn::data::EID &peer) throw ()
		{
			ibrcommon::MutexLock pending_lock(_pending_mutex);
			if (_pending_peers.find(peer) != _pending_peers.end()) {
				_pending_peers.erase(peer);
				eventDataChanged(peer);
			}
		}

		void MulticastRoutingExtension::eventBundleQueued(const dtn::data::EID &peer, const dtn::data::MetaBundle &meta) throw ()
		{
			// ignore the bundle if the scope is limited to local delivery
			if ((meta.hopcount <= 1) && (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))) return;

			// new bundles trigger a recheck for all neighbors
			const std::set<dtn::core::Node> nl = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();

			for (std::set<dtn::core::Node>::const_iterator iter = nl.begin(); iter != nl.end(); ++iter)
			{
				const dtn::core::Node &n = (*iter);

				if (n.getEID() != peer)
				{
					// trigger all routing modules to search for bundles to forward
					eventDataChanged(n.getEID());
				}
			}
		}

		void MulticastRoutingExtension::raiseEvent(const dtn::core::TimeEvent &time) throw ()
		{
			// expire bundles in the acknowledgment set
			{
				ibrcommon::MutexLock l(_subscriptions);
				_subscriptions.expire(time.getTimestamp());
			}

			if ((time.getTimestamp().get<size_t>() % 60) == 0)
			{
				// store persistent data to disk
				if (_persistent_file.isValid()) store(_persistent_file);
			}
		}

		void MulticastRoutingExtension::raiseEvent(const NodeHandshakeEvent &handshake) throw ()
		{
			if (handshake.state == NodeHandshakeEvent::HANDSHAKE_COMPLETED)
			{
				// transfer the next bundle to this destination
				_taskqueue.push( new SearchNextBundleTask( handshake.peer ) );
			}
		}

		void MulticastRoutingExtension::componentUp() throw ()
		{
			dtn::core::EventDispatcher<dtn::routing::NodeHandshakeEvent>::add(this);
			dtn::core::EventDispatcher<dtn::core::TimeEvent>::add(this);

			// reset task queue
			_taskqueue.reset();

			// restore persistent routing data
			if (_persistent_file.exists()) restore(_persistent_file);

			// routine checked for throw() on 15.02.2013
			try {
				// run the thread
				start();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_TAG(MulticastRoutingExtension::TAG, error) << "componentUp failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void MulticastRoutingExtension::componentDown() throw ()
		{
			dtn::core::EventDispatcher<dtn::routing::NodeHandshakeEvent>::remove(this);
			dtn::core::EventDispatcher<dtn::core::TimeEvent>::remove(this);

			// store persistent routing data
			if (_persistent_file.isValid()) store(_persistent_file);

			try {
				// stop the thread
				stop();
				join();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_TAG(MulticastRoutingExtension::TAG, error) << "componentDown failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		const std::string MulticastRoutingExtension::getTag() const throw ()
		{
			return "multicast";
		}

		void MulticastRoutingExtension::run() throw ()
		{
			class BundleFilter : public dtn::storage::BundleSelector
			{
			public:
				BundleFilter(const NeighborDatabase::NeighborEntry &entry, const dtn::core::FilterContext &context, const dtn::net::ConnectionManager::protocol_list &plist)
				 : _entry(entry), _plist(plist), _context(context)
				{ };

				virtual ~BundleFilter() {};

				virtual dtn::data::Size limit() const throw () { return _entry.getFreeTransferSlots(); };

				virtual bool addIfSelected(dtn::storage::BundleResult &result, const dtn::data::MetaBundle &meta) const throw (dtn::storage::BundleSelectorException)
				{
					// check Scope Control Block - do not forward bundles with hop limit == 0
					if (meta.hopcount == 0)
					{
						return false;
					}

					// we only handle non-singleton destinations
					if (!meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
					{
						return false;
					}

					// check Scope Control Block - do not forward bundles with hop limit <= 1
					if (meta.hopcount <= 1)
					{
						return false;
					}

					// request limits from neighbor database
					try {
						const RoutingLimitations &limits = _entry.getDataset<RoutingLimitations>();

						if (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
						{
							// check if the peer accepts bundles for other nodes
							if (limits.getLimit(RoutingLimitations::LIMIT_LOCAL_ONLY) > 0) return false;
						}
						else
						{
							// check if destination permits non-singleton bundles
							if (limits.getLimit(RoutingLimitations::LIMIT_SINGLETON_ONLY) > 0) return false;
						}

						// check if the payload is too large for the neighbor
						if ((limits.getLimit(RoutingLimitations::LIMIT_FOREIGN_BLOCKSIZE) > 0) &&
							((size_t)limits.getLimit(RoutingLimitations::LIMIT_FOREIGN_BLOCKSIZE) < meta.getPayloadLength())) return false;
					} catch (const NeighborDatabase::DatasetNotAvailableException&) { }

					// check if the neighbor data is up-to-date
					if (!_entry.isFilterValid()) throw dtn::storage::BundleSelectorException();

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

					// update filter context
					dtn::core::FilterContext context = _context;
					context.setMetaBundle(meta);

					// check bundle filter for each possible path
					for (dtn::net::ConnectionManager::protocol_list::const_iterator it = _plist.begin(); it != _plist.end(); ++it)
					{
						const dtn::core::Node::Protocol &p = (*it);

						// update context with current protocol
						context.setProtocol(p);

						// execute filtering
						dtn::core::BundleFilter::ACTION ret = dtn::core::BundleCore::getInstance().evaluate(dtn::core::BundleFilter::ROUTING, context);

						if (ret == dtn::core::BundleFilter::ACCEPT)
						{
							// put the selected bundle with targeted interface into the result-set
							static_cast<RoutingResult&>(result).put(meta, p);
							return true;
						}
					}

					return false;
				}

			private:
				const NeighborDatabase::NeighborEntry &_entry;
				const dtn::net::ConnectionManager::protocol_list &_plist;
				const dtn::core::FilterContext &_context;
			};

			// list for bundles
			RoutingResult list;

			// set of known neighbors
			std::set<dtn::core::Node> neighbors;

			while (true)
			{
				try {
					Task *t = _taskqueue.poll();
					std::auto_ptr<Task> killer(t);

					IBRCOMMON_LOGGER_DEBUG_TAG(MulticastRoutingExtension::TAG, 50) << "processing task " << t->toString() << IBRCOMMON_LOGGER_ENDL;

					try {
						/**
						 * SearchNextBundleTask triggers a search for a bundle to transfer
						 * to another host. This Task is generated by TransferCompleted, TransferAborted
						 * and node events.
						 */
						try {
							SearchNextBundleTask &task = dynamic_cast<SearchNextBundleTask&>(*t);

							// clear the result list
							list.clear();

							// lock the neighbor database while searching for bundles
							try {
								NeighborDatabase &db = (**this).getNeighborDB();

								ibrcommon::MutexLock l(db);
								NeighborDatabase::NeighborEntry &entry = db.get(task.eid, true);

								// check if enough transfer slots available (threshold reached)
								if (!entry.isTransferThresholdReached())
									throw NeighborDatabase::NoMoreTransfersAvailable(task.eid);

								// get a list of protocols supported by both, the local BPA and the remote peer
								const dtn::net::ConnectionManager::protocol_list plist =
										dtn::core::BundleCore::getInstance().getConnectionManager().getSupportedProtocols(entry.eid);

								// create a filter context
								dtn::core::FilterContext context;
								context.setPeer(entry.eid);
								context.setRouting(*this);

								// get the bundle filter of the neighbor
								const BundleFilter filter(entry, context, plist);

								// some debug output
								IBRCOMMON_LOGGER_DEBUG_TAG(MulticastRoutingExtension::TAG, 40) << "search some bundles not known by " << task.eid.getString() << IBRCOMMON_LOGGER_ENDL;

								// query some unknown bundle from the storage, the list contains max. 10 items.
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
							for (RoutingResult::const_iterator iter = list.begin(); iter != list.end(); ++iter)
							{
								try {
									transferTo(task.eid, (*iter).first, (*iter).second);
								} catch (const NeighborDatabase::AlreadyInTransitException&) { };
							}
						} catch (const NeighborDatabase::NoMoreTransfersAvailable &ex) {
							// remember that this peer has pending transfers
							ibrcommon::MutexLock pending_lock(_pending_mutex);
							_pending_peers.insert(ex.peer);

							IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const NeighborDatabase::EntryNotFoundException &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const dtn::storage::NoBundleFoundException &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
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
						IBRCOMMON_LOGGER_DEBUG_TAG(MulticastRoutingExtension::TAG, 20) << "task failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const std::exception &ex) {
					IBRCOMMON_LOGGER_DEBUG_TAG(MulticastRoutingExtension::TAG, 15) << "terminated due to " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					return;
				}

				yield();
			}
		}

		void MulticastRoutingExtension::__cancellation() throw ()
		{
			_taskqueue.abort();
		}

		/**
		 * stores all persistent data to a file
		 */
		void MulticastRoutingExtension::store(const ibrcommon::File &target)
		{
			// open the file
			std::ofstream output(target.getPath().c_str());

			// silently fail
			if (!output.good()) return;

			// lock the subscription map
			ibrcommon::MutexLock l(_subscriptions);

			// store the subscription map
			_subscriptions.store(output);

			// store the subscriptions
			{
				ibrcommon::MutexLock l(_subscriptions);
				_subscriptions.store(output);
			}
		}

		/**
		 * restore all persistent data from a file
		 */
		void MulticastRoutingExtension::restore(const ibrcommon::File &source)
		{
			// open the file
			std::ifstream input(source.getPath().c_str());

			// silently fail
			if (!input.good()) return;

			// lock the subscription map
			ibrcommon::MutexLock l(_subscriptions);

			// restore the subscription map
			_subscriptions.restore(input);

			// restore the ack'set
			{
				ibrcommon::MutexLock l(_subscriptions);
				_subscriptions.restore(input);
			}
		}

		MulticastRoutingExtension::SearchNextBundleTask::SearchNextBundleTask(const dtn::data::EID &eid)
			: eid(eid)
		{
		}

		MulticastRoutingExtension::SearchNextBundleTask::~SearchNextBundleTask()
		{
		}

		std::string MulticastRoutingExtension::SearchNextBundleTask::toString() const
		{
			return "SearchNextBundleTask: " + eid.getString();
		}

		MulticastRoutingExtension::NextExchangeTask::NextExchangeTask()
		{
		}

		MulticastRoutingExtension::NextExchangeTask::~NextExchangeTask()
		{
		}

		std::string MulticastRoutingExtension::NextExchangeTask::toString() const
		{
			return "NextExchangeTask";
		}
	}
}
