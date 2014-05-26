/*
 * EchoWorker.cpp
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

#include "EchoWorker.h"
#include "net/ConvergenceLayer.h"
#include "ibrdtn/utils/Utils.h"
#include <ibrdtn/data/TrackingBlock.h>
#include <ibrcommon/Logger.h>

using namespace dtn::core;
using namespace dtn::data;

namespace dtn
{
	namespace daemon
	{
		EchoWorker::EchoWorker()
		{
			AbstractWorker::initialize("echo");
		}

		void EchoWorker::callbackBundleReceived(const Bundle &b)
		{
			try {
				const PayloadBlock &payload = b.find<PayloadBlock>();

				// generate a echo
				Bundle echo;

				// make a copy of the payload block
				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				echo.push_back(ref);

				// set destination and mark the bundle as singleton destination
				echo.destination = b.source;
				echo.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);

				// set the source of the bundle
				echo.source = getWorkerURI();

				// set the lifetime to the same value as the received bundle
				echo.lifetime = b.lifetime;

				// sign the reply if the echo-request was signed too
				if (b.get(dtn::data::PrimaryBlock::DTNSEC_STATUS_VERIFIED))
					echo.set(dtn::data::PrimaryBlock::DTNSEC_REQUEST_SIGN, true);

				// encrypt the reply if the echo-request was encrypt too
				if (b.get(dtn::data::PrimaryBlock::DTNSEC_STATUS_CONFIDENTIAL))
					echo.set(dtn::data::PrimaryBlock::DTNSEC_REQUEST_ENCRYPT, true);

				try {
					const dtn::data::TrackingBlock &tracking = b.find<dtn::data::TrackingBlock>();
					dtn::data::TrackingBlock &target = echo.push_back<dtn::data::TrackingBlock>();

					// copy tracking block
					target = tracking;
				} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

				IBRCOMMON_LOGGER_DEBUG_TAG("EchoWorker", 5) << "echo request received, replying!" << IBRCOMMON_LOGGER_ENDL;

				// send it
				transmit( echo );
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {

			}
		}
	}
}
