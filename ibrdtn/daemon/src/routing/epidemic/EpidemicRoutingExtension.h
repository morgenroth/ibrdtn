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

#include "routing/SummaryVector.h"
#include "routing/BaseRouter.h"
#include "routing/NeighborDatabase.h"

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/BundleString.h>
#include <ibrdtn/data/ExtensionBlock.h>

#include <ibrcommon/thread/Queue.h>

#include <list>
#include <queue>

namespace dtn
{
	namespace routing
	{
		class EpidemicRoutingExtension : public BaseRouter::ThreadedExtension
		{
		public:
			EpidemicRoutingExtension();
			virtual ~EpidemicRoutingExtension();

			void notify(const dtn::core::Event *evt);

			/**
			 * @see BaseRouter::requestHandshake()
			 */
			virtual void requestHandshake(const dtn::data::EID&, NodeHandshake&) const;

		protected:
			void run();
			void __cancellation();

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
		};
	}
}

#endif /* EPIDEMICROUTINGEXTENSION_H_ */
