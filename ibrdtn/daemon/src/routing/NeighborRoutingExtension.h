/*
 * NeighborRoutingExtension.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef NEIGHBORROUTINGEXTENSION_H_
#define NEIGHBORROUTINGEXTENSION_H_

#include "routing/BaseRouter.h"
#include "routing/NeighborDatabase.h"
#include "core/Node.h"

#include <ibrdtn/data/MetaBundle.h>
#include "ibrdtn/data/EID.h"
#include <ibrcommon/thread/Queue.h>
#include <list>
#include <map>
#include <queue>

namespace dtn
{
	namespace routing
	{
		class NeighborRoutingExtension : public BaseRouter::ThreadedExtension
		{
		public:
			NeighborRoutingExtension();
			virtual ~NeighborRoutingExtension();

			void notify(const dtn::core::Event *evt);

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

			class ProcessBundleTask : public Task
			{
			public:
				ProcessBundleTask(const dtn::data::MetaBundle &meta, const dtn::data::EID &origin);
				virtual ~ProcessBundleTask();

				virtual std::string toString();

				const dtn::data::MetaBundle bundle;
				const dtn::data::EID origin;
			};

			/**
			 * hold queued tasks for later processing
			 */
			ibrcommon::Queue<NeighborRoutingExtension::Task* > _taskqueue;
		};
	}
}

#endif /* NEIGHBORROUTINGEXTENSION_H_ */
