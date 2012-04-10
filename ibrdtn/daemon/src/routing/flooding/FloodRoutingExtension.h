/*
 * EpidemicRoutingExtension.h
 *
 *  Created on: 18.02.2010
 *      Author: morgenro
 */

#ifndef FLOODROUTINGEXTENSION_H_
#define FLOODROUTINGEXTENSION_H_

#include "core/Node.h"

#include "routing/SummaryVector.h"
#include "routing/BaseRouter.h"
#include "routing/NeighborDatabase.h"

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/BundleString.h>

#include <ibrcommon/thread/Queue.h>

#include <list>
#include <queue>

namespace dtn
{
	namespace routing
	{
		class FloodRoutingExtension : public BaseRouter::ThreadedExtension
		{
		public:
			FloodRoutingExtension();
			virtual ~FloodRoutingExtension();

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

			/**
			 * hold queued tasks for later processing
			 */
			ibrcommon::Queue<FloodRoutingExtension::Task* > _taskqueue;
		};
	}
}

#endif /* EPIDEMICROUTINGEXTENSION_H_ */
