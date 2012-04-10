/*
 * DevNull.cpp
 *
 *  Created on: 27.01.2010
 *      Author: morgenro
 */

#include "DevNull.h"
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace daemon
	{
		void DevNull::callbackBundleReceived(const dtn::data::Bundle &b)
		{
			IBRCOMMON_LOGGER(info) << "Bundle " << b.toString() << " went to /null" << IBRCOMMON_LOGGER_ENDL;
		}
	}
}
