/*
 * QueueBundle.h
 *
 *  Created on: 15.02.2010
 *      Author: morgenro
 */

#ifndef QUEUEBUNDLEEVENT_H_
#define QUEUEBUNDLEEVENT_H_

#include "core/Event.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/MetaBundle.h"
#include "ibrdtn/data/EID.h"

namespace dtn
{
	namespace routing
	{
		class QueueBundleEvent : public dtn::core::Event
		{
		public:
			virtual ~QueueBundleEvent();

			const string getName() const;

			string toString() const;

			static const string className;

			static void raise(const dtn::data::MetaBundle &bundle, const dtn::data::EID &origin);

			const dtn::data::MetaBundle bundle;
			const dtn::data::EID origin;

		private:
			QueueBundleEvent(const dtn::data::MetaBundle &bundle, const dtn::data::EID &origin);
		};
	}
}

#endif /* QUEUEBUNDLEEVENT_H_ */
