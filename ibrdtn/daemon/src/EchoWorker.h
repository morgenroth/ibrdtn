/*
 * EchoWorker.h
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

#ifndef ECHOWORKER_H_
#define ECHOWORKER_H_

#include "core/AbstractWorker.h"

using namespace dtn::core;

namespace dtn
{
	namespace daemon
	{
		/**
		 * This is a implementation of AbstractWorker and is comparable with
		 * a application. This application can send/receive bundles and implements
		 * a basic echo funcionality. If a bundle is received the payload of the
		 * bundle is copied and returned to the sender.
		 *
		 * The application suffix to the node eid is /echo.
		 */
		class EchoWorker : public AbstractWorker
		{
		public:
			EchoWorker();
			virtual ~EchoWorker() {};

			void callbackBundleReceived(const Bundle &b);
		};
	}
}

#endif /*ECHOWORKER_H_*/
