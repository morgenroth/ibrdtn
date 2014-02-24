/*
 * Debugger.cpp
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

#include "Debugger.h"
#include <ibrcommon/Logger.h>
#include <sstream>

namespace dtn
{
	namespace daemon
	{
		void Debugger::callbackBundleReceived(const Bundle &b)
		{
			// do not print anything if debugging is disabled
			if (ibrcommon::Logger::getVerbosity() < 1) return;

			std::stringstream ss_blocks;

			for (dtn::data::Bundle::const_iterator iter = b.begin(); iter != b.end(); ++iter)
			{
				const dtn::data::Block &block = (**iter);
				ss_blocks << "[T=" << (unsigned int)block.getType() << ";F=" << block.getProcessingFlags().get<size_t>() << ";LEN=" << block.getLength() << ";]";
			}

			IBRCOMMON_LOGGER_DEBUG_TAG("Debugger", 1) << "F=" << b.procflags.get<size_t>() <<
					";SRC=" << b.source.getString() <<
					";DST=" << b.destination.getString() <<
					";RPT=" << b.reportto.getString() <<
					";CSD=" << b.custodian.getString() <<
					";TS=" << b.timestamp.get<size_t>() <<
					";SQ=" << b.sequencenumber.get<size_t>() <<
					";LT=" << b.lifetime.get<size_t>() <<
					";FO=" << b.fragmentoffset.get<size_t>() <<
					";AL=" << b.appdatalength.get<size_t>() <<
					";" << ss_blocks.str() <<
					IBRCOMMON_LOGGER_ENDL;
		}
	}
}
