/*
 * BundleEvent.cpp
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

#include "core/BundleEvent.h"
#include "core/EventDispatcher.h"

namespace dtn
{
	namespace core
	{
		BundleEvent::BundleEvent(const dtn::data::MetaBundle &b, const EventBundleAction action, dtn::data::StatusReportBlock::REASON_CODE reason) : m_bundle(b), m_action(action), m_reason(reason)
		{}

		BundleEvent::~BundleEvent()
		{}

		const dtn::data::MetaBundle& BundleEvent::getBundle() const
		{
			return m_bundle;
		}

		EventBundleAction BundleEvent::getAction() const
		{
			return m_action;
		}

		dtn::data::StatusReportBlock::REASON_CODE BundleEvent::getReason() const
		{
			return m_reason;
		}

		const std::string BundleEvent::getName() const
		{
			return "BundleEvent";
		}

		std::string BundleEvent::getMessage() const
		{
			switch (getAction())
			{
			case BUNDLE_DELETED:
				return "bundle " + getBundle().toString() + " deleted";
			case BUNDLE_CUSTODY_ACCEPTED:
				return "custody accepted for " + getBundle().toString();
			case BUNDLE_FORWARDED:
				return "bundle " + getBundle().toString() + " forwarded";
			case BUNDLE_DELIVERED:
				return "bundle " + getBundle().toString() + " delivered";
			case BUNDLE_RECEIVED:
				return "bundle " + getBundle().toString() + " received";
			case BUNDLE_STORED:
				return "bundle " + getBundle().toString() + " stored";
			default:
				break;
			}

			return "unkown";
		}

		void BundleEvent::raise(const dtn::data::MetaBundle &bundle, EventBundleAction action, dtn::data::StatusReportBlock::REASON_CODE reason)
		{
			// raise the new event
			dtn::core::EventDispatcher<BundleEvent>::queue( new BundleEvent(bundle, action, reason) );
		}
	}
}
