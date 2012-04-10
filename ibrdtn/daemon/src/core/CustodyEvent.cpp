/*
 * CustodyEvent.cpp
 *
 *  Created on: 06.03.2009
 *      Author: morgenro
 */

#include "core/CustodyEvent.h"
#include "ibrdtn/data/Exceptions.h"

namespace dtn
{
	namespace core
	{
		CustodyEvent::CustodyEvent(const dtn::data::MetaBundle &bundle, const EventCustodyAction action)
		: m_bundle(bundle), m_action(action)
		{
		}

		CustodyEvent::~CustodyEvent()
		{
		}

		const dtn::data::MetaBundle& CustodyEvent::getBundle() const
		{
			return m_bundle;
		}

		EventCustodyAction CustodyEvent::getAction() const
		{
			return m_action;
		}

		const std::string CustodyEvent::getName() const
		{
			return CustodyEvent::className;
		}

		std::string CustodyEvent::toString() const
		{
			return className;
		}

		void CustodyEvent::raise(const dtn::data::MetaBundle &bundle, const EventCustodyAction action)
		{
			raiseEvent( new CustodyEvent(bundle, action) );
		}

		const std::string CustodyEvent::className = "CustodyEvent";
	}
}
