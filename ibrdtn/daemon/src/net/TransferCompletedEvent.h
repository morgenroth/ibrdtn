/*
 * TransferCompletedEvent.h
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
 */

#ifndef TRANSFERCOMPLETEDEVENT_H_
#define TRANSFERCOMPLETEDEVENT_H_

#include "core/Event.h"
#include <ibrdtn/data/MetaBundle.h>
#include "ibrdtn/data/EID.h"

namespace dtn
{
	namespace net
	{
		class TransferCompletedEvent : public dtn::core::Event
		{
		public:
			virtual ~TransferCompletedEvent();

			const string getName() const;

			string toString() const;

			static const string className;

			static void raise(const dtn::data::EID peer, const dtn::data::MetaBundle &bundle);

			const dtn::data::EID& getPeer() const;
			const dtn::data::MetaBundle& getBundle() const;

		private:
			dtn::data::EID _peer;
			dtn::data::MetaBundle _bundle;
			TransferCompletedEvent(const dtn::data::EID peer, const dtn::data::MetaBundle &bundle);
		};
	}
}

#endif /* TRANSFERCOMPLETEDEVENT_H_ */
