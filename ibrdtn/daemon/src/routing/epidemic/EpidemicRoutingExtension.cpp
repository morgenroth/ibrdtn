/*
 * EpidemicRoutingExtension.cpp
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

#include "routing/epidemic/EpidemicRoutingExtension.h"

#include "routing/QueueBundleEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "core/NodeEvent.h"
#include "core/Node.h"
#include "net/ConnectionEvent.h"
#include "net/ConnectionManager.h"
#include "Configuration.h"
#include "core/BundleCore.h"
#include "core/EventDispatcher.h"
#include "core/BundleEvent.h"

#include <ibrdtn/data/MetaBundle.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>

#include <functional>
#include <list>
#include <algorithm>

#include <iomanip>
#include <ios>
#include <iostream>
#include <set>
#include <memory>

#include <stdlib.h>
#include <typeinfo>

namespace dtn
{
	namespace routing
	{
		const std::string EpidemicRoutingExtension::TAG = "EpidemicRoutingExtension";

		EpidemicRoutingExtension::EpidemicRoutingExtension()
		{
			// write something to the syslog
			IBRCOMMON_LOGGER_TAG(EpidemicRoutingExtension::TAG, info) << "Initializing epidemic routing module" << IBRCOMMON_LOGGER_ENDL;
		}

		EpidemicRoutingExtension::~EpidemicRoutingExtension()
		{
			join();
		}

		void EpidemicRoutingExtension::requestHandshake(const dtn::data::EID&, NodeHandshake &request) const
		{
			request.addRequest(BloomFilterSummaryVector::identifier);
		}

		void EpidemicRoutingExtension::eventDataChanged(const dtn::data::EID &peer) throw ()
		{
			// transfer the next bundle to this destination
			_taskqueue.push( new SearchNextBundleTask( peer ) );
		}

		void EpidemicRoutingExtension::eventBundleQueued(const dtn::data::EID &peer, const dtn::data::MetaBundle &meta) throw ()
		{
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

		void EpidemicRoutingExtension::raiseEvent(const NodeHandshakeEvent &handshake) throw ()
		{
			if (handshake.state == NodeHandshakeEvent::HANDSHAKE_COMPLETED)
			{
				// transfer the next bundle to this destination
				_taskqueue.push( new SearchNextBundleTask( handshake.peer ) );
			}
		}

		void EpidemicRoutingExtension::componentUp() throw ()
		{
			dtn::core::EventDispatcher<dtn::routing::NodeHandshakeEvent>::add(this);

			// reset the task queue
			_taskqueue.reset();

			// routine checked for throw() on 15.02.2013
			try {
				// run the thread
				start();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_TAG(EpidemicRoutingExtension::TAG, error) << "componentUp failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void EpidemicRoutingExtension::componentDown() throw ()
		{
			dtn::core::EventDispatcher<dtn::routing::NodeHandshakeEvent>::remove(this);

			try {
				// stop the thread
				stop();
				join();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_TAG(EpidemicRoutingExtension::TAG, error) << "componentDown failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void EpidemicRoutingExtension::__cancellation() throw ()
		{
			_taskqueue.abort();
		}

		void EpidemicRoutingExtension::run() throw ()
		{
			class BundleFilter : public dtn::storage::BundleSelector
			{
			public:
				BundleFilter(const NeighborDatabase::NeighborEntry &entry, const std::set<dtn::core::Node> &neighbors)
				 : _entry(entry), _neighbors(neighbors)
				{};

				virtual ~BundleFilter() {};

				virtual dtn::data::Size limit() const throw () { return _entry.getFreeTransferSlots(); };

				virtual bool addIfSelected(dtn::storage::BundleResult &result, const dtn::data::MetaBundle &meta) const throw (dtn::storage::BundleSelectorException)
				{
					// check Scope Control Block - do not forward bundles with hop limit == 0
					if (meta.hopcount == 0)
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

					// if this is a singleton bundle ...
					if (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
					{
						const dtn::core::Node n(meta.destination.getNode());

						// do not forward the bundle if the final destination is available
						if (_neighbors.find(n) != _neighbors.end())
						{
							return false;
						}
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

					// put the selected bundle with targeted interface into the result-set
					static_cast<RoutingResult&>(result).put(meta, "<insert-iface>");

					return true;
				};

			private:
				const NeighborDatabase::NeighborEntry &_entry;
				const std::set<dtn::core::Node> &_neighbors;
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

					IBRCOMMON_LOGGER_DEBUG_TAG(EpidemicRoutingExtension::TAG, 50) << "processing task " << t->toString() << IBRCOMMON_LOGGER_ENDL;

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
								NeighborDatabase::NeighborEntry &entry = db.get(task.eid, true);

								// check if enough transfer slots available (threshold reached)
								if (!entry.isTransferThresholdReached())
									throw NeighborDatabase::NoMoreTransfersAvailable();

								if (dtn::daemon::Configuration::getInstance().getNetwork().doPreferDirect()) {
									// get current neighbor list
									neighbors = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();
								} else {
									// "prefer direct" option disabled - clear the list of neighbors
									neighbors.clear();
								}

								// get the bundle filter of the neighbor
								const BundleFilter filter(entry, neighbors);

								// some debug output
								IBRCOMMON_LOGGER_DEBUG_TAG(EpidemicRoutingExtension::TAG, 40) << "search some bundles not known by " << task.eid.getString() << IBRCOMMON_LOGGER_ENDL;

								// query some unknown bundle from the storage
								list.clear();
								(**this).getSeeker().get(filter, list);
							} catch (const dtn::storage::BundleSelectorException&) {
								// query a new summary vector from this neighbor
								(**this).doHandshake(task.eid);
							}

							// send the bundles as long as we have resources
							for (RoutingResult::const_iterator iter = list.begin(); iter != list.end(); ++iter)
							{
								try {
									// transfer the bundle to the neighbor
									transferTo(task.eid, (*iter).first);
								} catch (const NeighborDatabase::AlreadyInTransitException&) { };
							}
						} catch (const NeighborDatabase::NoMoreTransfersAvailable &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const NeighborDatabase::NeighborNotAvailableException &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const dtn::storage::NoBundleFoundException &ex) {
							IBRCOMMON_LOGGER_DEBUG_TAG(TAG, 10) << "task " << t->toString() << " aborted: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
						} catch (const std::bad_cast&) { };
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_DEBUG_TAG(EpidemicRoutingExtension::TAG, 20) << "task failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const std::exception &ex) {
					IBRCOMMON_LOGGER_DEBUG_TAG(EpidemicRoutingExtension::TAG, 15) << "terminated due to " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					return;
				}

				yield();
			}
		}

		/****************************************/

		EpidemicRoutingExtension::SearchNextBundleTask::SearchNextBundleTask(const dtn::data::EID &e)
		 : eid(e)
		{ }

		EpidemicRoutingExtension::SearchNextBundleTask::~SearchNextBundleTask()
		{ }

		std::string EpidemicRoutingExtension::SearchNextBundleTask::toString()
		{
			return "SearchNextBundleTask: " + eid.getString();
		}
	}
}
