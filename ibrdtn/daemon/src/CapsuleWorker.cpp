/*
 * CapsuleWorker.cpp
 *
 *  Created on: 26.04.2011
 *      Author: morgenro
 */

#include "CapsuleWorker.h"
#include <ibrdtn/data/PayloadBlock.h>
#include "net/BundleReceivedEvent.h"
#include <ibrdtn/data/ScopeControlHopLimitBlock.h>
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace daemon
	{
		CapsuleWorker::CapsuleWorker()
		{
			AbstractWorker::initialize("/bundle-in-bundle", 2, true);
		}

		CapsuleWorker::~CapsuleWorker()
		{
		}

		void CapsuleWorker::callbackBundleReceived(const dtn::data::Bundle &capsule)
		{
			try {
				const PayloadBlock &payload = capsule.getBlock<PayloadBlock>();
				ibrcommon::BLOB::iostream stream = payload.getBLOB().iostream();

				// read the number of bundles
				SDNV nob; (*stream) >> nob;

				// read all offsets
				for (size_t i = 0; i < (nob.getValue() - 1); i++)
				{
					SDNV offset; (*stream) >> offset;
				}

				// create a deserializer for all bundles
				dtn::data::DefaultDeserializer deserializer(*stream);
				dtn::data::Bundle b;

				try {
					// read all bundles
					for (size_t i = 0; i < nob.getValue(); i++)
					{
						// deserialize the next bundle
						deserializer >> b;

						// increment value in the scope control hop limit block
						try {
							dtn::data::ScopeControlHopLimitBlock &schl = b.getBlock<dtn::data::ScopeControlHopLimitBlock>();
							schl.increment();
						} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

						// raise default bundle received event
						dtn::net::BundleReceivedEvent::raise(capsule._source, b, false, true);
					}
				}
				catch (const dtn::InvalidDataException &ex) {
					// display the rejection
					IBRCOMMON_LOGGER(warning) << "invalid bundle-data received: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				}
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };
		}
	}
}
