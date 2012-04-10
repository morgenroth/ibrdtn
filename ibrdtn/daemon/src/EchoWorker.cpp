#include "EchoWorker.h"
#include "net/ConvergenceLayer.h"
#include "ibrdtn/utils/Utils.h"
#include <ibrcommon/Logger.h>

using namespace dtn::core;
using namespace dtn::data;

namespace dtn
{
	namespace daemon
	{
		EchoWorker::EchoWorker()
		{
			AbstractWorker::initialize("/echo", 11, true);
		}

		void EchoWorker::callbackBundleReceived(const Bundle &b)
		{
			try {
				const PayloadBlock &payload = b.getBlock<PayloadBlock>();

				// generate a echo
				Bundle echo;

				// make a copy of the payload block
				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				echo.push_back(ref);

				// set destination and mark the bundle as singleton destination
				echo._destination = b._source;
				echo.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);

				// set the source of the bundle
				echo._source = getWorkerURI();

				// set the lifetime to the same value as the received bundle
				echo._lifetime = b._lifetime;

				IBRCOMMON_LOGGER_DEBUG(5) << "echo request received, replying!" << IBRCOMMON_LOGGER_ENDL;

				// send it
				transmit( echo );
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {

			}
		}
	}
}
