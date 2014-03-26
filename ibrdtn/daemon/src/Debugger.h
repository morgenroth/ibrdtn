/*
 * Debugger.h
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

#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include "core/AbstractWorker.h"

namespace dtn
{
	namespace daemon
	{
		/**
		 * This is a implementation of AbstractWorker and is comparable with
		 * a application. This application can send and receive bundles, but
		 * only implement a receiving component which print a message on the
		 * screen if a bundle is received.
		 *
		 * The application suffix to the node eid is /debugger.
		 */
		class Debugger : public dtn::core::AbstractWorker
		{
			public:
				Debugger()
				{
					AbstractWorker::initialize("debugger");
				};
				virtual ~Debugger() {};

				void callbackBundleReceived(const Bundle &b);
		};
	}
}

#endif /*DEBUGGER_H_*/
