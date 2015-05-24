/*
 * CapsuleWorker.cpp
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

#include "Configuration.h"
#include "CapsuleWorker.h"
#include "core/BundleCore.h"
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace daemon
	{
		CapsuleWorker::CapsuleWorker()
		{
			AbstractWorker::initialize("bundle-in-bundle");
		}

		CapsuleWorker::~CapsuleWorker()
		{
		}

		void CapsuleWorker::callbackBundleReceived(const dtn::data::Bundle &capsule)
		{
			try {
				const PayloadBlock &payload = capsule.find<PayloadBlock>();
				ibrcommon::BLOB::iostream stream = payload.getBLOB().iostream();

				// read the number of bundles
				dtn::data::Number nob;
				(*stream) >> nob;

				// read all offsets
				for (size_t i = 0; (nob - 1) > i; ++i)
				{
					dtn::data::Number offset;
					(*stream) >> offset;
				}

				// create a deserializer for all bundles
				dtn::data::DefaultDeserializer deserializer(*stream, dtn::core::BundleCore::getInstance());
				dtn::data::Bundle b;

				try {
					// read all bundles
					for (size_t i = 0; nob > i; ++i)
					{
						// deserialize the next bundle
						deserializer >> b;

						// pass extracted bundle through the filter
						FilterContext context;
						context.setBundle(b);
						context.setPeer(capsule.source.getNode());
						if (BundleCore::getInstance().filter(BundleFilter::INPUT, context, b) == BundleFilter::ACCEPT)
						{
							// inject bundle into core
							dtn::core::BundleCore::getInstance().inject(capsule.source, b, false);
						}
					}
				}
				catch (const dtn::InvalidDataException &ex) {
					// display the rejection
					IBRCOMMON_LOGGER_DEBUG_TAG("CapsuleWorker", 2) << "invalid bundle-data received: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				}
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };
		}
	}
}
