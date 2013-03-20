/*
 * ConvergenceLayer.cpp
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

#include "net/ConvergenceLayer.h"
#include "net/BundleReceiver.h"

namespace dtn
{
	namespace net
	{
		ConvergenceLayer::ConvergenceLayer()
		{
		}

		ConvergenceLayer::~ConvergenceLayer()
		{
		}

		ConvergenceLayer::Job::Job()
		{
		}

		ConvergenceLayer::Job::Job(const dtn::data::EID &eid, const dtn::data::BundleID &b)
		 : _bundle(b), _destination(eid)
		{
		}

		ConvergenceLayer::Job::~Job()
		{
		}

		void ConvergenceLayer::Job::clear()
		{
			_bundle = dtn::data::BundleID();
			_destination = dtn::data::EID();
		}
	}
}
