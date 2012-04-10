/*
 * CustodyEvent.h
 *
 *  Created on: 06.03.2009
 *      Author: morgenro
 */

#ifndef CUSTODYEVENT_H_
#define CUSTODYEVENT_H_

#include <string>
#include "core/Event.h"

#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/CustodySignalBlock.h>

namespace dtn
{
	namespace core
	{
		enum EventCustodyAction
		{
			CUSTODY_REJECT = 0,
			CUSTODY_ACCEPT = 1
		};

		class CustodyEvent : public Event
		{
		public:
			virtual ~CustodyEvent();

			EventCustodyAction getAction() const;
			const dtn::data::MetaBundle& getBundle() const;
			const std::string getName() const;

			std::string toString() const;

			static void raise(const dtn::data::MetaBundle &bundle, const EventCustodyAction action);

			static const std::string className;

		private:
			CustodyEvent(const dtn::data::MetaBundle &bundle, const EventCustodyAction action);

			const dtn::data::MetaBundle m_bundle;
			const EventCustodyAction m_action;
		};
	}
}

#endif /* CUSTODYEVENT_H_ */
