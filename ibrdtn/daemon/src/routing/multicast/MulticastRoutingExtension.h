/*
 * MulticastRoutingExtension.h
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

#ifndef MULTICASTROUTINGEXTENSION_H_
#define MULTICASTROUTINGEXTENSION_H_

#include "MulticastSubscriptionList.h"

#include "routing/RoutingExtension.h"
#include "routing/NodeHandshakeEvent.h"
#include "core/EventReceiver.h"
#include "core/TimeEvent.h"

#include <ibrdtn/data/EID.h>
#include <ibrcommon/data/File.h>
#include <ibrcommon/thread/Queue.h>

#include <set>

namespace dtn
{
	namespace routing
	{
		class MulticastRoutingExtension : public RoutingExtension, public ibrcommon::JoinableThread,
			public dtn::core::EventReceiver<dtn::routing::NodeHandshakeEvent>,
			public dtn::core::EventReceiver<dtn::core::TimeEvent>
		{
		public:
			static const std::string TAG;

			MulticastRoutingExtension();
			virtual ~MulticastRoutingExtension();

			virtual const std::string getTag() const throw ();

			/* virtual methods from BaseRouter::Extension */
			virtual void requestHandshake(const dtn::data::EID&, NodeHandshake&) const; ///< \see BaseRouter::Extension::requestHandshake
			virtual void responseHandshake(const dtn::data::EID&, const NodeHandshake&, NodeHandshake&); ///< \see BaseRouter::Extension::responseHandshake
			virtual void processHandshake(const dtn::data::EID&, NodeHandshake&); ///< \see BaseRouter::Extension::processHandshake
			virtual void componentUp() throw ();
			virtual void componentDown() throw ();

			virtual void raiseEvent(const dtn::routing::NodeHandshakeEvent &evt) throw ();
			virtual void raiseEvent(const dtn::core::TimeEvent &evt) throw ();

			virtual void eventDataChanged(const dtn::data::EID &peer) throw ();

			virtual void eventTransferSlotChanged(const dtn::data::EID &peer) throw ();

			virtual void eventBundleQueued(const dtn::data::EID &peer, const dtn::data::MetaBundle &meta) throw ();

		protected:
			virtual void run() throw ();
			void __cancellation() throw ();

		private:
			/**
			 * stores all persistent data to a file
			 */
			void store(const ibrcommon::File &target);

			/**
			 * restore all persistent data from a file
			 */
			void restore(const ibrcommon::File &source);

			MulticastSubscriptionList _subscriptions;

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

			/**
			 * hold queued tasks for later processing
			 */
			ibrcommon::Queue<Task* > _taskqueue;

			// set for pending transfers
			ibrcommon::Mutex _pending_mutex;
			std::set<dtn::data::EID> _pending_peers;

			ibrcommon::File _persistent_file; ///< This file is used to store persistent routing data
		};
	}
}

#endif /* MULTICASTROUTINGEXTENSION_H_ */
