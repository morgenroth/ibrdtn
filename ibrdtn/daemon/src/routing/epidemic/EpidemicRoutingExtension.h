/*
 * EpidemicRoutingExtension.h
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

#ifndef EPIDEMICROUTINGEXTENSION_H_
#define EPIDEMICROUTINGEXTENSION_H_

#include "core/Node.h"
#include "core/AbstractWorker.h"
#include "core/EventReceiver.h"

#include "routing/NodeHandshakeEvent.h"
#include "routing/RoutingExtension.h"
#include "routing/NeighborDatabase.h"

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/BundleString.h>
#include <ibrdtn/data/ExtensionBlock.h>

#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/Thread.h>

#include <list>
#include <queue>

namespace dtn
{
	namespace routing
	{
		class EpidemicRoutingExtension : public RoutingExtension, public ibrcommon::JoinableThread, public dtn::core::EventReceiver<dtn::routing::NodeHandshakeEvent>
		{
			static const std::string TAG;

		public:
			EpidemicRoutingExtension();
			virtual ~EpidemicRoutingExtension();

			virtual const std::string getTag() const throw ();

			virtual void eventDataChanged(const dtn::data::EID &peer) throw ();

			virtual void eventTransferSlotChanged(const dtn::data::EID &peer) throw ();

			virtual void eventBundleQueued(const dtn::data::EID &peer, const dtn::data::MetaBundle &meta) throw ();

			void raiseEvent(const dtn::routing::NodeHandshakeEvent &evt) throw ();
			void componentUp() throw ();
			void componentDown() throw ();

			/**
			 * @see BaseRouter::requestHandshake()
			 */
			virtual void requestHandshake(const dtn::data::EID&, NodeHandshake&) const;

		protected:
			void run() throw ();
			void __cancellation() throw ();

		private:
			class Task
			{
			public:
				virtual ~Task() {};
				virtual std::string toString() = 0;
			};

			class SearchNextBundleTask : public Task
			{
			public:
				SearchNextBundleTask(const dtn::data::EID &eid);
				virtual ~SearchNextBundleTask();

				virtual std::string toString();

				const dtn::data::EID eid;
			};

			/**
			 * hold queued tasks for later processing
			 */
			ibrcommon::Queue<EpidemicRoutingExtension::Task* > _taskqueue;

			// set for pending transfers
			ibrcommon::Mutex _pending_mutex;
			std::set<dtn::data::EID> _pending_peers;
		};
	}
}

#endif /* EPIDEMICROUTINGEXTENSION_H_ */
