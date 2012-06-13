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
			return BundleEvent::className;
		}

		std::string BundleEvent::toString() const
		{
			return className;
		}

		void BundleEvent::raise(const dtn::data::MetaBundle &bundle, EventBundleAction action, dtn::data::StatusReportBlock::REASON_CODE reason)
		{
			// raise the new event
			raiseEvent( new BundleEvent(bundle, action, reason) );
		}

		const string BundleEvent::className = "BundleEvent";
	}
}
