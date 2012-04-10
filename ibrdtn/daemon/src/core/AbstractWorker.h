#ifndef ABSTRACTWORKER_H_
#define ABSTRACTWORKER_H_

#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/BundleID.h>
#include <ibrdtn/data/EID.h>
#include "core/EventReceiver.h"
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Conditional.h>
#include <ibrcommon/thread/Thread.h>
#include "net/ConvergenceLayer.h"

#include <ibrcommon/thread/Queue.h>
#include <set>

using namespace dtn::data;

namespace dtn
{
	namespace core
	{
		class AbstractWorker : public ibrcommon::Mutex
		{
			class AbstractWorkerAsync : public ibrcommon::JoinableThread, public dtn::core::EventReceiver
			{
			public:
				AbstractWorkerAsync(AbstractWorker &worker);
				virtual ~AbstractWorkerAsync();
				void shutdown();

				virtual void raiseEvent(const dtn::core::Event *evt);

			protected:
				void run();
				void __cancellation();

			private:
				void prepareBundle(dtn::data::Bundle &bundle) const;

				AbstractWorker &_worker;
				bool _running;

				ibrcommon::Queue<dtn::data::BundleID> _receive_bundles;
			};

			public:
				AbstractWorker();

				virtual ~AbstractWorker();

				virtual const EID getWorkerURI() const;

				virtual void callbackBundleReceived(const Bundle &b) = 0;

			protected:
				void initialize(const string uri, const size_t cbhe, bool async);
				void transmit(const Bundle &bundle);

				EID _eid;

				void shutdown();

				/**
				 * subscribe to a end-point
				 */
				void subscribe(const dtn::data::EID &endpoint);

				/**
				 * unsubscribe to a end-point
				 */
				void unsubscribe(const dtn::data::EID &endpoint);

			private:
				AbstractWorkerAsync _thread;
				std::set<dtn::data::EID> _groups;
		};
	}
}

#endif /*ABSTRACTWORKER_H_*/
