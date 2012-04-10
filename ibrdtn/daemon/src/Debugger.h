#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include "core/AbstractWorker.h"

using namespace dtn::data;
using namespace dtn::core;

namespace dtn
{
	namespace daemon
	{
		/**
		 * This is a implementation of AbstractWorker and is comparable with
		 * a application. This application can send and receive bundles, but
		 * only implement a receiving component which print a message on the
		 * screen if a bundle is received.
		 *
		 * The application suffix to the node eid is /debugger.
		 */
		class Debugger : public AbstractWorker
		{
			public:
				Debugger()
				{
					AbstractWorker::initialize("/debugger", 1, true);
				};
				virtual ~Debugger() {};

				void callbackBundleReceived(const Bundle &b);
		};
	}
}

#endif /*DEBUGGER_H_*/
