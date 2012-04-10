/*
 * CapsuleWorker.h
 *
 *  Created on: 26.04.2011
 *      Author: morgenro
 */

#ifndef CAPSULEWORKER_H_
#define CAPSULEWORKER_H_

#include "core/AbstractWorker.h"
#include <ibrdtn/data/Bundle.h>
#include <ibrcommon/data/BLOB.h>
#include <list>

namespace dtn
{
	namespace daemon
	{
		class CapsuleWorker : public dtn::core::AbstractWorker
		{
		public:
			CapsuleWorker();
			virtual ~CapsuleWorker();

			void callbackBundleReceived(const dtn::data::Bundle &b);
		};
	}
}

#endif /* CAPSULEWORKER_H_ */
