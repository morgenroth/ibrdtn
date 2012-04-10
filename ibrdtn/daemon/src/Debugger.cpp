#include "Debugger.h"
#include "ibrdtn/utils/Utils.h"
#include <ibrcommon/Logger.h>

using namespace dtn::data;
using namespace dtn::core;
using namespace std;

namespace dtn
{
	namespace daemon
	{
		void Debugger::callbackBundleReceived(const Bundle &b)
		{
			IBRCOMMON_LOGGER_DEBUG(5) << "Bundle received " << b.toString() << IBRCOMMON_LOGGER_ENDL;
		}
	}
}
