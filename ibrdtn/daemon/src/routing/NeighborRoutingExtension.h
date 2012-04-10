/*
 * NeighborRoutingExtension.h
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
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
