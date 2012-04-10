/*
 * DevNull.h
 *
 *  Created on: 27.01.2010
 *      Author: morgenro
 */

#ifndef DEVNULL_H_
#define DEVNULL_H_

#include "core/AbstractWorker.h"

using namespace dtn::data;
using namespace dtn::core;

namespace dtn
{
	namespace daemon
	{
		class DevNull : public AbstractWorker
		{
		public:
			DevNull()
			{
				AbstractWorker::initialize("/null", 0, true);
			};
			virtual ~DevNull() {};

			void callbackBundleReceived(const Bundle &b);
		};
	}
}

#endif /* DEVNULL_H_ */
