/*
 * BundleGeneratedEvent.cpp
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

#include "core/BundleGeneratedEvent.h"
#include "core/BundleCore.h"
#include "core/EventDispatcher.h"
#include <ibrcommon/Logger.h>

namespace dtn
{

	namespace core
	{
		BundleGeneratedEvent::BundleGeneratedEvent(const dtn::data::Bundle &b)
		 : _bundle(b)
		{

		}

		BundleGeneratedEvent::~BundleGeneratedEvent()
		{

		}

		void BundleGeneratedEvent::raise(const dtn::data::Bundle &bundle)
		{
			// raise the new event
			dtn::core::EventDispatcher<BundleGeneratedEvent>::queue( new BundleGeneratedEvent(bundle) );
		}

		const std::string BundleGeneratedEvent::getName() const
		{
			return BundleGeneratedEvent::className;
		}

		std::string BundleGeneratedEvent::getMessage() const
		{
			return "Bundle generated " + _bundle.toString();
		}

		const dtn::data::Bundle& BundleGeneratedEvent::getBundle() const
		{
			return _bundle;
		}

		const string BundleGeneratedEvent::className = "BundleGeneratedEvent";
	}

}
