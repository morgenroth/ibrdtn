/*
 * BundleEvent.cpp
 *
 *  Created on: 06.03.2009
 *      Author: morgenro
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
