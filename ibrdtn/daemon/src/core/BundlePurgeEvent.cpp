/*
 * BundlePurgeEvent.cpp
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

#include "core/BundlePurgeEvent.h"
#include "core/EventDispatcher.h"

namespace dtn
{
	namespace core
	{
		BundlePurgeEvent::BundlePurgeEvent(const dtn::data::MetaBundle &meta, REASON_CODE r)
		 : bundle(meta), reason(r)
		{
		}

		BundlePurgeEvent::~BundlePurgeEvent()
		{
		}

		void BundlePurgeEvent::raise(const dtn::data::MetaBundle &meta, REASON_CODE reason)
		{
			// raise the new event
			dtn::core::EventDispatcher<BundlePurgeEvent>::queue( new BundlePurgeEvent(meta, reason) );
		}

		const std::string BundlePurgeEvent::getName() const
		{
			return "BundlePurgeEvent";
		}

		std::string BundlePurgeEvent::getMessage() const
		{
			return "purging bundle " + bundle.toString();
		}
	} /* namespace core */
} /* namespace dtn */
